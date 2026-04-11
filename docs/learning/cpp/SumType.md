---
title: Sum Type
tags:
  - C++
  - Variant
  - MetaProgramming
---

# Sum Type

!!! abstract "摘要"

    Sum type（或标签联合，Tagged Union）在函数式编程中是一种常见的数据结构，用于表示某个变量能够随时处于多个不同类型或状态中的恰好一种。在 C++ 中，`std::variant` 是表现 Sum type 的标准化支持，但在部分同构类型的区隔应用上仍有局限。本文深入分析现代 C++ 语境下 Sum type 的概念与变体实现，探讨其底层机制、性能开销以及工业界解决方案。

## 物理布局与内存开销

`std::variant` 的底层内存布局本质上是一个联合体（`union`）配合一个额外的类型索引（`index`）。这种设计意味着其大小并非仅仅是最大成员变量的大小，还会产生由对齐（Alignment）带来的内存膨胀。

### 内存对齐机制

在 64 位架构下，为了保证 CPU 能通过缓存行（Cache Line）快速访问，编译器会对 `std::variant` 内部保存的最大类型进行边界对齐。如果最大类型占据 8 字节且类型索引占据 1 字节，编译器往往会为类型索引分配 8 字节空间以满足缓存对齐约束，从而导致结构体积膨胀。

当维护大规模对象集合时，频繁使用未经压缩的变长状态对象将直接拉低缓存命中率（Cache Hit Rate），显著劣化系统吞吐量。因此，在内存受限环境下需要手动管理标签，或利用指针的未用低位（Tagged Pointers）进行极端压缩机制。

## 基本概念与背景

`sum type` 通常指代一种抽象数据类型的概念，它可以表示多种不同类型的值中的一种。通过将多个互相排斥且独立的数据类型聚合在一个全新类型下，允许该变量安全且严格地在这些类型变体间切换：

??? note "类型聚合概念"

    ```text
    Type C = Type A | Type B
    ```

在标准 C++ 库中，`std::variant` 提供了对此机制的基础支持。然而，当开发者需要表示具有同样底层物理类型但携带不同逻辑语义的状态值时，原生的 `sum type` 若没有辅助包装则难以直接区分结构。例如，`std::variant<uint64_t, uint64_t>` 是完全合规的 C++ 语法，但在使用 `visitor` 时或提取值时，编译器可能无法准确得知需要取哪个变体。

!!! note "std::variant - cppreference.com"

    参考自：[std::variant - cppreference.com](https://en.cppreference.com/w/cpp/utility/variant)
    
    A variant is permitted to hold the same type more than once, and to hold differently cv-qualified versions of the same type.

## 基于包装体的解决方案

一种解决同构类型冲突的常见方法是为每种逻辑语义附加一层抽象包装结构实体（`wrapper`）。通过强类型的包装结构，让底层类型能够在业务逻辑中显式分离并消除二义性。

以下代码示例使用模板包装类 `Left` 和 `Right` 来模拟类似 Haskell/Rust 中 `Either` 的双侧逻辑：

??? note "包装体解决方案"

    ```cpp title="either_wrap.cpp"
    #include <variant>
    #include <iostream>
    
    template <class T>
    struct Left {
        T val;
    };
    
    template <class T>
    struct Right {
        T val;
    };
    
    // 使用 std::variant 定义强类型的 Either sum type
    template <class T, class U>
    using Either = std::variant<Left<T>, Right<U>>;
    
    // 模拟随机成功与随机失败的结果返回
    Either<int, int> TrySomething() {
        if (rand() % 2 == 0) {
            return Left<int>{0};
        } else {
            return Right<int>{1};
        }
    }
    
    // 利用仿函数提供访问者功能
    struct Visitor {
        template <class T>
        void operator()(const Left<T>& val_wrapper) const {
            std::cout << "Success! Value is: " << val_wrapper.val << std::endl;
        }
    
        template <class T>
        void operator()(const Right<T>& val_wrapper) const {
            std::cout << "Failure! Value is: " << val_wrapper.val << std::endl;
        }
    };
    
    int main() {
        Visitor v;
        for (size_t i = 0; i < 10; ++i) {
            auto res = TrySomething();
            std::visit(v, res);
        }
        return 0;
    }
    ```

无论底层实际的数据载体释放了多大的范围，业务逻辑仅应对通过类型安全外壳进行的请求，完全避免了由于数据结构一致但意义不同导致的方法调用冲突。

## 运行时性能与 Visitor 机制

### 函数指针矩阵（Function Pointer Matrix）的生成

当调用 `std::visit` 时，程序需要将运行时的索引值转换为具体的函数调用。在这背后，编译器通常会生成一个包含所有变种操作路径对应的静态函数指针数组或二维函数指针矩阵。

通过解引用指针矩阵即可跳转到具体执行路径。在大部分平台上，这种模式比一连串连续的 `if-else` 或大规模的 `switch` 分支结构有更短且可预测的处理边界，使得 CPU 的分支预测器（Branch Predictor）可以准确对重复模式做出预取。但在变体数极度膨胀且访问逻辑错综复杂的背景下，函数指针跳转可能导致指令缓存失效（Instruction Cache Miss），引发流水线停顿。

## 编译期索引化访问

若在项目核心循环内不便为成千上万种情况逐一硬编码增加 `wrapper` 抽象层，利用 C++ 的索引元组与元组访问接口则成为另一种备选强类型方案。

这一方法主要用于应对复杂的编译期的元编程展开或代码生成：

??? note "底层类型索引序列实现"

    ```cpp title="variant_indexer.cpp"
    #include <iostream>
    #include <type_traits>
    #include <utility>
    #include <variant>
    
    template <std::size_t i>
    using index_t = std::integral_constant<std::size_t, i>;
    
    template <std::size_t i>
    constexpr index_t<i> index = {};
    
    // 生成包含编译时下标组成的 Variant
    template <std::size_t... Is>
    using number = std::variant<index_t<Is>...>;
    
    namespace helpers {
        template <class X>
        struct number_helper;
    
        template <std::size_t... Is>
        struct number_helper<std::index_sequence<Is...>> {
            using type = number<Is...>;
        };
    }
    
    // 提供指定数量的替换形式
    template <std::size_t N>
    using alternative = typename helpers::number_helper<std::make_index_sequence<N>>::type;
    
    namespace helpers {
        template <class... Ts, std::size_t... Is, class R = alternative<sizeof...(Ts)>>
        constexpr R get_alternative(std::variant<Ts...> const& v, std::index_sequence<Is...>) {
            constexpr R retvals[] = {R(index<Is>)...};
            return retvals[v.index()];
        }
    }
    
    template <class... Ts>
    constexpr alternative<sizeof...(Ts)> get_alternative(std::variant<Ts...> const& v) {
        return helpers::get_alternative(v, std::make_index_sequence<sizeof...(Ts)>{});
    }
    
    int main() {
        std::variant<int, int> var(std::in_place_index_t<1>{}, 7);
        auto which = get_alternative(var);
        
        std::visit([&var](auto I) { 
            std::cout << "Value mapped by Index: " << std::get<I>(var) << "
"; 
        }, which);
    
        return 0;
    }
    ```

该方案在底层完全消除了复制逻辑时由于参数特征一致产生的编译错误。编译期提取了全部索引维度，使访问和抽取操作通过强化的位标（Index）执行。

## 参考资料

- [templates - Sum types in C++ - Stack Overflow](https://stackoverflow.com/questions/64779163/sum-types-in-c)
