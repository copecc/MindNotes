---
title: CUDA 基础
tags:
  - CUDA
  - GPU
  - 并行计算
  - Kernel Launch
  - Occupancy
---

# CUDA 基础

!!! abstract "摘要"

    本文介绍 CUDA 的核心概念及性能分析框架。内容涵盖三个维度：线程组织与调度、存储层次结构与访存路径、常见性能瓶颈的成因与优化策略。

!!! info "参考资料"

    - [CUDA C++ Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/){target=_blank}
    - [CUDA C++ Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/){target=_blank}
    - [NCCL User Guide](https://docs.nvidia.com/deeplearning/nccl/user-guide/docs/){target=_blank}

## CUDA 的定位

CUDA 是 NVIDIA 提供的并行编程平台与编程模型。开发者可以用 C/C++、Python 或其他语言编写在 GPU 上执行的 kernel，并显式管理 host 与 device 之间的数据移动。

GPU 的核心特征是大规模并发执行能力。典型 GPU 包含大量 SM（Streaming Multiprocessor），每个 SM 可同时维护多个 warp。当某个 warp 因访存或数据依赖而等待时，调度器切换至其他 warp 执行，通过线程上下文切换隐藏延迟。

CUDA 程序的核心目标是充分利用 GPU 的并发能力，主要优化方向包括：

- 维持足够的并发线程，避免 SM 空闲。
- 优化同一 warp 内的内存访问与控制流，保持规整性。
- 最大化寄存器和 shared memory 的数据复用，降低 global memory 访问开销。

## 执行模型：Grid、Block、Warp 与 SM

CUDA kernel 的启动形式为：

$$
\text{kernel} \lll D_g, D_b, N_s, S \ggg (...)
$$

其中：

- `Dg`：grid 维度，即 block 的组织方式。
- `Db`：block 维度，即线程块内线程的组织方式。
- `Ns`：为每个 block 动态分配的 shared memory 字节数。
- `S`：该 kernel 所属的 stream。

grid 是 block 的集合，block 是线程协作的基本单位，warp 是硬件调度的基本单位。主流 CUDA GPU 中 warp 固定为 32 个线程。

同一 block 内的线程可以：（1）通过 shared memory 交换数据（2）通过 `__syncthreads()` 进行 block 级同步。不同 block 之间缺少 shared memory 直接共享和 block 级同步机制。因此 block 的划分决定了线程协作边界、数据共享范围和调度粒度。一个 block 必须完整地驻留在单个 SM 上执行，block 的执行位置始终限定在单个 SM 内。

???+ note "代码示例：一维索引"

    ```cpp title="index_template.cu"
    __global__ void vector_add(const float* a, const float* b, float* c, int n) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;

        if (idx < n) {
            c[idx] = a[idx] + b[idx];
        }
    }
    ```

    `idx` 是最常见的一维全局线程编号。对于二维或三维问题，将 `blockIdx`、`threadIdx` 的多个维度组合，映射到矩阵坐标或体素坐标。

## 执行配置

### Block Size

`block_size` 影响任务如何分配成 warp。由于基于 warp 的不可分割调度，应为 32 的整数倍。128、256、512 为常见选值。

过小的 block 无法产生足够的并发度；过大则导致寄存器与 shared memory 分配紧张。两者都会降低单个 SM 的活跃 block 数，限制整体吞吐量。

!!! note "工程实践"

    在缺乏 profiler 数据时，128 或 256 通常为稳妥的初始选择。

### Occupancy

占用率（Occupancy）是指流式多处理器（SM）中当前活跃的线程数与该硬件所能支持的最大并发线程数之比，计算公式为：

$$
Occupancy = \frac{\text{Active Threads Per SM}}{\text{Maximum Threads Per SM}}
$$

在 GPU 体系结构中，高占用率的主要物理意义是：SM 能够维持足够数量的活跃线程束。当部分 Warp 因等待内存数据而停滞时，调度器可以无缝切换至其他就绪的 Warp 执行计算，从而有效隐藏内存访问延迟。

在实际执行中，即使占用率极高，计算性能仍可能受限于以下硬件与逻辑瓶颈：

1. 全局内存带宽饱和：内存的物理读取带宽已被耗尽，导致所有并发的 Warp 都在排队等待数据。
2. 共享内存存储体冲突：同一 Warp 内的多个线程同时访问同一个 Bank 的不同物理地址，导致硬件将内存请求强制串行化处理。
3. 寄存器溢出：线程消耗的寄存器超出了 SM 的物理容量，局部变量被迫溢出至高延迟的本地内存。
4. 控制流发散：在同一 Warp 内，条件分支指令导致线程走向不同执行路径，硬件必须依次串行执行各个分支。
5. 原子操作竞争：大量线程同时尝试对同一个内存地址执行读改写（RMW）操作，导致严重的排队延迟。

因此，占用率仅是性能评估的基础指标之一，实际优化必须结合 Profiler（如 Nsight Compute）的性能分析数据综合判定。

SM 内部的活跃线程数由多个维度的物理资源上限共同决定。假设某 GPU 架构单 SM 的硬件限制为：最多并发 2048 个线程，且最多驻留 32 个 Block。

- 场景 A：受限于 Block 数量上限（小 Block Size）
    - 配置：设定 block_size = 32（即每个 Block 仅包含 1 个 Warp）。
    - 机制：SM 最多分配 32 个 Block，此时总活跃线程数为 $32 \times 32 = 1024$。
    - 结果：即使 SM 仍有 1024 个线程的计算余量，且寄存器与共享内存充足，占用率也会被 Block 数量的硬件上限严格截断在 50%（$1024 / 2048$）。
- 场景 B：受限于物理存储资源（大 Block Size）
    - 配置：设定 block_size = 512。
    - 机制：理论上 4 个 Block 即可达到 2048 个活跃线程。但大尺寸的 Block 通常需要分配大量的寄存器和 shared memory。
    - 结果：如果单个 Block 所需的存储资源极其庞大，SM 的总物理寄存器或共享内存可能只能满足 1 到 2 个 Block 的驻留需求。这会导致 SM 拒绝调度更多 Block，使得实际并发线程数大幅降低。

!!! info "总结"

    Block Size 的选择本质上是最大化指令吞吐量、维持足量的活跃 Warp 以隐藏延迟、以及控制单个 Block 对共享内存和寄存器资源消耗之间的权衡。最优的 Block Size 需要在以下三个要素之间取得平衡：最大化指令吞吐量。维持足量的活跃 Warp 以实现延迟隐藏。严格控制单个 Block 对共享内存和物理寄存器的资源消耗压力。

### Grid Size

对于一维元素级操作，常见配置为：

$$
\text{grid\_size} = \left\lceil \frac{N}{\text{block\_size}} \right\rceil
$$

???+ note "代码示例：基本 grid 配置"

    ```cpp title="launch_basic.cu"
    int block_size = 256;
    int grid_size = (n + block_size - 1) / block_size;

    vector_add<<<grid_size, block_size>>>(a, b, c, n);
    ```

    该配置保证总线程数至少覆盖所有元素。

### Grid-Stride Loop

对于大规模数据集，grid-stride loop 是更稳定的 kernel 写法。它让线程总数保持在一个可控范围内，再由每个线程按固定步长处理多个元素。

???+ note "代码示例：Grid-Stride Loop"

    ```cpp title="grid_stride_loop.cu"
    __global__ void saxpy(int n, float a, const float* x, float* y) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        int stride = blockDim.x * gridDim.x;

        for (int i = idx; i < n; i += stride) {
            y[i] = a * x[i] + y[i];
        }
    }
    ```

代码中：

- `idx` 表示线程第一次处理的位置。
- `stride = blockDim.x * gridDim.x` 表示整个 grid 的线程跨度。
- 每个线程通过 `i += stride` 继续处理后续元素。

grid-stride loop 的优势在于：

- kernel 与数据规模解耦。
- 可以控制总线程数，避免盲目创建过多线程。
- 每个线程可处理多个元素，从而摊销部分公共指令和初始化成本。

当 kernel 包含较多预处理逻辑，或者希望 grid size 接近 `SM_count` 的若干倍时，grid-stride loop 通常更适合作为基线实现。

### Tail Effect

GPU 调度基于 SM 并发推进。若在某时刻整个 GPU 最多容纳 `W` 个 block，则这 `W` 个 block 可视为一个近似的调度波次（wave）。

若总 block 数略高于一个完整 wave，例如 `W + 1`，最后一个 block 运行时只有极少数 SM 忙碌，其余 SM 空闲。这称为 tail effect，可能导致性能下降。

工程实践中的两个经验做法：

- 对简单 kernel，以 `ceil_div(N, block_size)` 作为基线。
- 对复杂 kernel，若采用 grid-stride loop，令 grid size 接近 `SM_count` 的整数倍，减少尾部效应。

## 内存层次与访存模式

CUDA 提供多级数据存储结构，各层次在物理位置、访问延迟和作用域上存在严格区分：

| 层次 | 作用域 | 物理位置 | 核心特征 |
| --- | --- | --- | --- |
| Register | 单线程 | 芯片内部（SM） | 延迟极低，容量极小。由编译器自动分配。 |
| Local Memory | 单线程逻辑私有 | 设备内存（Global） | 延迟极高。用于处理寄存器溢出或大尺寸局部数组。 |
| Shared Memory | 单 Block | 芯片内部（SM） | 延迟低。支持 Block 内线程高速协同与数据复用，需程序员显式管理容量与 Bank 划分。 |
| Global Memory | 整个 Device | 设备内存（DRAM） | 容量大且延迟高。读写性能高度依赖访存合并。 |
| Constant Memory | 整个 Device | 设备内存（带缓存） | 支持对同一地址的高效广播读取。 |
| Texture / Read-Only Cache | 整个 Device | 设备内存（带缓存） | 针对二维空间局部性或只读数据流优化的存取通道。 |

寄存器（Register） 是线程执行计算时速度最快的存储介质。当局部变量数量超出硬件寄存器上限时，编译器会将数据溢出至 Local Memory，此时指令将面临与 Global Memory 同等的访问延迟。

共享内存（Shared Memory） 提供低于 Global Memory 的访问延迟。核心应用场景包括数据重排、中间结果归约与大块数据复用。由于系统通过多存储体（Bank）并行提供带宽，设计时需避免相邻线程竞争同一 Bank。

常量内存（Constant Memory） 的硬件读取机制决定了其仅在并发线程访问完全相同的地址时才能发挥广播优势。如果同一 Warp 内的线程请求不同的常量内存地址，硬件将被迫串行化这些访问。

CUDA 程序的典型执行流程如下：

- Host Memory 与 Device Memory 之间的数据传输通常通过 PCIe 或 NVLink 进行，带宽远低于 GPU 内部存储访问。
- Device Global Memory 与 Shared Memory 之间的数据交换需要显式的内存拷贝指令，且访问模式对性能有显著影响。
- Shared Memory 与 Register 之间的数据流通常通过指令级别的寄存器分配和共享内存访问实现，编译器会根据资源需求自动优化数据布局。

在计算密集型场景中，Host 到 Device 的传输容易成为全局吞吐瓶颈。一般通过 Pinned Memory 与异步传输流来隐藏由于拷贝代理造成的等待时间。

### Global Memory

#### 访存合并机制

当同一个 Warp 内的 32 个线程共同访问 Global Memory 时，硬件内存控制器会尝试将这些离散的请求打包为数量最少的物理内存事务。这种硬件特性称为 访存合并（Memory Coalescing）。

高效的访存合并能够直接带来以下优势：

- 极大提升实际有效带宽利用率。
- 减少内存总事务数量。
- 降低 SM 等待内存数据加载的时钟周期。

#### 连续访问与跨步访问

一维数组中，最有利于访存合并的模式是连续访问，即相邻线程读取相邻地址。例如：$a[idx]$。这种情况下，一个 warp 内线程的访问地址通常集中在少量连续的内存区间内，硬件更容易把这些请求合并成较少的内存事务。

跨步访问的形式通常写成：$a[idx \times \text{stride}]$。当 `stride` 增大时，相邻线程读取的地址间隔也随之增大。这样会带来两个直接结果：一是同一个 warp 的访问更分散，二是一次缓存行加载后，真正被当前 warp 使用的数据比例下降。结果通常是访存合并效果变差，有效带宽下降。

???+ note "代码示例：连续访问与跨步访问"

    ```cpp title="coalescing_compare.cu"
    __global__ void load_contiguous(const float* x, float* y, int n) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;

        if (idx < n) {
            y[idx] = x[idx];
        }
    }

    __global__ void load_strided(const float* x, float* y, int n, int stride) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        int src = idx * stride;

        if (src < n) {
            y[idx] = x[src];
        }
    }
    ```

    `load_contiguous` 的地址模式更规整，通常能获得更高的带宽利用率。`load_strided` 的性能则会随着 `stride` 增大而下降。

#### 二维访问中的行访问与列访问

对于按行主序（Row-Major）存储的二维矩阵，相邻线程读取同一行中的相邻元素时，访问地址连续，通常可以形成较好的访存合并。

相邻线程读取同一列数据时，地址间隔接近整行跨度。在线程编号连续而地址跨度很大的情况下，同一个 warp 的请求会分散到更宽的地址区间内，访存合并效果随之下降，有效带宽也会降低。

矩阵转置和 tiled kernel 通常采用同一类处理方式：先把二维数据块加载到 shared memory，在 block 内完成局部重排，再按连续地址写回 global memory。这样做的目的，是把原本不利于合并的列访问转换成更规整的行访问。

#### SoA 与 AoS 的内存布局对访存的影响

多字段数据结构的物理布局会直接影响访存模式。若一个 warp 只处理一个字段，数组结构体（AoS, Array of Structures）布局会让相邻线程访问地址之间隔着整个结构体大小。访问跨度等于结构体大小，这类模式通常不利于访存合并。结构体数组（SoA, Structure of Arrays）会把各个字段分别存成独立的连续数组。这样一来，warp 在读取单一字段时更容易落到连续地址上，访存模式也更规整。

工程上，AoS 适合经常按对象整体读取的场景，SoA 更适合按单一字段批量处理的高吞吐场景。是否切换布局，取决于 kernel 的主要访问方向。

### Shared Memory

#### Bank 划分机制

为了提供与 L1 Cache 相当的高带宽，Shared Memory 在物理上被等分为多个能独立响应读写请求的并行存储体，即 Bank。

主流 CUDA 架构通常包含 32 个独立的 Bank，硬件内部的地址映射规律遵循以下线性公式：

$$
\text{bank} = \left(\frac{\text{address}}{4}\right) \bmod 32
$$

该公式建立在默认的 32-bit (即 4 字节) 访问粒度基础上。如果内存指令访问不同宽度的数值结构，Bank 的映射跨度也会随之产生变化。

#### Bank Conflict

当同一个 warp 发起 shared memory 访问时，性能主要取决于这些请求在各个 bank 上的分布方式。

- 多个线程访问不同 bank 时，硬件可以并行响应。
- 多个线程访问同一 bank 上的同一地址时，通常表现为 broadcast。
- 多个线程访问同一 bank 上的不同地址时，会发生 bank conflict。

bank conflict 会把原本可以并行完成的访问拆成多次服务，因此 shared memory 指令的有效延迟会上升。冲突线程越多，额外等待通常越明显。

#### 一维访问中的周期性冲突

在 reduction 或 scan 一类并行算法中，线程经常按倍增 stride 从 shared memory 读取数据。

当 stride 与 32 个 bank 之间形成固定整数关系时，多个线程的访问容易周期性落到同一组 bank 上，从而形成冲突。

???+ note "代码示例：一维访问中的冲突模式"

    ```cpp title="bank_conflict_1d.cu"
    __global__ void reduce_conflict(float* input, float* output) {
        __shared__ float s[1024];

        int tid = threadIdx.x;
        s[tid] = input[blockIdx.x * blockDim.x + tid];
        __syncthreads();

        for (int stride = 1; stride < blockDim.x; stride *= 2) {
            int index = (tid + 1) * stride * 2 - 1;

            if (index < blockDim.x) {
                s[index] += s[index - stride];
            }

            __syncthreads();
        }

        if (tid == 0) {
            output[blockIdx.x] = s[blockDim.x - 1];
        }
    }
    ```

    在步长倍增的归约或前缀和算法中，某些 stride 会让多个并发读取集中映射到少数几个 bank 上。

假设某次迭代中 Warp 内线程同时访问 `s[tid * 2]`，在 32-Bank 的标准设计下（考虑 4 字节的数据步长粒度），前若干线程的访问请求分布将出现规律性重叠：

| 线程 TID | 线性索引 | Bank 归属 |
| --- | --- | --- |
| 0 | 0 | 0 |
| 1 | 2 | 2 |
| 2 | 4 | 4 |
| 3 | 6 | 6 |
| ... | ... | ... |
| 16 | 32 | 0 |
| 17 | 34 | 2 |

当线性索引跨过 32 个元素后，bank 编号开始重复。若同一条指令中同时出现 `TID = 0` 和 `TID = 16` 这样的访问，两者都会落到 Bank 0，但地址不同，因此会发生冲突。

常见处理方式是 padding，即在逻辑索引到物理索引之间加入额外偏移，打破原有的周期映射关系：

$$
\text{physical\_index} = \text{logical\_index} + (\text{logical\_index} \gg 5)
$$

这个公式的含义是每 32 个逻辑元素插入一个空位，使原本重合的 bank 映射分散开。

!!! note "特殊情况：互质 stride 不需要 Padding"

    Padding 不是无条件有效的通用修正手段。

    对一维跨步访问 `s[tid * stride]` 而言，Bank 映射可写为：

    $$
    \text{bank} = (tid \times stride) \bmod 32
    $$

    当 $gcd(stride, 32) = 1$ 时，这个映射在一个 warp 内会遍历 32 个不同 bank。也就是说，若 $stride \in {3, 5, 7}$ 这类与 32 互质的跨度，本身就不会形成系统性 bank conflict。

    此时若机械地加入 padding，会改变原本一一对应的 Bank 映射关系，反而可能把无冲突访问改写成有冲突访问。因此，是否加入 padding 应根据实际 stride 与 bank 数的关系判断，而不是作为固定模板直接套用。

???+ note "代码示例：一维 padding 方案"

    ```cpp title="bank_padding_1d.cu"
    __device__ __forceinline__ int padded_index(int i) {
        return i + (i >> 5);
    }

    __global__ void reduce_padded(float* input, float* output) {
        __shared__ float s[1024 + 32];

        int tid = threadIdx.x;
        int pid = padded_index(tid);
        s[pid] = input[blockIdx.x * blockDim.x + tid];
        __syncthreads();

        for (int stride = 1; stride < blockDim.x; stride *= 2) {
            int index = (tid + 1) * stride * 2 - 1;

            if (index < blockDim.x) {
                int p_index = padded_index(index);
                int p_left = padded_index(index - stride);
                s[p_index] += s[p_left];
            }

            __syncthreads();
        }

        if (tid == 0) {
            output[blockIdx.x] = s[padded_index(blockDim.x - 1)];
        }
    }
    ```

#### 二维 Tile 的列访问冲突

二维 tile 是矩阵乘和矩阵转置中最常见的 shared memory 数据结构。

???+ note "代码示例：二维 Tile 声明"

    ```cpp title="tile_decl.cu"
    __shared__ float tile[32][32];
    ```

按行访问时，相邻线程通常落在连续地址上，访问模式较规整。按列访问时，warp 内相邻线程会跨过整行宽度，地址跨度随之增大。

若 tile 行宽为 32，并按列读取第 0 列，则线性索引可近似写成：

$$
\text{index} = t \times 32 + 0
$$

前几个线程的 Bank 映射为：

| 线程 t | 线性索引 | Bank 归属 |
| --- | --- | --- |
| 0 | 0 | 0 |
| 1 | 32 | 0 |
| 2 | 64 | 0 |
| 3 | 96 | 0 |

这说明多个线程会系统性落到同一个 bank 上，访问会被串行化。

常见处理方式是修改二维数组的 leading dimension，例如加入一列 padding，把行宽改成 33：

$$
\text{index} = t \times 33 + 0
$$

此时同一列向下的访问会被错开到不同 bank：

| 线程 t | 线性索引 | Bank 归属 |
| --- | --- | --- |
| 0 | 0 | 0 |
| 1 | 33 | 1 |
| 2 | 66 | 2 |
| 3 | 99 | 3 |

列访问不再持续压到同一个 bank 上，冲突会明显减轻。

???+ note "代码示例：二维 Tile 的冲突与修正"

    ```cpp title="bank_conflict_2d.cu"
    __global__ void transpose_tile(float* out, const float* in, int ld) {
        __shared__ float tile_bad[32][32];
        __shared__ float tile_good[32][33];

        int x = blockIdx.x * 32 + threadIdx.x;
        int y = blockIdx.y * 32 + threadIdx.y;

        tile_bad[threadIdx.y][threadIdx.x] = in[y * ld + x];
        tile_good[threadIdx.y][threadIdx.x] = in[y * ld + x];
        __syncthreads();

        out[x * ld + y] = tile_good[threadIdx.x][threadIdx.y];
    }
    ```

#### 一维与二维模式的冲突区别

- **一维访问**：冲突通常来自 stride 与 bank 数之间的周期关系。常见修正方式是在逻辑索引到物理索引之间加入周期性 padding。
- **二维访问**：冲突通常来自 tile 行宽与 bank 数相等。常见修正方式是在二维数组声明中增加一列 padding，例如把 `[32][32]` 改成 `[32][33]`。

!!! warning "Broadcast 认知区分"

    多个线程读取同一个 shared memory 地址时，访问模式通常表现为 broadcast。broadcast 不属于 bank conflict。

## 控制流发散

GPU 按 SIMT（Single Instruction, Multiple Threads）模型执行 warp 内线程。若同一个 warp 中的线程在分支上走向不同路径，硬件通常需要依次执行这些路径，并在每一段执行期间屏蔽其他线程。

这种现象称为 **warp divergence**。它会降低 warp 的有效活跃线程比例，是吞吐下降的常见来源之一。

warp divergence 常见于以下场景：

- **边界保护检查**：例如 `if (idx < n)`。这类分支通常只影响处理尾部数据的少数 warp。
- **数据依赖分支**：例如 `if (x[idx] > 0)`。若数据分布随机，这类分支会在整个 grid 中持续出现。
- **稀疏结构遍历**：例如图算法或稀疏矩阵计算中，相邻线程处理长度差异很大的邻接表或行。
- **动态循环条件**：同一 warp 内线程迭代次数不同，先完成的线程会等待其他线程。

分支对性能的影响取决于出现范围和发散程度。边界保护这类局部 divergence 往往不是主要瓶颈。数据依赖造成的大范围 divergence 更值得优先关注，常见处理方式包括数据重排、规整线程工作分布和合并状态分支。

???+ note "代码示例：数据相关分支与发散"

    ```cpp title="warp_divergence.cu"
    __global__ void threshold_relu(const float* x, float* y, int n, float t) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;

        if (idx < n) {
            if (x[idx] > t) {
                y[idx] = x[idx];
            } else {
                y[idx] = 0.0f;
            }
        }
    }
    ```

在这个例子里，如果一个 warp 内只有部分线程满足 `x[idx] > t`，硬件会分别执行两个分支路径。在每条路径上，只有满足该路径条件的线程处于活跃状态。

## 线程协作与同步机制

### `__syncthreads()`

`__syncthreads()` 是 block 级屏障，用于对齐同一个 block 内线程的执行阶段。

它常见于（1）保证 shared memory 写入完成后再读取（2）分阶段执行 block 内算法。

它的作用范围限定在 block 内。若算法需要 grid 级同步，常见做法是拆成多个 kernel，或使用更高层的同步机制。

### Warp Shuffle Primitive

warp shuffle primitive 允许同一个 warp 内线程直接交换寄存器值。它适用于协作范围明确限定在 warp 内的场景。

- `__shfl_sync(mask, value, src_lane)`：从 warp 内编号为 `src_lane` 的线程读取寄存器值，适合广播、重排和显式 lane-to-lane 拷贝。
- `__shfl_down_sync(mask, value, delta)`：从当前线程编号向下偏移 `delta` 的线程读取值，适合树形归约和分层累加。
- `__shfl_xor_sync(mask, value, lane_mask)`：按异或模式交换线程间数据，适合 butterfly 结构，例如某些 scan、归约和配对交换模式。

其中 `mask` 是一个 32 位的位掩码，表示 warp 内哪些线程是活跃的。只有在 `mask` 中对应位为 1 的线程才会参与 shuffle 操作。

这组三个接口主要用于：（1）warp 内归约（2）warp 内 scan（3）小范围数据重排。

??? example "Shuffle 模式示例"

    下面用 4 个 lane 的最小示意说明三种交换模式。设每个 lane 的初始值分别是 `a0`、`a1`、`a2`、`a3`。

    | 函数 | 参数 | lane 0 读到 | lane 1 读到 | lane 2 读到 | lane 3 读到 | 
    | --- | --- | --- | --- | --- | --- |
    | `__shfl_sync(mask, v, 2)` | `src_lane = 2` | `a2` | `a2` | `a2` | `a2` | 
    | `__shfl_down_sync(mask, v, 1)` | `delta = 1` | `a1` | `a2` | `a3` | `undefined` or `reserved` | 
    | `__shfl_xor_sync(mask, v, 1)` | `lane_mask = 1` | `a1` | `a0` | `a3` | `a2` | 

    若把 `lane_mask` 改成 `2`，则 `__shfl_xor_sync` 的配对会变成：[0, 2] 和 [1, 3]。

    这类模式常见于多轮归约和 scan，因为每一轮都可以按照固定的二进制位配对线程。


???+ note "Warp 内归约"

    ```cpp title="warp_reduce.cu"
    __device__ float warp_sum(float v) {
        unsigned mask = 0xffffffffu;

        for (int offset = 16; offset > 0; offset >>= 1) {
            v += __shfl_down_sync(mask, v, offset);
        }

        return v;
    }
    ```

??? example "Shuffle 归约示意"

    下面用 8 个 lane 的简化示意说明 `__shfl_down_sync` 的两轮归约过程。设初始值分别为：

    $$
    [a_0, a_1, a_2, a_3, a_4, a_5, a_6, a_7]
    $$

    第一轮使用 `offset = 4`：

    | lane | 读入值 | 更新后值 |
    | --- | --- | --- |
    | 0 | `a4` | `a0 + a4` |
    | 1 | `a5` | `a1 + a5` |
    | 2 | `a6` | `a2 + a6` |
    | 3 | `a7` | `a3 + a7` |
    | 4 | 超出本轮有效归约范围 | 保持原值或由调用侧忽略 |
    | 5 | 超出本轮有效归约范围 | 保持原值或由调用侧忽略 |
    | 6 | 超出本轮有效归约范围 | 保持原值或由调用侧忽略 |
    | 7 | 超出本轮有效归约范围 | 保持原值或由调用侧忽略 |

    第二轮使用 `offset = 2`：

    | lane | 读入值 | 更新后值 |
    | --- | --- | --- |
    | 0 | `a2 + a6` | `a0 + a4 + a2 + a6` |
    | 1 | `a3 + a7` | `a1 + a5 + a3 + a7` |
    | 2 | 超出本轮有效归约范围 | 保持原值或由调用侧忽略 |
    | 3 | 超出本轮有效归约范围 | 保持原值或由调用侧忽略 |

    继续执行 `offset = 1` 后，`lane 0` 就会拿到这一组 lane 的最终归约结果。32-lane warp 的写法与这个过程完全同构，只是轮次数更多。

相较 shared memory 方案，shuffle 的主要特点是显式的寄存器级数据交换，block级同步需求更少，且数据始终留在寄存器中。这使得它在 warp 内归约和 scan 场景中非常高效。但是要求只能在 warp 内使用，并且需要正确处理活跃线程掩码以避免未定义行为。

在很多归约场景中，更常见的工程写法是分层组合 shuffle、shared memory 和 atomic：

1. 先在每个 warp 内用 shuffle 完成局部归约。
2. 再把每个 warp 的结果写到 shared memory。
3. 由一个 warp 继续归约这些中间结果。
4. 最后只对全局计数器执行一次 atomic。

???+ note "Shuffle + Shared Memory + Atomic 的分层归约"

    ```cpp title="hierarchical_reduction.cu"
    __device__ int warp_sum_int(int v) {
        unsigned mask = 0xffffffffu;

        for (int offset = 16; offset > 0; offset >>= 1) {
            v += __shfl_down_sync(mask, v, offset);
        }

        return v;
    }

    __global__ void count_positive_hierarchical(const float* x, int* counter, int n) {
        __shared__ int warp_sums[32];

        int tid = threadIdx.x;
        int idx = blockIdx.x * blockDim.x + tid;
        int lane = tid & 31;
        int warp_id = tid >> 5;

        int v = (idx < n && x[idx] > 0.0f) ? 1 : 0;
        v = warp_sum_int(v);

        if (lane == 0) {
            warp_sums[warp_id] = v;
        }
        __syncthreads();

        if (warp_id == 0) {
            int block_sum = (tid < (blockDim.x >> 5)) ? warp_sums[lane] : 0;
            block_sum = warp_sum_int(block_sum);

            if (lane == 0) {
                atomicAdd(counter, block_sum);
            }
        }
    }
    ```

这个模式的关键点包括：

- warp 内交换尽量留在寄存器中完成。
- shared memory 只用于保存每个 warp 的中间结果。
- 全局 atomic 从“每线程一次”降到“每 block 一次”。

这类写法通常比纯 atomic 或纯 shared memory 归约更接近现代 CUDA kernel 的常见实现方式。

### Cooperative Groups

cooperative groups 为 CUDA 提供更清晰的协作组抽象，用于显式表达线程协作边界。

常见用法包括：

- `cg::this_thread_block()` 表示当前 block。
- `cg::tiled_partition<32>(block)` 将 block 划成多个固定大小的 tile。

它的主要价值体现在：

- 让同步范围更明确。
- 让 warp-tile 协作代码更易读。
- 减少手工假设“某几行线程天然是一组”的隐式写法。

### Atomic 的作用与代价

atomic 用于解决并发的 read-modify-write 冲突。例如多个线程都要更新同一个计数器时，`atomicAdd` 可以保证更新原子性。

atomic 的主要代价来自热点地址争用。若大量线程同时更新同一个地址，硬件会把这些更新串行化处理，形成 **atomic hotspot**。

???+ note "直接 atomic 与分层归约"

    ```cpp title="atomic_hotspot.cu"
    __global__ void count_positive_naive(const float* x, int* counter, int n) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;

        if (idx < n && x[idx] > 0.0f) {
            atomicAdd(counter, 1);
        }
    }

    __global__ void count_positive_block_reduce(const float* x, int* counter, int n) {
        __shared__ int local_sum[256];
        int tid = threadIdx.x;
        int idx = blockIdx.x * blockDim.x + tid;

        local_sum[tid] = (idx < n && x[idx] > 0.0f) ? 1 : 0;
        __syncthreads();

        for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
            if (tid < stride) {
                local_sum[tid] += local_sum[tid + stride];
            }
            __syncthreads();
        }

        if (tid == 0) {
            atomicAdd(counter, local_sum[0]);
        }
    }
    ```

    第二种写法保留了 atomic 语义，同时把更新频率从“每个正值一次”降到“每个 block 一次”，通常能显著降低争用。

如果 block 内归约进一步改用 warp shuffle，再配合 shared memory 只保存 warp 级中间结果，还会进一步减少：

- shared memory 读写次数。
- `__syncthreads()` 次数。
- block 内归约阶段的控制开销。

## Stream

### Stream 的作用

stream 是 CUDA 中的执行队列。一个 stream 内的 kernel launch、`cudaMemcpyAsync`、event record 和 wait 按提交顺序推进。多个 stream 之间不存在显式依赖时，运行时可以把这些操作分发到不同的硬件执行资源上并发执行。

### Overlap 的成立条件

典型的 copy-compute overlap 目标，是让 H2D 拷贝、kernel 计算和 D2H 拷贝在时间线上形成部分重叠。这个目标依赖三个条件同时成立：操作被提交到彼此独立的非默认 stream，数据传输使用异步拷贝接口，host 侧缓冲区使用 pinned memory。设备侧还需要具备相应的 copy engine 与并发执行能力，否则不同阶段仍会串行推进。

overlap 的收益来自流水化执行。前一批次进入 kernel 计算后，下一批次可以开始 H2D 拷贝，上一批次也可以继续 D2H 回传。这样可以把数据传输延迟部分隐藏在计算阶段内部，整体吞吐通常高于单一 stream 下的顺序执行。

??? note "示意图：两个 Stream 的 Copy-Compute Overlap"

    ```mermaid
    timeline
        title Stream Overlap 示例
        section Stream 0
          H2D batch0 : 0, 2
          Kernel batch0 : 2, 6
          D2H batch0 : 6, 8
        section Stream 1
          H2D batch1 : 2, 4
          Kernel batch1 : 4, 8
          D2H batch1 : 8, 10
    ```

### Pinned Memory

默认的 pageable host memory 在发起异步拷贝时，运行时经常需要额外的中转步骤。pinned memory 会把 host 页面固定下来，使 DMA 可以直接从这段内存发起传输。异步拷贝和 copy-compute overlap 因此获得更稳定的执行条件。

从工程角度看，pinned memory 对应的是更高的传输确定性和更低的额外管理开销。代价在于固定页数量需要受控，过量使用会增加 host 内存管理压力。

### 常见误区

常见问题集中在依赖关系和数据路径两侧。只增加 stream 数量而继续使用同步拷贝，执行路径仍然会在 host 侧串行化。改用异步接口但继续使用 pageable memory，传输阶段仍然可能退回到额外中转。kernel 规模过小时，launch 开销和调度开销会压缩 overlap 空间，吞吐提升也会随之减弱。

另一个常见问题是隐式同步。默认 stream、跨 stream 的 event 依赖、共享缓冲区复用不当，都会让时间线上原本可以并发的阶段重新串行化。stream 的优化对象是依赖图和执行流水，而不是简单增加队列数量。

## Unified Memory

Unified Memory 提供统一虚拟地址空间，使 host 和 device 可以通过统一指针访问同一块逻辑内存。

它的直接价值是降低数据管理复杂度，适用于原型验证、算法实验以及 CPU/GPU 共享复杂数据结构的场景。从性能角度看，Unified Memory 的核心分析对象始终是页迁移行为，而不是指针形式本身。

### 按需迁移与 Page Fault

当 GPU 首次访问当前不在 device 上的页面时，可能触发 page fault，并把数据迁移到 device memory。这类迁移会带来额外延迟。

如果访问模式在 CPU 和 GPU 间反复切换，同一页可能来回迁移，性能会明显下降。

### Prefetch 与 Advise

为了降低按需迁移带来的不确定性，运行时提供了 `cudaMemPrefetchAsync` 和 `cudaMemAdvise`。前者用于显式提前迁移数据，后者用于向运行时提供访问倾向信息。两者的目标都是让页迁移时机和驻留位置更接近程序的真实访问模式。

???+ note "代码示例：Unified Memory 预取"

    ```cpp title="unified_memory_prefetch.cu"
    float* x = nullptr;
    cudaMallocManaged(&x, n * sizeof(float));

    initialize_on_cpu(x, n);

    int device = 0;
    cudaMemPrefetchAsync(x, n * sizeof(float), device);
    saxpy<<<grid, block>>>(n, a, x, x);
    ```

这种写法会把迁移时机从首次访问触发改成显式预取，访问延迟因此更可控。

!!! note "Unified Memory 的适用场景"

    Unified Memory 适合原型开发、复杂数据结构和先保证正确性再逐步优化的路径。对延迟和带宽高度敏感的关键路径，工程实现通常仍采用显式数据管理。

## 现代 CUDA 常见进阶特性

### `cp.async`

`cp.async` 的核心目的，是让从 global memory 到 shared memory 的搬运与计算形成流水线重叠。

传统 tiled kernel 往往采用“先加载 tile，再同步，再计算当前 tile，最后进入下一轮加载”的顺序结构。`cp.async` 允许把下一块 tile 的加载与当前 tile 的计算交织起来，从而把一部分搬运延迟隐藏在计算阶段内部。

???+ note "伪代码示例：双缓冲流水线"

    ```cpp title="cp_async_pipeline.cu"
    for (int k = 0; k < num_tiles; ++k) {
        async_copy(global_tile[k + 1], shared_buffer[next]);
        compute(shared_buffer[current]);
        wait_for_async_copy();
        swap(current, next);
    }
    ```

`cp.async` 适合 tiled GEMM、convolution 以及分阶段加载和计算交替出现的 kernel。这类 kernel 具有稳定的 tile 边界和明确的流水线阶段，能够承接双缓冲或多阶段缓冲结构。

---

### Cooperative Groups

在复杂 kernel 中，cooperative groups 常与 warp-level primitive 一起使用，用于明确表达 tile 级协作、block 级协作和同步边界。它的价值在于把协作范围显式写进代码结构，使 warp 级操作、tile 级操作和 block 级操作保持清晰边界，减少协作范围写错后的同步错误和数据竞争。

---

### CUDA Graph

当程序需要重复执行同一套 kernel 与 memcpy 组合时，CPU 端逐次提交这些操作会产生固定开销。若单个 kernel 本身很小，这部分 launch overhead 可能变得显著。

CUDA Graph 通过把一组操作组织为一个可重复执行的图，减少重复提交成本。它适合固定拓扑的推理循环、重复批处理流水线以及小 kernel 数量很多的场景。常见入口包括显式构建 graph，以及从 stream capture 生成 graph。

???+ note "代码示例：通过 Stream Capture 构建 Graph"

    ```cpp title="cuda_graph_capture.cu"
    cudaStream_t stream;
    cudaStreamCreate(&stream);

    cudaGraph_t graph;
    cudaGraphExec_t graph_exec;

    cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);
    cudaMemcpyAsync(d_x, h_x, bytes, cudaMemcpyHostToDevice, stream);
    kernel1<<<grid, block, 0, stream>>>(d_x, d_y);
    kernel2<<<grid, block, 0, stream>>>(d_y, d_z);
    cudaMemcpyAsync(h_z, d_z, bytes, cudaMemcpyDeviceToHost, stream);
    cudaStreamEndCapture(stream, &graph);

    cudaGraphInstantiate(&graph_exec, graph, nullptr, nullptr, 0);

    for (int iter = 0; iter < steps; ++iter) {
        cudaGraphLaunch(graph_exec, stream);
    }
    ```

这个模式适合重复执行固定拓扑流水线的场景。与每轮重新发起多次 kernel launch 相比，graph 的主要价值在于降低 CPU 提交成本并复用固定依赖结构。

---

### NCCL

NCCL 是 NVIDIA 提供的多 GPU collective communication 库，常与 CUDA stream 配合使用。

它覆盖 `all-reduce`、`all-gather`、`reduce-scatter` 和 `broadcast` 等常见 collective。NCCL 与 CUDA stream 配合后，通信操作可以进入与 kernel 相同的依赖链，以便在满足依赖关系时尽量与计算重叠。例如在训练场景中，局部梯度计算完成后可以立即进入梯度归约，同时继续推进后续局部计算或参数更新准备。

???+ note "代码示例：在 Stream 上提交 NCCL All-Reduce"

    ```cpp title="nccl_allreduce.cu"
    cudaStream_t stream;
    cudaStreamCreate(&stream);

    ncclComm_t comm;
    // comm 初始化略

    local_backward<<<grid, block, 0, stream>>>(grad, partial_grad);
    ncclAllReduce(
        partial_grad,
        partial_grad,
        count,
        ncclFloat32,
        ncclSum,
        comm,
        stream
    );
    optimizer_step<<<grid, block, 0, stream>>>(partial_grad, weight);
    ```

这里的关键点是 NCCL 操作也可以进入 CUDA stream，因此它能与计算一起纳入同一个依赖链中管理。真正能否形成良好的 overlap，还取决于通信拓扑、张量大小和前后 kernel 的耗时分布。

NCCL 在工程上的作用，是把跨 GPU 的 collective 通信从普通 kernel 计算路径中分离出来。单 GPU CUDA 学习通常只涉及本地 kernel 和本地内存层次，多 GPU 数据交换则由 NCCL 负责组织。

---

### NCCL Collective 与训练 / 推理场景

不同 collective 的区别，主要体现在结果分布方式和后续算子需求上。

| Collective | 结果分布 | 训练中的常见用途 | 推理中的常见用途 |
| --- | --- | --- | --- |
| `all-reduce` | 每个 rank 都得到完整归约结果 | 数据并行训练中的梯度同步；优化器更新前获得完整梯度 | 多副本统计量同步；少量跨卡状态对齐 |
| `reduce-scatter` | 每个 rank 得到归约结果的一部分分片 | ZeRO、分片优化器、分片梯度场景；把同步和分片分发合并 | 张量并行或专家并行中的分片输出归约 |
| `all-gather` | 每个 rank 收集所有 rank 的分片 | 参数、激活或分片梯度的重组；与 `reduce-scatter` 形成配对 | 张量并行推理中的分片激活拼接；KV 或中间表示重组 |

训练场景里，`all-reduce` 最常见于数据并行梯度同步，语义直接，对优化器最友好。

训练系统进一步追求显存和通信效率时，常把一次完整的 `all-reduce` 改写成 `reduce-scatter`、本地分片更新或后续分片计算、按需 `all-gather` 的组合结构。这种组织方式常见于参数分片、优化器状态分片和更大规模的并行训练。

推理场景中，`all-gather` 更常见于张量并行输出拼接，`reduce-scatter` 更常见于分片结果归约后直接保留本地分片，`all-reduce` 则更多出现在需要全副本一致结果的小规模同步路径中。

??? note "统一时序图：Copy、Compute、Collective 与 Graph 的关系"

    下图把一次典型训练或推理批次中的三类操作放到同一条时间线上。`H2D / D2H` 表示拷贝，`Forward / Backward / Optimizer` 表示计算，`NCCL All-Reduce` 表示多 GPU collective。若这组依赖关系在每个 step 中都保持不变，就适合进一步用 CUDA Graph 做 capture 和重复 launch。

    ```mermaid
    timeline
        title 单个 Step 中的 Copy、Compute、Collective 统一视图
        section Stream 0
          H2D batch_k : 0, 2
          Forward batch_k : 2, 6
          Backward local grad_k : 6, 9
          NCCL All-Reduce grad_k : 9, 11
          Optimizer step_k : 11, 12
          D2H result_k : 12, 13
        section Stream 1
          H2D batch_k+1 : 6, 8
          Forward batch_k+1 : 8, 12
        section Graph View
          Capture fixed DAG once : 0, 1
          Re-launch captured step graph : 1, 13
    ```

    这张图表达两个重点。stream 负责安排 copy、compute 和 collective 在时间上的依赖与并行关系。CUDA Graph 负责把这条固定依赖链整体缓存下来，减少每一轮都重新提交各个节点的 CPU 开销。

## 性能分析方法与调优顺序

CUDA 调优最好遵循“先定位瓶颈类型，再局部优化”的顺序。

**第一步：判断是 Compute-Bound 还是 Memory-Bound。**

如果一个 kernel 的算术强度低、global memory 吞吐高而 ALU 利用率有限，那么它更可能是 memory-bound。

如果一个 kernel 的访存模式已经很规整，但 Tensor Core 或普通算术单元利用率仍不足，则更可能是 compute-bound。

**第二步：检查访存模式。**

优先检查 global memory 是否 coalesced，shared memory 是否存在 bank conflict，以及数据是否能够在寄存器或 shared memory 中复用。这一阶段处理的是最典型的结构性瓶颈。

**第三步：检查控制流和同步。**

继续检查 warp divergence 是否显著，`__syncthreads()` 是否过于频繁，以及 atomic hotspot 是否存在。控制流和同步问题会直接降低 warp 有效执行比例，并放大等待开销。

**第四步：检查资源压力。**

再检查每线程寄存器使用量、每 block shared memory 使用量，以及 occupancy 是否被资源约束压低。资源压力过高时，局部优化可能会被寄存器溢出、shared memory 配额或并发度下降抵消。

---

**常见工具**

`Nsight Compute` 常用于查看 achieved occupancy、warp stall 原因、global memory throughput、shared memory replay 和 branch efficiency。这些指标用于判断问题更接近带宽、同步、控制流还是资源约束。

编译阶段的 `ptxas` 输出则常用于查看每线程寄存器使用量、static shared memory 和 spill 到 local memory 的情况。它更适合在实现阶段快速确认资源占用是否已经偏离预期。

---

**常见性能问题与观测指标。**

| 现象 | 常见指标入口 | 常见修正方向 |
| --- | --- | --- |
| global memory 访问不规整 | global load/store efficiency、dram throughput | 调整线程映射、改进 coalescing、重排数据布局 |
| shared memory bank conflict | shared replay、shared memory stall | 调整 tile leading dimension、加入 padding、修改 shared memory 访问模式 |
| warp divergence | branch efficiency、warp execution efficiency | 调整分支布局、压缩数据相关分支、重构索引模式 |
| atomic hotspot | atomic throughput、warp stall related to atomic | block 内先归约、分层累加、降低热点地址争用 |
| 寄存器压力过高 | achieved occupancy、register per thread、local spill | 调整 block size、控制临时变量规模、检查 spilling |
| 同步开销偏高 | barrier stall、warp stall breakdown | 减少不必要的 `__syncthreads()`、使用 warp 级原语替代部分 block 级同步 |

!!! note "一个更稳妥的调优顺序"

    1. 先确认正确性。
    2. 再确认瓶颈是计算、带宽还是同步。
    3. 然后处理最明显的结构性问题，例如不合并访存、bank conflict、atomic hotspot。
    4. 最后再讨论 `launch_bounds`、`maxrregcount`、更激进的 block size 调整。

*[ SM ]: Streaming Multiprocessor
*[ CTA ]: Cooperative Thread Array，通常可视为 block
*[ SIMT ]: Single Instruction, Multiple Threads
*[ H2D ]: Host to Device
*[ D2H ]: Device to Host
*[ UM ]: Unified Memory
*[ WMMA ]: Warp Matrix Multiply Accumulate
*[ GEMM ]: General Matrix Multiply
*[ NCCL ]: NVIDIA Collective Communications Library
