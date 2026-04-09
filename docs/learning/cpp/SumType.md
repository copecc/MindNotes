---
title: Sum Type
tags:
  - C++
---

# Sum Type

!!! abstract "摘要"

    Sum type（或标签联合，Tagged Union）在函数式编程中是一种常见的数据结构，用于表示某个变量能够随时处于多个不同类型或状态中的恰好一种。在 C++ 中，`std::variant` 是表现 Sum type 的标准化支持，但在部分同构类型的区隔应用上仍有局限。本文分析了在现代 C++ 语境下 Sum type 的概念以及其变体实现。

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

一种解决同构类型冲突的常见方法是为每种逻辑语义附加一层抽象包装结构实体（`wrapper`），让底层类型通过强类型的包装显式分离开来。

以下代码示例使用简单的模板包装类 `Left` 和 `Right` 来模拟类似 Haskell/Rust 中 `Either` 的双边分支逻辑：

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
    
    // 利用仿函数提供访问者（Visitor）功能
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

如此一来，无论底层实际的数据载体（如 `int`）是否重叠，业务逻辑仅需要应对通过类型安全包装的外壳，规避了直接依赖类型的歧义。

## 编译期索引化访问（进阶技巧）

如果不想为成千上万种情况增加硬编码的 `wrapper`，可以通过提取并操作 `std::variant` 的底层类型索引序列（`std::index_sequence`）提供更为强大的通用元编程提取过程。

这一方法主要用于应对复杂的编译期的元编程展开或代码生成：

??? note "底层类型索引序列实现"

    ```cpp title="variant_indexer.cpp"
    #include <iostream>
    #include <type_traits>
    #include <utility>
    #include <variant>
    
    // 常量整型的包装
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
    } // namespace helpers
    
    // 提供指定数量的替换形式
    template <std::size_t N>
    using alternative = typename helpers::number_helper<std::make_index_sequence<N>>::type;
    
    namespace helpers {
        template <class... Ts, std::size_t... Is, class R = alternative<sizeof...(Ts)>>
        constexpr R get_alternative(std::variant<Ts...> const& v, std::index_sequence<Is...>) {
            constexpr R retvals[] = {R(index<Is>)...};
            return retvals[v.index()];
        }
    } // namespace helpers
    
    template <class... Ts>
    constexpr alternative<sizeof...(Ts)> get_alternative(std::variant<Ts...> const& v) {
        return helpers::get_alternative(v, std::make_index_sequence<sizeof...(Ts)>{});
    }
    
    int main() {
        // 原地利用下标初始化同样的类型
        std::variant<int, int> var(std::in_place_index_t<1>{}, 7);
        
        auto which = get_alternative(var);
        
        std::visit([&var](auto I) { 
            std::cout << "Value mapped by Index: " << std::get<I>(var) << "\n"; 
        }, which);
    
        return 0;
    }
    ```

编译期通过索引化访问，消除了针对同一物理类型的值产生多次复制时出现的语义模糊，为动态分支提取提供了更严谨的基础结构保证。

## 参考资料

- [templates - Sum types in C++ - Stack Overflow](https://stackoverflow.com/questions/64779163/sum-types-in-c)
