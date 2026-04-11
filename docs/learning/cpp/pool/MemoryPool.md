---
title: 内存池
---

# 内存池

!!! abstract "核心概念"

    **内存池** (Memory Pool) 是一种预先分配并自行管理内存生命周期的资源管理机制。其核心目标是降低频繁调用系统通用分配器（如 `malloc` 或 `new`）的开销，控制内存碎片，并提供可预测的内存分配延迟。

## 工程背景与原理

在系统级开发中，通用内存分配器需要兼顾不同请求大小、页级分配与回收、并发同步等复杂场景。当业务场景存在特定规律时，引入自定义内存池可以显著优化性能。

- 降低分配开销：避免频繁的用户态至内核态切换与分配器内部锁竞争。
- 减少内存碎片：通过统一接管特定尺寸或范围的内存块，消除外部碎片。
- 提升数据局部性：预留连续内存空间，提高 CPU 缓存命中率。

### 内存对齐

内存池底层实现必须严格保证内存对齐。

- 固定尺寸池：切块时通常使用 `alignas(T)` 确保每个对象块满足对齐要求。
- 变长块池：对每个分配请求的计算位移与大小进行对齐向上舍入。
- 未对齐风险：可能导致缓存线跨越访问、性能退化，甚至在某些架构上触发未定义行为。

## 核心设计路线对比

内存池的实现通常分为两种主要架构：固定尺寸池与变长块池。两者针对完全不同的工程需求。

| 设计维度 | 固定尺寸池 | 变长块池 |
| --- | --- | --- |
| 分配单位 | 单一块大小固定 | 单一块大小可变 |
| 外部碎片 | 基本不存在 | 存在，需合并机制 |
| 分配时间复杂度 | 依靠空闲集合优化至 $O(1)$ | 需遍历适配，如 First-Fit |
| 块合并需求 | 不需要 | 需要相邻空闲块合并 |
| 接口形态 | 返回对象指针 `T*` | 建议返回句柄或指针加大小 |
| 典型应用场景 | 对象池、固定结构节点缓存 | 小型 Arena、受限空间通用块分配器 |

## 固定尺寸池

固定尺寸池将预分配的内存空间划分为大小统一的块。每个块的状态仅为被占用或空闲。

### 状态管理机制

- 线性扫描：使用 `std::vector<Block>` 持有块并标注占用标志位。分配时需进行 $O(N)$ 的遍历查询，随池占用率升高，性能将严重恶化。
- 空闲集合：通过额外的空闲链表或空闲索引栈显式维护空闲块集合。分配与回收操作仅涉及集合头部的移除与插入，时间复杂度稳定降至 $O(1)$。

