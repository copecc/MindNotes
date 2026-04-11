---
title: Popcount
tags:
  - C++
  - BitOperation
  - Benchmark
---

# 经典与进阶 Popcount 算法笔记

!!! abstract "摘要"

    本文系统性说明 popcount 的实现机制，覆盖基础位计数方法与进阶实现（Parallel Fold、Hacker's Delight、HAKMEM 169、Lookup Table）。文中重点讨论 32 位特化实现如何扩展到泛型无符号整数接口，并给出全类型正确性测试与统一性能测试。所有结果均以 C++20 标准库 `std::popcount` 作为参考基线。

## 机制解析与工程目标

> 参考资料：本文部分位运算技巧与推导过程参考了 [popcount 算法分析](https://zhuanlan.zhihu.com/p/341488123){target=_blank}。

在常见 C++ 实现中，工程问题通常来自位宽假设与接口语义不一致：

- 多个经典算法按 `uint32_t` 编写，仅对 32 位输入具备直接正确性保证。
- 直接将该类实现应用于 `uint64_t` 或其他无符号类型时，可能出现结果错误或语义边界不明确。

本文采用以下目标：

- 对外提供统一泛型接口。
- 对内复用稳定的 32 位内核实现。
- 使用跨类型正确性测试验证行为一致性。
- 使用统一 benchmark 对比不同算法在不同位宽下的性能。

## strict_unsigned_integral 定义说明


???+ note "`strict_unsigned_integral`"

    本文中的多数接口采用 `strict_unsigned_integral auto` 作为形参约束。其定义如下：

    ```cpp
    template <typename T, typename... U>
    concept neither = (!std::same_as<T, U> && ...);

    template <typename T>
    concept strict_unsigned_integral =
      std::unsigned_integral<T> && neither<T, bool, char, char8_t, char16_t, char32_t, wchar_t>;
    ```

该定义包含两层约束：

- `std::unsigned_integral<T>`：确保类型是无符号整数，并具备按位运算语义。
- `neither<...>`：额外排除 `bool` 与字符类型，避免将字符编码或逻辑值误作为位计数输入。

采用该约束的主要原因：

- 防止接口被不符合算法预期的数据类型调用。
- 使模板错误在编译期暴露，减少运行时歧义。
- 保持测试与 benchmark 的输入域一致，确保结果可比较。

在该约束下，`uint8_t`、`uint16_t`、`uint32_t`、`uint64_t`、`size_t` 等常见无符号整数类型可直接使用。

## 经典位计数基础算法

这组算法天然适合泛型输入，不依赖 32 位专用掩码。

???+ note "基础算法"

    ```cpp title="basic_popcount.cpp"
    constexpr bool has_single_bit(strict_unsigned_integral auto x) noexcept {
      return x && !(x & (x - 1));
    }

    constexpr int iterated_popcnt(strict_unsigned_integral auto n) {
      int count = 0;
      for (; n; n >>= 1) {
        count += n & 1;
      }
      return count;
    }

    constexpr int sparse_popcnt(strict_unsigned_integral auto n) {
      int count = 0;
      while (n) {
        ++count;
        n &= n - 1;
      }
      return count;
    }

    constexpr int dense_popcnt(strict_unsigned_integral auto n) {
      int count = sizeof(n) * 8;
      n ^= static_cast<decltype(n)>(-1);
      while (n) {
        --count;
        n &= n - 1;
      }
      return count;
    }
    ```

说明：

- `iterated_popcnt` 逐位右移并累加最低位，逻辑直接，循环次数与位宽线性相关。
- `sparse_popcnt` 采用 Brian Kernighan 方法，每轮消去一个最低位 1，时间复杂度与置位数相关。
- `dense_popcnt` 先取反再执行 Kernighan 过程，适用于置位密集输入。

## 进阶位计数与 32 位分块适配

`parallel_popcnt`、`nifty_popcnt`、`hacker_popcnt`、`hakmem_popcount` 等写法本质依赖 32 位常量。为了在更宽类型中复用其性能特征，可采用“32 位内核 + 分块分发器”的统一结构。

### 分块分发器

策略如下：

- 若类型位宽不超过 32 位，直接转换后调用 32 位内核。
- 若类型位宽超过 32 位，按 32 位块拆分并对各块结果求和。

???+ note "32 位分块分发器"

    ```cpp title="apply_32bit_chunks.cpp"
    template <auto Func>
    constexpr int apply_32bit_chunks(strict_unsigned_integral auto n) {
      if constexpr (sizeof(n) <= sizeof(uint32_t)) {
        return Func(static_cast<uint32_t>(n));
      } else {
        constexpr size_t chunks = sizeof(n) / sizeof(uint32_t);
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return (Func(static_cast<uint32_t>(n >> (Is * 32))) + ...);
        }(std::make_index_sequence<chunks>{});
      }
    }
    ```

关键机制：

- `if constexpr` 在编译期选择分支，无运行时判断开销。
- `std::index_sequence` 与折叠表达式在编译期展开所有分块计算。

### 模板参数包展开

`std::index_sequence` 与 `std::make_index_sequence` 是 C++14 引入的编译期整数序列工具，常用于模板参数包展开。

定义关系可以概括为：

- `std::index_sequence<Is...>`：仅表示一组 `size_t` 非类型模板参数。
- `std::make_index_sequence<N>`：生成 `std::index_sequence<0, 1, ..., N-1>`。

例如：

- `std::make_index_sequence<4>` 对应 `std::index_sequence<0, 1, 2, 3>`。

在本文的分块实现中，`chunks` 表示 32 位分块数量，`std::make_index_sequence<chunks>{}` 负责生成所有分块索引，再由泛型 lambda 以 `Is...` 接收并在折叠表达式中展开：

$$
\sum_{i=0}^{\text{chunks}-1} \text{Func}\big(\text{chunk}(n, i)\big)
$$

这种写法的工程价值在于：

- 分块数量在编译期确定，循环可被静态展开。
- 算法主体仅描述“按索引处理每一块”，避免手写重复代码。
- 与 `if constexpr` 配合后，不同位宽路径都能保持零额外分派开销。

## 经典 32 位进阶算法

以下算法保留 32 位内核版本，并通过泛型包装器导出统一接口。

### Parallel Fold

该方法按分治思想逐级聚合位计数：先聚合相邻 1 位，再聚合相邻 2 位、4 位，最终覆盖 32 位范围。其实现无分支，主要由位运算与加法组成。

???+ note "Parallel Fold"

    ```cpp title="parallel_popcnt.cpp"
    #define POW2(c)     (1U << (c))
    #define MASK(c)     (static_cast<uint32_t>(-1) / (POW2(POW2(c)) + 1U))
    #define COUNT(x, c) (((x) & MASK(c)) + (((x) >> (POW2(c))) & MASK(c)))

    constexpr int parallel_popcnt_32(uint32_t n) {
      n = COUNT(n, 0);
      n = COUNT(n, 1);
      n = COUNT(n, 2);
      n = COUNT(n, 3);
      n = COUNT(n, 4);
      return n;
    }

    #undef COUNT
    #undef MASK
    #undef POW2

    constexpr int parallel_popcnt(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<parallel_popcnt_32>(n);
    }
    ```

??? example "Example"

    设 8 位示意输入 `n = 0b1011'0100`（十进制 180），其置位位点为 bit7、bit5、bit4、bit2。

    - 第 1 轮 `COUNT(n, 0)`：按 2 位分组统计。
      - 10 -> 1，11 -> 2，01 -> 1，00 -> 0。
      - 此时每个 2 位字段保存该字段内的置位数。
    - 第 2 轮 `COUNT(n, 1)`：把两个相邻 2 位字段合并为 4 位字段计数。
      - 高 4 位 1011 的计数为 3。
      - 低 4 位 0100 的计数为 1。
    - 后续 `COUNT(n, 2..4)` 在 32 位实现中继续做层级合并，最终在低位区域得到全字计数。

    最终结果为 `parallel_popcnt_32(0b1011'0100) = 4`。

!!! warning "Parallel Fold 扩展到 64/128 位的实现建议"

    可以为 64 位或 128 位实现原生 Parallel Fold 内核，但不建议在当前 32 位宏实现上直接追加 `COUNT(n, 5)`、`COUNT(n, 6)`。

    - 现有宏使用 `1U` 与 `uint32_t` 语义，位宽扩展后容易触发移位越界、常量截断或未定义行为。
    - 掩码生成与移位表达式必须保证“移位位数严格小于目标类型位宽”。
    - 64 位可通过独立常量与独立内核实现，128 位通常依赖编译器扩展类型，可移植性与维护成本更高。

### Nifty Popcnt

该算法通过常数除法构造掩码并逐级累加，最后通过 `% 255` 提取汇总结果。

???+ note "Nifty Popcnt"

    ```cpp title="nifty_popcnt.cpp"
    constexpr int nifty_popcnt_32(uint32_t n) {
      constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
      n = (n & (max / 3))  + ((n >> 1) & (max / 3));
      n = (n & (max / 5))  + ((n >> 2) & (max / 5));
      n = (n & (max / 17)) + ((n >> 4) & (max / 17));
      return n % 255;
    }

    constexpr int nifty_popcnt(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<nifty_popcnt_32>(n);
    }
    ```

??? example "Example"

    设输入 `n = 0b1011'0100`，且 `max = 0xFFFF'FFFF`。

    - `max / 3 = 0x5555'5555`：提取奇偶位并相加，得到每 2 位分组计数。
    - `max / 5 = 0x3333'3333`：继续合并为每 4 位分组计数。
    - `max / 17 = 0x0F0F'0F0F`：继续合并为每 8 位分组计数。
    - `% 255`：把分组和规约到低位结果。

    对于该输入，高 4 位 1011 贡献 3，低 4 位 0100 贡献 1，故总和为 4，即 `nifty_popcnt_32(0b1011'0100) = 4`。

### Hacker's Delight

该实现来源于《Hacker's Delight》，通过更紧凑的位运算路径减少指令数量。

???+ note "Hacker's Delight"

    ```cpp title="hacker_popcnt.cpp"
    constexpr int hacker_popcnt_32(uint32_t n) {
      n -= (n >> 1) & 0x55'55'55'55;
      n  = (n & 0x33'33'33'33) + ((n >> 2) & 0x33'33'33'33);
      n  = ((n >> 4) + n) & 0x0F'0F'0F'0F;
      n += n >> 8;
      n += n >> 16;
      return n & 0x00'00'00'3F;
    }

    constexpr int hacker_popcnt(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<hacker_popcnt_32>(n);
    }
    ```

??? example "Example"

    设 8 位示意输入 `n = 0b1011'0100`。

    - 第一步：`n -= (n >> 1) & 0x55...`。
      - 每个 2 位字段变为该字段内置位数，等价于把 10/11/01/00 映射到 1/2/1/0。
    - 第二步：`(n & 0x33...) + ((n >> 2) & 0x33...)`。
      - 把相邻两个 2 位字段合并为 4 位字段计数，得到高半字节 3、低半字节 1。
    - 第三步：`((n >> 4) + n) & 0x0F...` 后，再执行 `n += n >> 8`、`n += n >> 16`。
      - 把局部计数逐级聚合到低位。
    - 第四步：`n & 0x3F` 取 32 位结果范围内的有效计数值。

    最终 `hacker_popcnt_32(0b1011'0100) = 4`。

### HAKMEM 169

该方法使用三位分组与模运算完成汇总，源自 MIT AI Lab 的 HAKMEM 文档。

???+ note "HAKMEM 169"

    ```cpp title="hakmem_popcount.cpp"
    constexpr int hakmem_popcount_32(uint32_t n) {
      uint32_t tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
      return ((tmp + (tmp >> 3)) & 030707070707) % 63;
    }

    constexpr int hakmem_popcount(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<hakmem_popcount_32>(n);
    }
    ```

??? example "Example"

    设输入 `n = 0b1011'0100`。

    - 先计算
      `tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111)`。
      该步骤利用八进制掩码在并行位切片上完成“三位分组计数”的预处理。
    - 再计算 `((tmp + (tmp >> 3)) & 030707070707)`。
      该步骤把相邻分组的计数进行局部聚合。
    - 最后 `% 63`，将聚合结果规约到最终计数。

    对该输入，输出为 `hakmem_popcount_32(0b1011'0100) = 4`。

### Lookup Table

查表法适合按字节展开，能自然支持不同位宽。

???+ note "Lookup Table"

    ```cpp title="lookup_popcnt.cpp"
    namespace detail {
    #define BIT2(n) n, n + 1, n + 1, n + 2
    #define BIT4(n) BIT2(n), BIT2(n + 1), BIT2(n + 1), BIT2(n + 2)
    #define BIT6(n) BIT4(n), BIT4(n + 1), BIT4(n + 1), BIT4(n + 2)
    #define BIT8(n) BIT6(n), BIT6(n + 1), BIT6(n + 1), BIT6(n + 2)

    inline constexpr uint8_t lookup_table[256] = {BIT8(0)};

    #undef BIT8
    #undef BIT6
    #undef BIT4
    #undef BIT2
    } // namespace detail

    constexpr int lookup_popcnt(strict_unsigned_integral auto n) {
      constexpr size_t bytes = sizeof(n);
      return [&]<size_t... Is>(std::index_sequence<Is...>) {
        return (detail::lookup_table[(n >> (Is * 8)) & 0xFF] + ...);
      }(std::make_index_sequence<bytes>{});
    }
    ```

??? example "Example"

    对 32 位输入 `n = 0xF0'0F'00'03`：

    - 根据 `(n >> (Is * 8)) & 0xFF`，依次提取字节。
      - Is = 0：0x03。
      - Is = 1：0x00。
      - Is = 2：0x0F。
      - Is = 3：0xF0。
    - 查询 `lookup_table`。
      - `lookup_table[0x03] = 2`。
      - `lookup_table[0x00] = 0`。
      - `lookup_table[0x0F] = 4`。
      - `lookup_table[0xF0] = 4`。
    - 使用折叠表达式求和：2 + 0 + 4 + 4 = 10。

    因此 `lookup_popcnt(0xF0'0F'00'03u) = 10`。

## std::popcount

在 C++20 中，`std::popcount` 被纳入 `<bit>` 头文件，提供了一个标准化的接口。其实现通常会根据平台特性选择最优路径：

- 在支持硬件指令（如 x86 的 `POPCNT`）的平台上，直接映射到该指令。
- 在不支持硬件指令的平台上，可能采用上述某种经典算法或其变体实现。

## 性能结果的典型解读

### std::popcount

- 通常可作为最强基线。
- 在支持硬件指令的平台上，通常可直接映射到 `POPCNT`。

### Lookup Table

- 在 `uint8_t` 场景常表现较好。
- 在 `uint64_t` 场景需要多次查表与位操作，优势可能下降。

### Hacker / Nifty / Parallel

- 主要由位运算组成，性能波动相对较小。
- 在更宽位数场景下，通过 32 位分块仍可保持较高吞吐。

### Sparse / Iterated / Dense

- 依赖循环，性能更容易受输入分布影响。
- 在随机 `uint64_t` 数据上，循环次数通常高于 `uint8_t` 场景。

## 完整代码

以下代码包含 Concepts 约束、全部算法实现、分块适配器、正确性测试与性能测试框架。

??? note "完整代码"

    ```cpp title="main.cpp"
    #include <bit>
    #include <bitset>
    #include <cassert>
    #include <chrono>
    #include <concepts>
    #include <cstdint>
    #include <iomanip>
    #include <iostream>
    #include <limits>
    #include <random>
    #include <stdexcept>
    #include <string>
    #include <utility>
    #include <vector>

    template <typename T, typename... U>
    concept neither = (!std::same_as<T, U> && ...);

    template <typename T>
    concept strict_unsigned_integral =
        std::unsigned_integral<T> && neither<T, bool, char, char8_t, char16_t, char32_t, wchar_t>;

    constexpr bool has_single_bit(strict_unsigned_integral auto x) noexcept {
      return x && !(x & (x - 1));
    }

    constexpr int iterated_popcnt(strict_unsigned_integral auto n) {
      int count = 0;
      for (; n; n >>= 1) {
        count += n & 1;
      }
      return count;
    }

    constexpr int sparse_popcnt(strict_unsigned_integral auto n) {
      int count = 0;
      while (n) {
        ++count;
        n &= n - 1;
      }
      return count;
    }

    constexpr int dense_popcnt(strict_unsigned_integral auto n) {
      int count = sizeof(n) * 8;
      n ^= static_cast<decltype(n)>(-1);
      while (n) {
        --count;
        n &= n - 1;
      }
      return count;
    }

    template <auto Func>
    constexpr int apply_32bit_chunks(strict_unsigned_integral auto n) {
      if constexpr (sizeof(n) <= sizeof(uint32_t)) {
        return Func(static_cast<uint32_t>(n));
      } else {
        constexpr size_t chunks = sizeof(n) / sizeof(uint32_t);
        return [&]<size_t... Is>(std::index_sequence<Is...>) {
          return (Func(static_cast<uint32_t>(n >> (Is * 32))) + ...);
        }(std::make_index_sequence<chunks>{});
      }
    }

    constexpr int parallel_popcnt_32(uint32_t n) {
    #define POW2(c)     (1U << (c))
    #define MASK(c)     (static_cast<uint32_t>(-1) / (POW2(POW2(c)) + 1U))
    #define COUNT(x, c) (((x) & MASK(c)) + (((x) >> (POW2(c))) & MASK(c)))
      n = COUNT(n, 0);
      n = COUNT(n, 1);
      n = COUNT(n, 2);
      n = COUNT(n, 3);
      n = COUNT(n, 4);
      return n;
    #undef COUNT
    #undef MASK
    #undef POW2
    }

    constexpr int parallel_popcnt(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<parallel_popcnt_32>(n);
    }

    constexpr int nifty_popcnt_32(uint32_t n) {
      constexpr uint32_t max = std::numeric_limits<uint32_t>::max();
      n = (n & (max / 3))  + ((n >> 1) & (max / 3));
      n = (n & (max / 5))  + ((n >> 2) & (max / 5));
      n = (n & (max / 17)) + ((n >> 4) & (max / 17));
      return n % 255;
    }

    constexpr int nifty_popcnt(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<nifty_popcnt_32>(n);
    }

    constexpr int hacker_popcnt_32(uint32_t n) {
      n -= (n >> 1) & 0x55'55'55'55;
      n  = (n & 0x33'33'33'33) + ((n >> 2) & 0x33'33'33'33);
      n  = ((n >> 4) + n) & 0x0F'0F'0F'0F;
      n += n >> 8;
      n += n >> 16;
      return n & 0x00'00'00'3F;
    }

    constexpr int hacker_popcnt(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<hacker_popcnt_32>(n);
    }

    constexpr int hakmem_popcount_32(uint32_t n) {
      uint32_t tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
      return ((tmp + (tmp >> 3)) & 030707070707) % 63;
    }

    constexpr int hakmem_popcount(strict_unsigned_integral auto n) {
      return apply_32bit_chunks<hakmem_popcount_32>(n);
    }

    namespace detail {
    #define BIT2(n) n, n + 1, n + 1, n + 2
    #define BIT4(n) BIT2(n), BIT2(n + 1), BIT2(n + 1), BIT2(n + 2)
    #define BIT6(n) BIT4(n), BIT4(n + 1), BIT4(n + 1), BIT4(n + 2)
    #define BIT8(n) BIT6(n), BIT6(n + 1), BIT6(n + 1), BIT6(n + 2)

    inline constexpr uint8_t lookup_table[256] = {BIT8(0)};

    #undef BIT8
    #undef BIT6
    #undef BIT4
    #undef BIT2
    } // namespace detail

    constexpr int lookup_popcnt(strict_unsigned_integral auto n) {
      constexpr size_t bytes = sizeof(n);
      return [&]<size_t... Is>(std::index_sequence<Is...>) {
        return (detail::lookup_table[(n >> (Is * 8)) & 0xFF] + ...);
      }(std::make_index_sequence<bytes>{});
    }

    template <strict_unsigned_integral T>
    void test_correctness_for_type(const std::string& type_name) {
      std::mt19937_64 gen(42);
      std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<T>::max());

      constexpr int TEST_CASES = 100000;
      for (int i = 0; i < TEST_CASES; ++i) {
        T num = static_cast<T>(dist(gen));
        int expected = std::popcount(num);

        if (iterated_popcnt(num) != expected) throw std::runtime_error("iterated failed on " + type_name);
        if (sparse_popcnt(num)   != expected) throw std::runtime_error("sparse failed on " + type_name);
        if (dense_popcnt(num)    != expected) throw std::runtime_error("dense failed on " + type_name);
        if (parallel_popcnt(num) != expected) throw std::runtime_error("parallel failed on " + type_name);
        if (nifty_popcnt(num)    != expected) throw std::runtime_error("nifty failed on " + type_name);
        if (hacker_popcnt(num)   != expected) throw std::runtime_error("hacker failed on " + type_name);
        if (hakmem_popcount(num) != expected) throw std::runtime_error("hakmem failed on " + type_name);
        if (lookup_popcnt(num)   != expected) throw std::runtime_error("lookup failed on " + type_name);
      }
    }

    void run_correctness_tests() {
      std::cout << "[+] Running Differential Testing (Fuzzing) for All Types...\n";

      auto check_edge_cases = []<strict_unsigned_integral T>() {
        T zero = 0;
        T all_ones = std::numeric_limits<T>::max();
        assert(hacker_popcnt(zero) == std::popcount(zero));
        assert(hacker_popcnt(all_ones) == std::popcount(all_ones));
      };

      check_edge_cases.template operator()<uint8_t>();
      check_edge_cases.template operator()<uint64_t>();

      test_correctness_for_type<uint8_t>("uint8_t");
      test_correctness_for_type<uint16_t>("uint16_t");
      test_correctness_for_type<uint32_t>("uint32_t");
      test_correctness_for_type<uint64_t>("uint64_t");
      test_correctness_for_type<size_t>("size_t");

      std::cout << "[+] All types passed successfully!\n";
    }

    template <typename Func, typename T>
    void benchmark_algo(const std::string& name, Func func, const std::vector<T>& data) {
      auto start = std::chrono::high_resolution_clock::now();

      int sum = 0;
      for (T val : data) {
        sum += func(val);
      }

      volatile int prevent_optimization = sum;
      (void)prevent_optimization;

      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end - start;

      std::cout << std::left << std::setw(20) << name
                << ": " << std::right << std::setw(8) << elapsed.count() << " ms\n";
    }

    template <strict_unsigned_integral T>
    void run_benchmark_for_type(const std::string& type_name) {
      std::cout << "\n[=== Benchmarking Type: " << type_name
                << " (sizeof = " << sizeof(T) << " bytes) ===]\n";

      constexpr size_t DATA_SIZE = 10'000'000;
      std::vector<T> data(DATA_SIZE);

      std::mt19937_64 gen(12345);
      std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<T>::max());
      for (auto& v : data) {
        v = static_cast<T>(dist(gen));
      }

      auto wrap_std      = [](T n) { return std::popcount(n); };
      auto wrap_iterated = [](T n) { return iterated_popcnt(n); };
      auto wrap_sparse   = [](T n) { return sparse_popcnt(n); };
      auto wrap_dense    = [](T n) { return dense_popcnt(n); };
      auto wrap_parallel = [](T n) { return parallel_popcnt(n); };
      auto wrap_nifty    = [](T n) { return nifty_popcnt(n); };
      auto wrap_hacker   = [](T n) { return hacker_popcnt(n); };
      auto wrap_hakmem   = [](T n) { return hakmem_popcount(n); };
      auto wrap_lookup   = [](T n) { return lookup_popcnt(n); };

      benchmark_algo("std::popcount", wrap_std, data);
      std::cout << std::string(35, '-') << "\n";
      benchmark_algo("Hacker's Delight", wrap_hacker, data);
      benchmark_algo("Lookup Table", wrap_lookup, data);
      benchmark_algo("Parallel Fold", wrap_parallel, data);
      benchmark_algo("Nifty Popcnt", wrap_nifty, data);
      benchmark_algo("HAKMEM 169", wrap_hakmem, data);
      std::cout << std::string(35, '-') << "\n";
      benchmark_algo("Sparse (Kernighan)", wrap_sparse, data);
      benchmark_algo("Dense Popcnt", wrap_dense, data);
      benchmark_algo("Iterated (Slow)", wrap_iterated, data);
    }

    void run_all_benchmarks() {
      std::cout << "\n[+] Warming up CPU cache and running benchmarks (10 Million items)...\n";
      run_benchmark_for_type<uint8_t>("uint8_t");
      run_benchmark_for_type<uint32_t>("uint32_t");
      run_benchmark_for_type<uint64_t>("uint64_t");
    }

    int main() {
      static_assert(has_single_bit(1U));
      static_assert(!has_single_bit(0U));
      static_assert(!has_single_bit(3U));
      static_assert(lookup_popcnt(0b1111111100001111ULL) == 12);
      static_assert(hacker_popcnt(0xFFFFFFFFFFFFFFFFULL) == 64);

      try {
        run_correctness_tests();
        run_all_benchmarks();
      } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
      }

      return 0;
    }
    ```

## 使用建议

- 业务代码优先使用 `std::popcount`，其语义与可移植性最佳。
- 自定义实现适用于学习位运算技巧、兼容旧平台或做专项性能研究。
- 进行性能比较时，应在目标编译器与优化参数（如 `-O2`/`-O3`）下执行 benchmark。

*[POPCNT]: x86 架构上的硬件位计数指令。
