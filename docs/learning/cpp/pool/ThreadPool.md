---
title: 线程池
tags:
  - C++
  - Concurrency
  - ThreadPool
---

# 线程池实现与演进

!!! abstract "摘要"

    本文基于一份经典的 C++11 线程池实现，分析其核心结构与工作原理，并探讨如何利用 C++17 与 C++20 的现代语言特性对其进行重构。同时，本文总结了该基础实现的性能特征与工业级线程池的常见设计演进方向。

## 经典实现分析

该经典实现的核心是一个 **固定大小工作线程集合 + 全局任务队列 + 互斥锁 + 条件变量** 的并发模型。

!!! note "参考项目"

    本文的基础代码实现参考自 GitHub 开源项目：[progschj/ThreadPool](https://github.com/progschj/ThreadPool/tree/master){target=_blank}。

其核心执行流程如下：

- 构造函数中创建并启动若干工作线程。
- 工作线程阻塞等待条件变量的唤醒。
- `enqueue` 方法将传入的任务封装为 `std::packaged_task`，推入任务队列，并向调用方返回 `std::future` 对象。
- 析构函数中设置停止标志位，唤醒所有挂起的线程，并通过 `join()` 逐个回收线程资源。

???+ note "C++11 经典实现代码"

    ```cpp title="thread_pool.h"
    #ifndef THREAD_POOL_H
    #define THREAD_POOL_H

    #include <vector>
    #include <queue>
    #include <memory>
    #include <thread>
    #include <mutex>
    #include <condition_variable>
    #include <future>
    #include <functional>
    #include <stdexcept>

    class ThreadPool {
    public:
        ThreadPool(size_t);

        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args)
            -> std::future<typename std::result_of<F(Args...)>::type>;

        ~ThreadPool();

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop;
    };

    inline ThreadPool::ThreadPool(size_t threads)
        : stop(false)
    {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock,
                            [this] { return this->stop || !this->tasks.empty(); });

                        if (this->stop && this->tasks.empty()) {
                            return;
                        }

                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    template<class F, class... Args>
    auto ThreadPool::enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    inline ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

    #endif
    ```

该实现的结构具有以下特点：

- 代码精简，逻辑清晰，非常适合作为并发编程的教学范例。
- 任务执行结果通过 `std::future` 返回，提供了直观的异步数据获取接口。
- 采用单一全局任务队列模型，所有工作线程在同一把互斥锁上发生竞争。

## C++17 现代化改造

在 C++17 标准下，我们主要利用更新的类型推导与并发设施来简化代码编写，使代码更符合现代 C++ 规范。

???+ note "C++17 优化版代码"

    ```cpp title="thread_pool.h"
    #ifndef THREAD_POOL_H
    #define THREAD_POOL_H

    #include <vector>
    #include <queue>
    #include <memory>
    #include <thread>
    #include <mutex>
    #include <condition_variable>
    #include <future>
    #include <functional>
    #include <stdexcept>
    #include <type_traits>

    class ThreadPool {
    public:
        explicit ThreadPool(size_t threads);

        template<class F, class... Args>
        [[nodiscard]] auto enqueue(F&& f, Args&&... args)
            -> std::future<std::invoke_result_t<F, Args...>>;

        ~ThreadPool();

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop = false;
    };

    inline ThreadPool::ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F, class... Args>
    auto ThreadPool::enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock lock(queue_mutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    inline ThreadPool::~ThreadPool() {
        {
            std::scoped_lock lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& worker : workers) {
            worker.join();
        }
    }

    #endif
    ```

主要的改造点包括：

- 利用 `std::invoke_result_t` 替换已弃用的 `std::result_of`，提供更标准的返回值类型推导。
- 引入 CTAD（类模板参数推导），将 `std::unique_lock<std::mutex>` 简写为 `std::unique_lock`，降低模板代码的冗余。
- 在析构函数中使用 `std::scoped_lock` 替代 `std::unique_lock`，更贴合无需配合条件变量且仅需局部加锁的作用域语义。
- 采用类成员就地初始化（如 `bool stop = false;`），精简构造函数的初始化列表。
- 为 `enqueue` 方法添加 `[[nodiscard]]` 属性，在调用方忽略 `std::future` 返回值时触发编译器警告，预防异步任务异常被静默吞没。

## C++20 版本进一步演进

C++20 提供了更强大的线程管理工具与 Lambda 捕获机制，可进一步提升代码的安全性与可读性。

???+ note "C++20 演进版代码"

    ```cpp title="thread_pool.h"
    #ifndef THREAD_POOL_H
    #define THREAD_POOL_H

    #include <vector>
    #include <queue>
    #include <memory>
    #include <thread>
    #include <mutex>
    #include <condition_variable>
    #include <future>
    #include <functional>
    #include <stdexcept>
    #include <type_traits>

    class ThreadPool {
    public:
        explicit ThreadPool(size_t threads);

        template<class F, class... Args>
        [[nodiscard]] auto enqueue(F&& f, Args&&... args)
            -> std::future<std::invoke_result_t<F, Args...>>;

        ~ThreadPool();

    private:
        std::vector<std::jthread> workers;
        std::queue<std::function<void()>> tasks;

        std::mutex queue_mutex;
        std::condition_variable condition;
        bool stop = false;
    };

    inline ThreadPool::ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    template<class F, class... Args>
    auto ThreadPool::enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [f = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return std::invoke(std::move(f), std::move(args)...);
            }
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock lock(queue_mutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    inline ThreadPool::~ThreadPool() {
        {
            std::scoped_lock lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
    }

    #endif
    ```

核心改进包括：

- 使用 `std::jthread` 替换 `std::thread`。`std::jthread` 能够在析构时自动完成 `join()` 操作，提供了更安全、不易泄露的线程生命周期管理模型。
- 利用 Lambda 捕获包展开替换 `std::bind`。不仅提升了代码的直观可读性，还能借助 `std::invoke` 统一支持普通函数、函数对象以及成员函数指针的调用，且对仅支持移动语义（move-only）的参数更加友好。

## 经典实现的性能特征与工程局限

该单队列模型在处理粗粒度、耗时较长的异步任务时表现良好，但在高并发、低延迟或短任务密集的场景下存在显著的性能瓶颈。

- **全局锁竞争开销**：所有工作线程在获取任务时都会竞争唯一的 `queue_mutex`。当系统核心数增多或任务执行时间极短时，线程的大部分 CPU 周期会消耗在锁竞争与上下文切换上。
- **动态内存分配惩罚**：任务队列中保存的 `std::function`，以及任务封装过程中的 `std::packaged_task` 和 `std::shared_ptr` 均可能触发堆内存分配。高频触发堆分配会导致分配器成为新的瓶颈，并增加内存碎片化。
- **缓存失效问题**：多个核心频繁修改同一个中心化队列的数据结构与相应的锁状态，容易引发缓存行失效，导致缓存一致性协议流量激增，拉低整体访存效率。
- **缺乏高级调度原语**：基础实现无法支持优先级调度、任务取消、依赖关系图以及感知系统物理拓扑结构（如 NUMA 亲和性）。

## 工业级线程池的设计演进

针对上述局限性，现代工业级线程池通常会向更完备的**任务调度系统**方向演进，引入多种高级机制以压榨硬件性能。

- **工作窃取（Work-Stealing）机制**：为每个工作线程维护局部的双端队列。线程优先处理本地队列中的任务，只有当本地队列耗尽时，才去其他线程的队列尾部“窃取”任务。这极大地分散了同步冲突点。
- **无锁结构与细粒度控制**：在关键路径上采用基于原子指令的无锁（Lock-free）数据结构，或是引入自旋锁、分段锁等轻量级同步原语，以缩短临界区的范围与竞争概率。
- **内存对象池化与静态分配**：为了规避 `new/delete` 的开销，工程实践中常采用预分配的环形缓冲区、内存池复用任务节点，或引入更轻量的可调用对象包装器以替代开销较大的通用 `std::function`。
- **NUMA 感知与亲和性绑定**：在多 CPU 插槽架构下，通过将线程与核心进行硬绑定，建立线程本地的内存就近访问策略，确保数据与执行代码始终在同一 NUMA 节点的缓存甚至本地内存内流转。

*[CTAD]: Class Template Argument Deduction
*[NUMA]: Non-Uniform Memory Access
