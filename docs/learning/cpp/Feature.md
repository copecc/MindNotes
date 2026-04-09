
# 新特性概览

## C++ 11

C++11 是 C++ 语言标准化以来的最大变革，引入了现代 C++ 的核心基础设施。

### 右值引用与移动语义（Rvalue References & Move Semantics）

C++11 引入了右值引用（`&&`），允许开发者直接绑定并操作即将销毁的临时对象（右值）。

这直接催生了移动语义（Move Semantics）。在此之前，深拷贝是传递大对象的唯一方式；通过移动语义，可以采用“资源窃取”的方式，将底层指针的所有权从临时对象转移，从而避免昂贵的堆内存分配与复制操作。

!!! example "移动与转发示例"

    ```cpp
    #include <string>
    #include <utility>
    class Base {
    public:
        std::string buffer;
        // 移动构造函数：窃取 resource
        Base(std::string&& res) : buffer(std::move(res)) {}
    };

    template <typename T>
    void wrapper(T&& arg) {
        // perfect forwarding 完美转发，无损保留右值属性
        Base b(std::forward<T>(arg)); 
    }
    ```

- `std::move`本质上是一个安全的类型转换（`static_cast<T&&>`），它本身不移动任何东西，只是将左值强制转换为右值引用，从而匹配移动构造函数或移动赋值运算符。
- 配合完美转发（`std::forward`）和模板类型推导（引用折叠规则），使得在模板中可以无损保留参数的左右值属性。

### Lambda 表达式

Lambda 表达式提供了一种在调用点定义匿名函数对象的机制。

!!! example "Lambda 捕获示例"

    ```cpp
    int multiplier = 3;
    // 按值捕获 multiplier
    auto times = [multiplier](int a) { return a * multiplier; };
    ```

编译器在底层会将 Lambda 表达式转换为一个带有重载 `operator()` 的匿名闭包类（Closure Class）。捕获列表（Capture List）中的变量会成为该类的成员变量。

- 值捕获：在闭包对象构造时拷贝值。
- 引用捕获：在闭包对象中存储引用或指针，需要严格注意悬垂引用的生命周期问题。

### 强类型枚举与空指针（enum class & nullptr）

C++11 以 `nullptr` 替代了传统的宏 `NULL`（本质上是整数 `0`），消除了指针和整型重载解析时的歧义。同时引入了作用域强类型枚举 `enum class`。

!!! example "强类型与空指针"

    ```cpp
    void func(int x) {}
    void func(char* ptr) {}

    func(0);       // 调用 func(int)
    func(nullptr); // 明确调用 func(char*) (无歧义)
    enum class Color : uint8_t { Red, Green, Blue };
    // Color c = 0; // Error：禁止隐式类型转换，必须使用 Color::Red
    ```

### 智能指针（Smart Pointers）

为了解决裸指针（Raw Pointers）带来的内存泄漏和使用后释放（Use-After-Free）问题，C++11 引入了现代内存管理组件，基于 RAII（Resource Acquisition Is Initialization）原则管理堆内存。

- `std::unique_ptr`：独占所有权模型，底层实现通常只是一个原生指针包裹，在开启编译器优化后具有零开销抽象，不可复制，只可移动（基于移动语义）。
- `std::shared_ptr`：共享所有权模型，内部维护一个指针指向控制块（包含强引用计数、弱引用计数及删除器）。引用计数的修改是原子操作，因此本身是线程安全的，但对象本身的读写需要额外同步。
- `std::weak_ptr`：弱引用模型，用于打破 `std::shared_ptr` 的循环引用。它只增加控制块的弱引用计数，不影响对象的生命周期。

### 类型推导与编译期常量（auto & constexpr）

- `auto`与`decltype`：静态类型推导，编译器在编译期根据初始化表达式或变量声明推导出准确类型，常用于迭代器声明或复杂模板返回值。
- `constexpr`：常量表达式，指示编译器在编译期计算函数或变量的值，以换取极高的运行时性能和程序数据段（`.rodata`）的优化。

## C++ 17

C++17 侧重于语法的简化和实用标准库的填充，提升了开发体验。

### 结构化绑定（Structured Binding）

结构化绑定允许直接从由元组（Tuple）、对（Pair）或公开成员聚合类返回的复合结构中解构出多个变量。