???+ note "固定尺寸池示例"

    ```cpp title="fixed_block_pool.hpp"
    #include <cstddef>
    #include <memory>
    #include <mutex>
    #include <new>
    #include <shared_mutex>
    #include <stdexcept>
    #include <type_traits>
    #include <utility>
    #include <vector>

    template <typename T>
    class FixedBlockPool {
    public:
        struct Stats {
            std::size_t capacity = 0;
            std::size_t used = 0;

            std::size_t free() const noexcept {
                return capacity - used;
            }
        };

    private:
        struct Block {
            // 每个槽位只负责容纳一个 T，对象本体在 storage 中原地构造。
            alignas(T) std::byte storage[sizeof(T)];
            bool used = false;
        };

        static_assert(std::is_standard_layout_v<Block>);

        std::vector<Block> blocks_;
        std::vector<std::size_t> free_indices_;
        std::size_t used_count_ = 0;
        mutable std::shared_mutex mutex_;

        static T* ptr(Block& block) noexcept {
            return std::launder(reinterpret_cast<T*>(block.storage));
        }

        static const T* ptr(const Block& block) noexcept {
            return std::launder(reinterpret_cast<const T*>(block.storage));
        }

        static Block* block_from_ptr(T* obj) noexcept {
            // deallocate 时通过对象地址回退到所属 Block。
            auto* raw = reinterpret_cast<std::byte*>(obj);
            return reinterpret_cast<Block*>(raw - offsetof(Block, storage));
        }

        std::size_t index_from_block(const Block* block) const noexcept {
            return static_cast<std::size_t>(block - blocks_.data());
        }

    public:
        explicit FixedBlockPool(std::size_t capacity)
            : blocks_(capacity) {
            free_indices_.reserve(capacity);
            // 预先建立空闲索引栈，分配时只需弹出栈顶。
            for (std::size_t i = 0; i < capacity; ++i) {
                free_indices_.push_back(capacity - 1 - i);
            }
        }

        template <typename... Args>
        T* allocate(Args&&... args) {
            std::unique_lock lock(mutex_);

            if (free_indices_.empty()) {
                return nullptr;
            }

            const std::size_t index = free_indices_.back();
            free_indices_.pop_back();

            Block& block = blocks_[index];
            // 在预留槽位上直接构造对象，避免额外堆分配。
            ::new (static_cast<void*>(block.storage))
                T(std::forward<Args>(args)...);
            block.used = true;
            ++used_count_;
            return ptr(block);
        }

        void deallocate(T* obj) {
            if (obj == nullptr) {
                return;
            }

            std::unique_lock lock(mutex_);
            Block* block = block_from_ptr(obj);
            const std::size_t index = index_from_block(block);

            if (!blocks_[index].used) {
                throw std::logic_error("Double free detected");
            }

            // 对象析构后，把槽位重新放回空闲索引栈。
            std::destroy_at(obj);
            blocks_[index].used = false;
            free_indices_.push_back(index);
            --used_count_;
        }
    };
    ```

### 适用场景边界

固定尺寸池特别适用于生命周期无序且频繁创建与销毁的定长结构，如抽象语法树节点。当请求的内存大小差异极大时，不具备空间灵活性。

## 变长块池

变长块池允许每次请求不同大小的内存块。由于回收后会产生不同大小的空间碎片，需要维护更复杂的元数据并执行连续空间检索。

### 核心操作与元数据设计

- 切分：当获取的空闲块容量超过请求数值时，需要将其截断为请求所需的“已用块”与剩余的“新空闲块”。
- 合并：释放内存块时，需检查并在物理上合并前后相邻的空闲块，以消除外部碎片并承接后续更大块的分配。
- 元数据管理：通常使用单片底层连续内存管理真实数据，并配套数组结构记录各分配块在底层内存的偏移量、实际容量与双向链表信息。

!!! warning "底层扩容与裸指针失效风险"

    变长块池在支持底层重分配扩容的场景下，地址发生变化会导致已分发给用户的裸指针失效。通常采用句柄机制，仅返回内部索引与偏移量；或者禁用内存上限之后的动态扩容。

