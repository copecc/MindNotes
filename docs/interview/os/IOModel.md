---
comments: false
title: IO 模型
show_question_index: true
question_index_title: 问题导航
question_index_collapsed: true
question_index_style: abstract
question_index_levels:
  - 2
---

# IO 模型

## Proactor 模式是如何感知数据就绪的？与 Reactor 有何不同？

在 Proactor（异步 I/O）模式中，系统内核利用 DMA 将网卡数据直接拷贝至用户空间缓冲区。完成后，内核生成一个完成事件（包含传输字节数与上下文信息）并压入 I/O 完成队列。应用进程从队列提取事件并直接触发回调进行业务处理。

Reactor 模式（如 epoll）仅通知应用进程“资源可读/写”，应用进程仍需主动发起 read/write 系统调用来完成数据拷贝；Proactor 则由操作系统包揽了完整的 I/O 读写与数据搬运。

??? note "应用进程感知数据就绪的机制"

    应用进程感知数据准备就绪的机制主要依赖于操作系统提供的 I/O 完成队列（I/O Completion Queue）和事件通知接口。具体流程如下：

    1. 注册与提交（Submission）：应用进程调用操作系统的异步 I/O 接口（如 Windows 的 `WSARecv` 或 Linux 的 `io_uring_submit`），向内核提交 I/O 请求。提交时必须携带文件描述符（Socket）、用户空间缓冲区地址、请求读取的字节数以及一个用于标识该请求的上下文对象（Context/Token）。
    2. 内核处理与直接内存访问（DMA）：内核接管该请求，并在后台等待网卡数据到达。数据到达后，网卡通过直接内存访问（DMA，Direct Memory Access）将数据写入内核缓冲区，随后内核利用 CPU 将数据从内核缓冲区拷贝到应用进程之前指定的**用户空间缓冲区**。
    3. 完成事件入队（Completion Enqueue）：数据拷贝完成后，内核会生成一个**完成事件（Completion Event）**。该事件包含操作结果（成功或错误码）、实际传输的字节数以及最初传入的上下文对象。内核将此事件压入由操作系统维护的 I/O 完成队列中。
    4. 应用进程获取结果（Reaping）：应用进程通常会分配一个或多个工作线程，这些线程调用特定的系统 API（例如 Windows 的 `GetQueuedCompletionStatus`）阻塞等待或轮询 I/O 完成队列。当完成事件出队时，工作线程提取上下文对象，得知对应的缓冲区内已有完整数据，并直接调用预先绑定的回调函数（Callback）进行业务逻辑处理。

---

## Linux 的 io_uring 是如何解决传统 epoll 性能瓶颈的？

??? note "传统 epoll 的性能瓶颈"

    `epoll` 的工作流需要多次调用系统 API。无论是通过 `epoll_ctl` 注册或修改文件描述符（File Descriptor, FD），还是通过 `epoll_wait` 阻塞等待事件就绪，每次调用都强制 CPU 进行一次从用户态到内核态的上下文切换。

    `epoll` 仅提供 I/O 就绪通知。当 `epoll_wait` 返回后，应用进程仍需主动调用 `read` 或 `write` 函数来执行实际的 I/O 操作。这不仅引入了额外的系统调用，还伴随着数据从内核空间缓冲区到用户空间缓冲区的内存拷贝。

Linux 内核 5.1 引入的 `io_uring` 是一个全新的原生异步 I/O 框架，专为高性能网络和存储 I/O 设计。它通过以下几个核心机制，从根本上解决了传统 `epoll` 的性能瓶颈：

1. 共享内存环形队列： 引入提交队列（SQ）和完成队列（CQ），通过 mmap 映射至用户态内存。应用层与内核交换 I/O 请求和结果时彻底消除了数据拷贝开销。
2. 系统调用聚合： 允许在用户态连续组装多个 SQE（提交项），通过一次 io_uring_enter 批量提交，大幅降低了系统调用频率。
3. SQPOLL 模式： 内核分配专用线程持续轮询 SQ。应用层更新尾指针即可触发 I/O，实现了零系统调用的极致异步处理。

---