!!! example "结构化解构遍历"

    ```cpp
    #include <map>
    #include <string>
    std::map<int, std::string> m = { {1, "C++"}, {2, "Rust"} };
    // 直接解构 key 和 value，无需使用 it->first 和 it->second
    for (const auto& [key, value] : m) {
        // ...
    }
    ```

底层上，编译器会创建一个隐藏的临时对象保存右侧的复合结果，并为结构化绑定声明的所有标识符建立对该临时对象内部元素的引用别名。

### 编译期条件分支（if constexpr）

在模板元编程中，传统的基于 SFINAE 规则或模板特化的条件分发非常冗长。

!!! example "简化的模板分发"

    ```cpp
    template <typename T>
    auto get_value(T x) {
        if constexpr (std::is_pointer_v<T>) {
            return *x; // 当且仅当 T 为指针时，这部分代码才会被编译
        } else {
            return x;
        }
    }
    ```

`if constexpr` 使得编译器能够根据编译期可知的条件，在实例化模板时直接丢弃不匹配的分支（Discarded Statement）。被丢弃的分支甚至不需要通过类型检查（只要没有语法错误），极大地简化了变参模板和泛型代码的编写。

### optional 与 variant

C++17 在标准库中引入了用于错误处理与状态建模的值类型语义组件，替代了传统的多指针出参或复杂的类型安全联合体。

!!! example "显式错误处理与状态建模"

    ```cpp
    #include <optional>
    #include <variant>
    #include <string>
    std::optional<int> find_id(const std::string& name) {
        if (name == "Admin") return 1;
        return std::nullopt; // 代替返回 -1 或抛出异常
    }

    // 类型安全的联合体 (Type-safe Union)
    using Token = std::variant<int, double, std::string>;
    Token val = 42; 
    // 利用 std::holds_alternative 获取状态
    if (std::holds_alternative<int>(val)) {
        int i = std::get<int>(val); 
    }
    ```

### 折叠表达式（Fold Expressions）

为了更简便地处理可变参数模板（Variadic Templates），C++17 引入了折叠表达式机制，无需像 C++11 那样手写递归终止模板。

!!! example "无递归展开参数包"

    ```cpp
    template<typename... Args>
    auto sum(Args... args) {
        // C++17 一元右折叠
        return (args + ...);
    }

    sum(1, 2, 3, 4);  // 缩减为 (1 + (2 + (3 + 4))) -> 10
    ```

### string_view

对于只读字符串的高频查阅，频繁使用 `std::string` （乃至触发小字符串优化 SSO 以外的堆分配）代价极其高昂。

`std::string_view` 提供了一个非拥有的（Non-owning）字符串视图，其内部本质上只存了一个只读字符指针（`const char*`）和一个长度（`size_t`）。由于不涉及内存分配与析构，通过它可以实现零拷贝的字符串子串切片和参数传递。

!!! warning "注意"

    由于 `std::string_view` 不管理内存，开发者必须确保底层被引用字符串的生命周期比视图对象更长，且不要假定视图是以 `\0` 结尾的。

## C++ 20

C++20 是继 C++11 以来的又一重大里程碑，涵盖了四大核心语言特性，彻底革新了开发模式。

### 模块（Modules）

这是为了替代几十年来基于预处理器宏和文本复制的 `#include` 机制。包含大量模板库的头文件会呈指数级膨胀，造成编译奇慢；宏污染也是难以绕过的坑。

!!! example "模块声明与导入"

    ```cpp
    // file: math.ixx (Module Interface Unit)
    export module math; 
    // 只导出需要的符号，内部宏与私有实现对外部透明
    export int add(int a, int b) {
        return a + b;
    }

    // file: main.cpp 
    import math; // O(1) 的二进制解析，不进行文本展开
    // int res = add(1, 2);
    ```

模块仅编译一次，后续被 `import` 时直接读取生成的编译模块（BMI）即可，大大提升了构建速度。更重要的是，使用模块可以真正封装实现细节与宏指令。

### 概念与约束（Concepts & Constraints）

在 C++20 之前，模板错误往往导致数以百计的毫无意义的“模板实例化回溯”报错，且 SFINAE （如 `std::enable_if`）非常晦涩。

!!! example "通过 requires 子句添加概念约束"

    ```cpp
    #include <concepts>
    // 使用预定义的概念限制泛型必须支持整数运算
    template <std::integral T> 
    T add(T a, T b) { return a + b; }

    // 使用 requires 强制重载决策
    template <typename T>
    void process(T val) requires std::convertible_to<T, int> { }
    ```