???+ note "变长块池示例"

    ```cpp title="variable_block_pool.hpp"
    #include <cstddef>
    #include <cstdint>
    #include <mutex>
    #include <shared_mutex>
    #include <stdexcept>
    #include <vector>

    class VariableBlockPool {
    public:
        struct Stats {
            std::size_t capacity_bytes = 0;
            std::size_t used_bytes = 0;

            std::size_t free_bytes() const noexcept {
                return capacity_bytes - used_bytes;
            }
        };

        struct Handle {
            int block = -1;
            std::size_t offset = 0;
            std::size_t size = 0;

            explicit operator bool() const noexcept {
                return block >= 0;
            }
        };

    private:
        struct BlockMeta {
            std::size_t offset = 0;
            std::size_t size = 0;
            bool used = false;
            bool active = true;
            // 物理链用于识别相邻块，支持回收后的区段合并。
            int prev_phys = -1;
            int next_phys = -1;
            // 空闲链只串未使用块，避免每次分配扫描全部元数据。
            int prev_free = -1;
            int next_free = -1;
        };

        static constexpr std::size_t kAlignment = alignof(std::max_align_t);

        std::vector<std::byte> arena_;
        std::vector<BlockMeta> blocks_;
        int free_head_ = -1;
        std::size_t used_bytes_ = 0;
        mutable std::shared_mutex mutex_;

        static std::size_t align_up(std::size_t n) noexcept {
            return (n + kAlignment - 1) & ~(kAlignment - 1);
        }

        int create_block(const BlockMeta& meta) {
            blocks_.push_back(meta);
            return static_cast<int>(blocks_.size() - 1);
        }

        void insert_into_free_list(int index) {
            BlockMeta& block = blocks_[index];
            block.prev_free = -1;
            block.next_free = free_head_;

            if (free_head_ != -1) {
                blocks_[free_head_].prev_free = index;
            }

            free_head_ = index;
        }

        void remove_from_free_list(int index) {
            BlockMeta& block = blocks_[index];

            if (block.prev_free != -1) {
                blocks_[block.prev_free].next_free = block.next_free;
            } else {
                free_head_ = block.next_free;
            }

            if (block.next_free != -1) {
                blocks_[block.next_free].prev_free = block.prev_free;
            }

            block.prev_free = -1;
            block.next_free = -1;
        }

        int find_first_fit(std::size_t bytes) const {
            // 示例采用 first-fit，重点展示空闲链和物理链如何配合。
            for (int i = free_head_; i != -1; i = blocks_[i].next_free) {
                const BlockMeta& block = blocks_[i];
                if (block.active && !block.used && block.size >= bytes) {
                    return i;
                }
            }
            return -1;
        }

        void merge_with_next(int index) {
            const int next = blocks_[index].next_phys;
            if (next == -1) {
                return;
            }

            BlockMeta& current = blocks_[index];
            BlockMeta& next_block = blocks_[next];
            if (!next_block.active || next_block.used) {
                return;
            }

            // 被并入的后继块先从空闲链摘除，再把容量并入当前块。
            remove_from_free_list(next);

            current.size += next_block.size;
            current.next_phys = next_block.next_phys;

            if (current.next_phys != -1) {
                blocks_[current.next_phys].prev_phys = index;
            }

            next_block.active = false;
            next_block.size = 0;
            next_block.prev_phys = -1;
            next_block.next_phys = -1;
        }

        void merge_around(int& index) {
            // 先向前合并，再向后合并，避免遗漏连续的空闲区段。
            const int prev = blocks_[index].prev_phys;
            if (prev != -1 && blocks_[prev].active && !blocks_[prev].used) {
                remove_from_free_list(index);
                merge_with_next(prev);
                index = prev;
            }

            merge_with_next(index);
        }

    public:
        explicit VariableBlockPool(std::size_t capacity_bytes)
            : arena_(align_up(capacity_bytes)) {
            blocks_.reserve(128);
            blocks_.push_back(BlockMeta{
                0, arena_.size(), false, true, -1, -1, -1, -1
            });
            free_head_ = 0;
        }

        Handle allocate(std::size_t bytes) {
            bytes = align_up(bytes);
            if (bytes == 0) {
                return {};
            }

            std::unique_lock lock(mutex_);

            const int index = find_first_fit(bytes);
            if (index == -1) {
                return {};
            }

            remove_from_free_list(index);

            const std::size_t old_offset = blocks_[index].offset;
            const std::size_t old_size = blocks_[index].size;
            const int old_next = blocks_[index].next_phys;

            // 请求明显小于空闲块时，把剩余尾部切出来继续作为空闲块。
            if (old_size > bytes) {
                const int tail = create_block(BlockMeta{
                    old_offset + bytes,
                    old_size - bytes,
                    false,
                    true,
                    index,
                    old_next,
                    -1,
                    -1
                });

                if (old_next != -1) {
                    blocks_[old_next].prev_phys = tail;
                }

                blocks_[index].next_phys = tail;
                blocks_[index].size = bytes;
                insert_into_free_list(tail);
            }

            blocks_[index].used = true;
            used_bytes_ += blocks_[index].size;

            return Handle{index, blocks_[index].offset, blocks_[index].size};
        }

        void deallocate(Handle handle) {
            if (!handle) {
                return;
            }

            std::unique_lock lock(mutex_);

            if (handle.block < 0 ||
                static_cast<std::size_t>(handle.block) >= blocks_.size()) {
                throw std::logic_error("invalid handle");
            }

            int index = handle.block;
            BlockMeta& block = blocks_[index];

            if (!block.active || !block.used) {
                throw std::logic_error("double free or stale handle");
            }

            block.used = false;
            used_bytes_ -= block.size;
            insert_into_free_list(index);

            // 释放后尝试和前后相邻空闲块做 coalesce，减少外部碎片。
            merge_around(index);
        }

        std::byte* data(Handle handle) {
            std::shared_lock lock(mutex_);
            if (!handle || handle.block < 0 ||
                static_cast<std::size_t>(handle.block) >= blocks_.size()) {
                return nullptr;
            }

            if (!blocks_[handle.block].active || !blocks_[handle.block].used) {
                return nullptr;
            }
            // 句柄只描述块位置，真正读写时再映射回 arena 中的字节区间。
            return arena_.data() + blocks_[handle.block].offset;
        }

        const std::byte* data(Handle handle) const {
            std::shared_lock lock(mutex_);
            if (!handle || handle.block < 0 ||
                static_cast<std::size_t>(handle.block) >= blocks_.size()) {
                return nullptr;
            }

            if (!blocks_[handle.block].active || !blocks_[handle.block].used) {
                return nullptr;
            }
            return arena_.data() + blocks_[handle.block].offset;
        }

        Stats stats() const {
            std::shared_lock lock(mutex_);
            return Stats{arena_.size(), used_bytes_};
        }

        std::size_t capacity_bytes() const {
            std::shared_lock lock(mutex_);
            return arena_.size();
        }

        std::size_t used_bytes() const {
            std::shared_lock lock(mutex_);
            return used_bytes_;
        }

        bool is_used(Handle handle) const {
            std::shared_lock lock(mutex_);
            if (!handle || handle.block < 0 ||
                static_cast<std::size_t>(handle.block) >= blocks_.size()) {
                return false;
            }

            const BlockMeta& block = blocks_[handle.block];
            return block.active && block.used;
        }
    };
    ```

## 并发环境特征

多核系统中两类内存池存在不同的竞争热点与优化方向。

### 固定尺寸池并发表现

- 分配路径极短，不需要合并或拆分，核心锁竞争较低。
- 进阶实现通常将池按线程划分片区，或引入线程本地存储以实现高并发无锁分配。

### 变长块池并发表现

- 因需遍历空闲链表、更新多个元信息节点及处理区段合并，导致锁临界区较宽。
- 工程实践中较少使用单体全能变长池。多采用类似预分级大小归类技术路线，以分桶机制降低全局竞争，超大请求则转交内核管理。

!!! warning "可见性防线限制"

    接管底层内存管理将导致工具对池内细节边界探测失效。这会增大悬挂引用和双重释放问题排查难度。因此生产环境往往需要引入探测哨兵、安全隔离或魔法边界判断实现人工侧漏拦截。

*[ First-Fit ]: 内存分配策略之一，分配首个大小满足所需条件的空闲块。
*[ Arena ]: 预先分配的大块连续内存区域，作为后续细粒度切分的底层基底。
*[ Valgrind ]: 一款包含多种内存调试、内存泄漏检测以及性能分析工具的软件。
*[ AddressSanitizer ]: 一种基于编译器的快速内存错误检测器。
