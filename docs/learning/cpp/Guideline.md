---
title: Guideline
tags:
  - C++
  - Guideline
  - ABI
  - OOP
---

# C++ 语言与工程核心准则笔记

本文档记录了 C++ 工程实践中关于类型系统、指针修饰符、链接属性以及跨语言接口设计的核心机制。

## `const` 与指针的相互作用

在 C++ 类型系统中，理解 `const` 与指针组合的核心机制是：`const` 修饰符对其左侧的主体生效，如果左侧没有主体，则对其右侧的主体生效。

!!! note "核心原则"

    在分析指针声明时，应从变量名出发，向左右两侧展开解析。`const` 修饰哪个层级，该层级的访问权限即被限制为只读。

### 单层指针的访问权限

| 声明语法 | 指针自身是否可变 | 所指数据是否可变 |
| --- | --- | --- |
| `const int* p` | 是 | 否 |
| `int const* p` | 是 | 否 |
| `int* const p` | 否 | 是 |
| `const int* const p` | 否 | 否 |

在上述组合中，`const int* p` 与 `int const* p` 具有完全相同的底层属性，均表示指向常量的指针。而 `int* const p` 则是常量指针，表示指针自身的内存地址不可变更。

??? note "单层指针代码示例"

    ```cpp title="const_pointer.cpp"
    int a = 10;
    int b = 20;

    const int* p1 = &a;
    p1 = &b;      // 合法操作：可以改变指针的指向
    // *p1 = 30;  // 错误：不可通过该指针修改底层数据

    int* const p2 = &a;
    *p2 = 30;     // 合法操作：可以修改底层数据
    // p2 = &b;   // 错误：不可改变指针自身的指向
    ```

### 多级指针的推导

对于多级指针，分析逻辑相同，只需将访问层级逐级降维拆解。以二级指针为例，存在三个访问层级：`ptr`、`*ptr` 以及 `**ptr`。

不同 `const` 位置的具体约束如下：

- `const int** ptr`
  允许修改 `ptr` 和 `*ptr`，禁止修改 `**ptr`。

- `int* const* ptr`
  允许修改 `ptr`，禁止修改 `*ptr`，允许修改 `**ptr`。

- `int** const ptr`
  禁止修改 `ptr`，允许修改 `*ptr` 和 `**ptr`。

### 多级指针的 Const Correctness

在 C++ 类型系统中，双重指针 `int**` 无法发生隐式类型转换到 `const int**`。该规则的本质是为了维护类型系统中的 **Const Correctness** 约束，防止绕过类型保护。

如果编译器允许此类形变，将导致以下安全性问题：

??? note "类型安全性破坏示例"

    ```cpp title="const_correctness_violation.cpp"
    const int c = 10;
    int* p;
    
    // 假设编译器允许 int** 隐式转换为 const int**
    const int** pp = &p; 
    
    *pp = &c;            // 将常量地址写入变量 p 中
    *p = 20;             // 运行时错误：通过可写的底层指针修改了只读常量
    ```

!!! warning "多级指针的类型约束"

    在多级指针传递中，如果在更深层级引入 `const` 约束，在此层级之上的所有指针层级也必须具备只读约束。正确的安全转换目标应当为 `const int* const*`。

### 类型定义机制的差异

预处理器宏 `#define` 仅限于文本替换层面的操作，完全脱离了 C++ 的类型系统与作用域规则。引入类型别名时，应当严格使用 `using` 或 `typedef`。

??? note "宏替换与类型别名的差异"

    ```cpp title="macro_vs_typedef.cpp"
    #define INTPTR_MACRO int*
    typedef int* INTPTR_TYPE;

    int value = 0;

    const INTPTR_MACRO p1 = &value; // 预处理展开为：const int* p1
    const INTPTR_TYPE p2 = &value;  // 等价于语义：int* const p2
    ```

上述代码中，`INTPTR_MACRO` 直接在原地展开文本，导致 `const` 修饰目标发生偏移。而 `INTPTR_TYPE` 将“指向整型的指针”封装为独立类型，`const` 直接修饰该指针类型本身。

### `const volatile` 的语义与应用

`const` 与 `volatile` 修饰符能够在同一变量声明中并存，二者作用于不同的内存操作约束层面：

- `const`
  限制当前编译单元的代码路径中对变量进行写操作。

- `volatile`
  强制编译器取消对该变量读取的缓存优化，每次访问均需触发直接的内存读取指令。

结合后，`const volatile` 的精确工程定义是：数据在当前软件上下文中为只读属性，但该内存地址的内容随时可能被外部硬件上下文或异步执行流改变。

其典型应用场景包括：