Concepts 作为一等公民引入，允许对模板参数进行静态约束，明确指定模板期望的数据类型特征（如“必须可哈希”、“必须支持迭代器”）。当约束不满足时，编译器会精准报出不符合的概念，极大地增强了模板编程的健壮性和可读性。

### 范围库（Ranges）

Ranges 弥补了传统 STL 算法需要显式传递 `begin()` 和 `end()` 的繁琐，并带来了延迟求值（Lazy Evaluation）的新范式。

!!! example "以管道风格进行链式计算"

    ```cpp
    #include <vector>
    #include <ranges>
    std::vector<int> data = {1, 2, 3, 4};
    // 求偶数元素的平方（此处只是视图声明，延迟求值直到被遍历）
    auto view = data 
              | std::views::filter([](int n){ return n % 2 == 0; }) 
              | std::views::transform([](int n){ return n * n; });
    ```

通过视图（Views）和范围适配器（Range Adaptors），开发者可以使用管道运算符（`|`）实现惰性、链式的数据处理管道（类似于函数式编程），极大地提升了处理大规模或无限数据流时的表达能力，且编译器能将视图链优化为极高校准的循环操作。

### 协程（Coroutines）

协程为 C++ 带来了原生的无栈协程（Stackless Coroutines）支持，通过 `co_await`、`co_yield` 和 `co_return` 关键字使得异步编程能够以同步的面貌编写。

!!! example "无栈协程示例结构"

    ```cpp
    #include <coroutine>
    // 假设已有 Generator 封装实现...
    Generator<int> iota(int n) {
        while (true) {
            co_yield n++; // 挂起并返回当前 n，将控制权交还给调用者
        }
    }
    ```

无栈协程的机制是编译器将函数的局部变量、参数和挂起点状态保存到分配在堆上的协程状态帧（Coroutine State）中，而不是操作系统的调用栈上。这使得数十万个并发 I/O 操作可以在极低开销下调度。目前 C++20 主要提供底层设施（如 promise_type），通常需要借助额外的协程库（如 cppcoro或 Asio）来实现业务逻辑。

### 三路比较运算符（Space Ship Operator）

C++20 新增了 `<=>` 飞船运算符，它可以由编译器为你的结构体默认生成一套一致的比较逻辑(`<`, `>`, `<=`, `>=`, `==`, `!=`)，彻底消除了重载 6 个操作符的痛点。

!!! example "编译器自动生成的默认比较逻辑"

    ```cpp
    #include <compare>
    #include <string>
    struct Person {
        std::string name;
        int age;
        // 要求编译器默认生成所有 6 个比较运算符
        auto operator<=>(const Person&) const = default;
    };
    ```

### 源码分支预测提示（likely/unlikely）

它们本质上是给编译器的分支预测提示。它们是 C++20 标准正式引入的属性（前身是 `__builtin_expect` 宏）。工作原理如下：

!!! example "指导代码生成路径布局"

    ```cpp
    double compute(double x) {
        if (x < 0) [[unlikely]] {
            throw std::domain_error("Negative number");
        }
        return std::sqrt(x);
    }
    ```

- 编译器层面（代码重排）：编译器会将被标记为 `[[likely]]` 的基本块（Basic Block）在物理内存中紧跟判断指令之后（形成 Fall-through 顺序流）；将被标记为 `[[unlikely]]` 的冷代码剥离出主流程，甚至放入独立的冷代码段。
- 硬件层面（I-Cache 与预测）：这种内存连续性排布极大地提升了 CPU 预取指令时的 L1 指令缓存（I-Cache）命中率和指令密度，同时也完美契合 CPU 的静态分支预测偏好。

因此：

- 标记为 `likely` 的路径更可能被排布在顺序执行路径上，提升指令缓存局部性。
- 标记为 `unlikely` 的路径通常被放到冷区，减少对热点路径的干扰。

这类标记尤其适合系统编程中的热点路径，例如高频分支判断、错误检查和懒加载分支。

!!! warning "注意"

    现代 CPU 的动态分支预测极其强大。除非有明确的性能剖析数据（Profiling），或用于极低频的异常/错误拦截分支，否则不要滥用这两个属性，以免对硬件产生负面误导。
