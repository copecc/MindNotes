---
title: 枚举与字符串反射转换
tags:
  - C++
  - Enum
---

# 枚举与字符串反射转换

!!! abstract "摘要"

    在 C++ 开发中，将枚举值与字符串名称进行双向转换是一项常见的需求。本文探讨了基于传统映射以及基于编译器宏 `__PRETTY_FUNCTION__` 实现静态反射的两种不同策略，并分析了其实现细节与适用场景。

## 传统设计与实现

在常规的工程实践中，为了获取枚举类中值的名称或通过字符串名称定位到对应的枚举值，最直观的解决方案是维护一个映射容器（如 `std::map<int, std::string>`），或者依赖底层值的索引与名称的对应关系。

例如，通过维护一个 `std::vector<QString>` 来实现对枚举值和索引下标的双向转换：

??? note "传统枚举转换"

    ```cpp title="enum_map.cpp"
    enum class Buff { AOE, RAGE, FROZE, BLEED, TRACK, STAR, FLASH, QUICK };

    std::vector<QString> buff2string = {"AOE", "RAGE", "FROZE", "BLEED", "TRACK", "STAR", "FLASH", "QUICK"};

    // 将字符串名称转换为对应的枚举值
    auto buff = static_cast<Buff>(std::find(buff2string.begin(), buff2string.end(), target_string) - buff2string.begin());

    // 将枚举值转换成其对应的字符串名称
    auto buff_string = buff2string[static_cast<int>(buff)];
    ```

上述方案的同等效果也可以通过一个辅助函数中的 `switch case` 控制流枚举返回来实现。该方案的缺点是每次增加枚举值时，都必须手动维护对应的字符串容器或分支逻辑，容易引入由于更新不同步导致的潜在运行时错误。

## 基于编译器宏的静态反射方案

为了规避手动维护映射的开销，可以利用特定编译器扩展宏（如 GCC/Clang 支持的 `__PRETTY_FUNCTION__` 或 MSVC 支持的类似宏）在编译期提取函数的完整签名字符串，进而利用字符串处理提取枚举值名称。

### 宏展开机理

以下代码展示了如何通过模板参数结合宏来提取完整的函数名称字符串：

??? note "展开机理示例"

    ```cpp title="enum_reflect_basic.cpp"
    template <typename T, T V>
    char const* enum2string() {
        // __PRETTY_FUNCTION__ 是编译器的扩展支持宏。
        // 在编译阶段，它会被替换成一个包含当前所在函数名称、入参等详细类型信息的常数字符串。
        return __PRETTY_FUNCTION__;
    }

    enum class Color { red, green };

    int main() {
        std::cout << enum2string<unsigned int, 999u>() << std::endl;
        std::cout << enum2string<bool, false>() << std::endl;
        std::cout << enum2string<Color, Color::red>() << std::endl;
        
        // 以上分别输出（以 GCC 为例）：
        // const char* enum2string() [with T = unsigned int; T V = 999]
        // const char* enum2string() [with T = bool; T V = false]
        // const char* enum2string() [with T = Color; T V = Color::red]
    }
    ```

### 字符串解析与提取

获得完整的展开字符串后，需要对其进行切分以获取关键信息。以 `with T = Color; T V = Color::red` 为例，具体的提取规则如下：

- 第一个 `=` 和 `;` 之间的内容是枚举类型的全名，本例中为 `Color`。
- 第二个 `=` 和 `]` 之间的内容是枚举值的全称，本例中为 `Color::red`。
- 最后一个 `:` 和 `]` 之间的内容是枚举值的短名称，本例中为 `red`（适用于 `scoped enum`）。
- 对于传统的 C++ 枚举（即未增加 `class` 关键字），全称和短名称是相同的，提取时即可依据是否存在冒号 `::` 来判定是否为作用域枚举（Scoped Enum）。

### 编译期数据结构设计

结合 `C++17` 中引入的 `std::string_view`，可以在编译期完成无动态内存分配的解析。定义一个结构体用于存储提取出的反射信息，并实现一个 `constexpr` 模板函数自动在编译期提取名称：

??? note "完整的 constexpr 提取实现"

    ```cpp title="enum_reflect_constexpr.cpp"
    #include <string_view>
    
    // 存储枚举反射信息的结构体
    // 采用 std::string_view 存储切片，避免静态期间的动态内存分配
    struct ReflectionEnumInfo {
        bool scoped; // 是否为作用域枚举
        std::string_view name, valueFullName, valueName; // 分别代表类型名、值全名、值短名
    
        // 通过编译期常量表达式解析目标边界截取视图
        constexpr ReflectionEnumInfo(char const* info, std::size_t e1, std::size_t s, std::size_t e2, std::size_t colon, std::size_t end)
            : scoped(colon != 0), 
              name(info + e1 + 2, s - e1 - 2), 
              valueFullName(info + e2 + 2, end - e2 - 2), 
              valueName(scoped ? std::string_view(info + colon + 1, end - colon - 1) : valueFullName) {}
    };
    
    template <typename E, E V>
    constexpr ReflectionEnumInfo Renum() {
        char const* info = __PRETTY_FUNCTION__;
                
        std::size_t l = strlen(info);
        std::size_t e1 = 0, s = 0, e2 = 0, colon = 0, end = 0;
        
        for (std::size_t i = 0; i < l && !end; ++i) {
            switch(info[i]) {
                case '=': 
                    (!e1) ? e1 = i : e2 = i; 
                    break;            
                case ';': 
                    s = i; 
                    break;                
                case ':': 
                    colon = i; 
                    break;
                case ']': 
                    end = i; 
                    break;
            }
        }
            
        return {info, e1, s, e2, colon, end}; 
    }
    ```

利用上述泛型模板编程机制与现代 C++ 特性，能够以极低开销实现相对可靠的枚举名称提取及跨平台适配。

## 参考资料

- [静态反射C++枚举名字的超简方案——C++闲得慌系列之（一）](https://zhuanlan.zhihu.com/p/419673631)
- [C/C++如何用最简洁的方式同时声明枚举和对应的字符串列表？](https://www.zhihu.com/question/482502037)