- 内存映射 I/O 区域中的只读状态寄存器。
- 由中断服务程序（ISR）或 DMA 硬件控制器持续覆写的数据存储区。
- 内核空间直接映射至用户态的只读共享状态内存页。

??? note "硬件寄存器映射示例"

    ```cpp title="const_volatile_register.cpp"
    // 定义一个指向 32 位内存寄存器的不可写指针，且要求每次直接读取硬件
    const volatile uint32_t* const adc_data_register =
        reinterpret_cast<const volatile uint32_t*>(0x4001244C);

    uint32_t read_current_adc_value() {
        return *adc_data_register; // 必定生成数据总线上的读指令
    }
    ```

## `extern "C"` 与跨边界链接

关键字 `extern "C"` 的直接作用是指定 C 语言的 **ABI 链接规范**，而非表示其所包含的代码是由 C 语言编写。它的核心职责包括：

- 显式禁用 C++ 编译器的名称修饰（Name Mangling）机制。
- 指导链接器按照 C ABI 的标准符号解析规则来导出与解析符号表。

其常见的工程应用场景如下：

- 与纯 C 编写的系统底层库或第三方动态库进行符号链接。
- 声明并调用由汇编语言显式导出的裸符号。
- 为 Rust、Go、Python 或 Java JNI 提供长期稳定的二进制入口点。
- 为基于 `dlsym` 或 `GetProcAddress` 运行时查找的插件架构提供工厂函数。

!!! warning "ABI 边界限制"

    `extern "C"` 仅重置符号的链接属性，并不影响代码块内部的 C++ 语法支持。但是，跨越此边界传递的参数与返回值必须维持严格的 C ABI 兼容性。优先使用普通旧数据（POD）、错误码体系或不透明指针（Opaque Pointer）。

### Flat C API Wrapper 模式

在将复杂的 C++ 面向对象特性导出给其他语言生态（如 Rust 或 Python）时，应当采取 Flat C API Wrapper 设计模式。

其基本封装规则如下：

1. 运用不透明指针（Opaque Pointer）对外隐藏 C++ 对象内存布局以及虚表实现。
2. 将类的成员函数展平为普通 C 函数，并将指向实例对象的指针作为函数的首个参数进行传递。
3. 必须在 `extern "C"` 的函数实现边界内部捕获所有抛出的 C++ 异常，并将其转换为对应的数值状态码后返回。

??? note "平铺式 C 接口代码示例"

    ```cpp title="sensor_api_wrapper.h"
    #ifdef __cplusplus
    extern "C" {
    #endif

    // 不透明结构体声明
    typedef struct CSensor CSensor;

    CSensor* Sensor_create_temperature(double initial_temp);
    
    void Sensor_destroy(CSensor* sensor);
    
    double Sensor_read_data(const CSensor* sensor, int* out_error);
    
    void Sensor_calibrate(CSensor* sensor, double offset, int* out_error);

    #ifdef __cplusplus
    }
    #endif
    ```

## 头文件中的 `static` 变量存储机制

若开发者在头文件中直接定义静态全局变量：

??? note "头文件静态变量示例"

    ```cpp title="header_static.h"
    static int shared_count = 0;
    ```

在包含此头文件的每一个独立编译单元（Translation Unit）中，编译器都会利用内部链接属性（Internal Linkage）为该变量构造一份完全独立的内存副本。链接器不会合并这些属于不同目标文件但在命名上相同的内容。

为了解决多文件数据同步与代码膨胀问题：

- 当存在跨文件的全局共享需求时：在头文件中使用 `extern` 进行声明，而在单一源文件中提供实际定义。
- 采用 C++17 或更新标准：直接利用 `inline` 修饰符声明内联变量，交由链接器统一合并定义。

## 构造函数的执行异常与 RAII 资源管理

在 C++ 对象生命周期规则中，如果在对象的构造函数执行过程中抛出异常，此时该对象被标记为**未完成构造**状态。

此机制的具体表现与影响如下：

1. 对象自身的析构函数绝对不会被触发。
2. 在该抛出端点之前已经成功独立初始化的对象成员（及其基类），系统会按照与构造顺序严格相反的方向依次调用析构函数进行清理。
3. 如果构造函数内部直接调用裸指针进行了资源分配，因为对象主体析构不会触发，直接分配的裸堆内存将产生内存泄漏。

!!! tip "RAII 的核心地位"

    强制要求使用遵循 RAII 模式的资源封装（如 `std::unique_ptr` 或自动释放文件句柄）。通过依赖成员对象的自动析构机制自动回收内存，确保即使外部抛出异常，所有已成功完成初始化的内部成员仍能正确进行资源清理。
