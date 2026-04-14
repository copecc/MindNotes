---
title: GEMM
tags:
  - CUDA
  - GEMM
  - cuBLAS
  - Tensor Core
  - WMMA
---

# GEMM

!!! abstract "摘要"

    本文梳理 CUDA 中通用矩阵乘法 (GEMM) 算子的演进与优化路径。从朴素的 Naive GEMM 实现出发，经历 Tiled GEMM、共享内存复用、寄存器分块以及向量化加载，最终逐步逼近硬件算力上限。并附带探讨 Tensor Core / WMMA 以及 cuBLAS 相关的工程选型。

## 核心问题与运算特征

GEMM (General Matrix Multiply) 是 CUDA 中典型的高频核心算子。深度学习中的线性层、卷积变换、Attention 中的大量操作都可以归约为 GEMM 或 batched GEMM。它同时具有高计算密度和明确的数据复用结构的特点。很多深度性能优化技术，例如 Tiling、共享内存复用、向量化加载、软件流水线 (Software Pipelining) 以及 Tensor Core 都会在 GEMM 中集中体现。

在讨论具体优化的实现之前，需要首先明确通用矩阵乘法的理论计算量。GEMM 的基本形式为：

$$
C = \alpha \cdot op(A)op(B) + \beta \cdot C
$$

结合现有实现，假设输入矩阵 $A$ 的维度为 $M \times K$，矩阵 $B$ 的维度为 $K \times N$，所得结果矩阵 $C$ 的维度为 $M \times N$。

结果矩阵 $C$ 中的每一个元素 $C_{i,j}$ 均由矩阵 $A$ 的第 $i$ 行与矩阵 $B$ 的第 $j$ 列进行点积（Dot Product）计算得到。计算单个元素 $C_{i,j}$ 需要 $K$ 次对应元素的乘法以及 $K - 1$ 次乘积的累加。

结果矩阵 $C$ 共有 $M \times N$ 个元素，全局总运算量为 $M \times N \times K$ 次乘法与 $M \times N \times (K - 1)$ 次加法。

在高性能计算设备（如 GPU）中，一次乘法与一次加法通常会被组合为单次的乘累加操作（Multiply-Accumulate, MAC）。因此，总的 MAC 执行次数约为 $M \times N \times K$。对于边长为 $N$ 的方阵，标准矩阵乘法的时间复杂度为 $O(N^3)$。

对于大规模矩阵运算，实际执行性能往往最先受限于全局内存访问与数据搬运的物理带宽上限，而非单纯的算术逻辑单元能力。这也是后续优化路线致力于提升计算访存比（Arithmetic Intensity）的核心工程原因。

## 优化路线概览

CUDA GEMM 算子的优化主要分为以下几个阶段：

1. **朴素实现 (Naive Implementation)**：直接使用多维线程网格映射矩阵元素进行计算。
2. **访存合并 (Coalesced Memory Access)**：调整线程与数据映射关系，满足全局内存合并访问。
3. **共享内存分块 (Shared Memory Tiling)**：引入共享内存缓存数据，降低全局内存访问延迟与带宽压力。
4. **一维寄存器分块 (1D Register Tiling / Thread Coarsening)**：通过线程粗化复用寄存器数据，增加计算访存比，减少共享内存访问。
5. **二维寄存器分块 (2D Register Tiling)**：采用外积 (Outer Product) 形式进行二维分块，最大化数据复用。
6. **向量化访存 (Vectorized Memory Loads)**：利用 `float4` 等内置类型提升一次性内存加载的总线宽度。
7. **共享内存填充与重排 (Padding & Swizzling)**：消除共享内存 bank conflict 并优化全局内存访存模式。

## 原理与实现细节

### 朴素实现
基础思路是将计算资源映射到输出矩阵 $C$ 上。每一个线程负责计算矩阵 $C$ 中的一个元素。

由于每一次乘加 (FMA) 计算都需要从全局内存中加载对应的行与列，内存访问非常密集，且由于矩阵通常采用行优先 (Row-Major) 存储，访问 $A$ 或 $B$ 时往往存在较差的空间局部性。

???+ note "朴素实现代码"

    ```cpp title="gemm1"
    --8<-- "learning/cuda/kernel/gemm1.cu"
    ```

### 访存合并

CUDA 需要同一个 warp 内部的线程合并访问一段连续的对齐的全局内存地址（即 Memory Coalescing）。
在上面的朴素实现中，随着 `threadIdx.x` 的增长（同一 warp 内部），`row` 发生变化，导致访问 `d_A[row * A_n_cols + k]` 为跳跃访问。同样地，`d_C` 的访问也是非连续的。

优化方式是调换 `row` 和 `col` 对 `threadIdx` 的映射关系，使 `threadIdx.x`（连续线程）对应 `col`，从而保证对 `d_C` 和 `d_B` 取值的内存连续性。

???+ note "访存合并代码"

    ```cpp title="gemm2"
    --8<-- "learning/cuda/kernel/gemm2.cu"
    ```

### 共享内存分块

由于每一个元素的计算都需要多次从全局内存读入相同的行列数据，全局内存带宽将快速成为性能瓶颈。
根据 Roofline Model，若要提升算存上限，必须增加运算访存比 (Arithmetic Intensity)。

引入 Shared Memory 作为 Cache。每次将一块较小的 `TILE_WIDTH` 大小的 $A$ 矩阵和 $B$ 矩阵从全局内存搬入 Shared Memory 中，再由 block 内的线程在这个 Tile 上完成局部矩阵乘加。随后继续处理下一个 $K$ 方向的分块。

通过对比基于全局内存的 Naive GEMM 与基于共享内的 Tiled GEMM，数据复用的巨大差异十分明显：

| 实现策略 | 全局内存 (Global Memory) 复用 | 共享内存 (Shared Memory) 复用 | 运算访存比 (Arithmetic Intensity) 演进方向 |
| --- | --- | --- | --- |
| **Naive GEMM** | 极低。同一个 `A[row, k]` 会被多个线程直接重复读取。 | 无 | 持续受限于 Memory-Bound |
| **Tiled GEMM** | 较高。一次加载满足整个线程块内的计算需求。 | 有（按局部瓦片缓存与反复计算） | 更容易向 Compute-Bound 方向跨越 |

因此，Tiled GEMM 的核心收益不单纯是加速了存取，更重要的是将单次全局内存读取转化为多次有效的计算复用。

??? example "分块计算 (Tiled Multiply-Accumulate) 示意"

    对于输出矩阵 $C$ 中的一个子块 $C_{tile}$，它的计算被分为多个阶段 $t$。在每一个阶段，都会从处于全局内存中的全量矩阵取出对应的 $A_{tile}^{(t)}$ 与 $B_{tile}^{(t)}$ 转入快速的共享内存：

    $$
    \begin{bmatrix}
    C_{0,0} & C_{0,1} \\
    C_{1,0} & C_{1,1}
    \end{bmatrix}_{tile}
    \mathrel{+}=
    \begin{bmatrix}
    A_{0,0} & A_{0,1} \\
    A_{1,0} & A_{1,1}
    \end{bmatrix}_{tile}^{(t)}
    \times
    \begin{bmatrix}
    B_{0,0} & B_{0,1} \\
    B_{1,0} & B_{1,1}
    \end{bmatrix}_{tile}^{(t)}
    $$
    
    沿着 $K$ 维度不断滑动取出下一个区块，直至将所有对应的分块全部被乘加完毕。通过在 Block 内复用这载入的 $2 \times 2$ 晶块，实现了内存访存带宽的骤减。

???+ note "共享内存分块代码"

    ```cpp title="gemm3"
    --8<-- "learning/cuda/kernel/gemm3.cu"
    ```

???+ note "同步屏障"

    每加载一个 Tile 完成后，需要调用 `__syncthreads()` 防止读取发生脏数据覆盖。
    当计算完成后，由于同一个 block 还需读取下一个 Tile 的新数据覆写此共享内存块，所以必须在块计算末尾再次调用 `__syncthreads()`。

### 寄存器分块

即便降低了对全局内存的请求，Shared Memory 依然比 Register (寄存器) 慢，同时可能面临 Bank Conflict 的风险与指令延迟瓶颈。

线程粗化 (Thread Coarsening) 的设计使一个线程不再只计算一个输出元素，而是负责一组连续的输出元素。这样在一个线程内部，从 Shared Memory 加载的数据可以被多次重复用于不同的乘加计算。引入局部数组 `float dot_prod[COARSE_FACTOR]` 缓冲多条运算结果，提升了计算访存比，从而更有效地掩盖访存与指令延迟。

#### 优化的核心机制

这部分设计的核心目标是最大化内存带宽利用率，具体机制如下：

- **寄存器复用 (Register Reuse)**：在代码中，设置了 `COARSE_FACTOR = 8`，意味着每个线程负责计算输出矩阵 $C$ 中同一列上的 $8$ 个连续元素。在内层循环中，共享内存中的 $B$ 矩阵元素读取一次即可存入寄存器，随后该寄存器变量被连续使用 $8$ 次，与 $A$ 矩阵分块中的 $8$ 个不同元素进行乘法累加。这种机制将对共享内存 $B$ 分块的读请求减少了 $87.5\%$。
- **合并全局内存访问 (Memory Coalescing)**：矩阵 $A$ 和 $B$ 在共享内存中的分块维度不同（如 $A$ 为 $64 \times 8$，$B$ 为 $8 \times 64$）。为了在填充 Shared Memory 时保持连续线程对全局内存的合并访问，代码使用了一维线程块，并通过坐标映射关系，使相同的一维线程索引在读取这两种维度的数据时，分别对应出符合连续访问要求的逻辑坐标。
- **指令级并行 (Instruction-Level Parallelism, ILP)**：通过使用局部数组进行累加，编译器通常会将最内层的循环完全展开。这允许硬件发射多条无数据依赖的浮点乘加（FMA）指令，从而有效隐藏指令延迟。

#### 坐标与索引映射逻辑

为了理解线程映射，首先明确线程块（Thread Block）的规模。以分块大小 $64 \times 64$、`COARSE_FACTOR=8` 为例，分块 $A$ ($64 \times 8 = 512$ 个元素) 与分块 $B$ ($8 \times 64 = 512$ 个元素) 决定了线程块需包含 512 个线程（一维索引 `threadIdx.x` 范围 $0 \sim 511$）。

**1. 负责数据加载的视图坐标（1D $\rightarrow$ 2D）**

为了保证从全局内存加载数据时的合并访存，连续的线程 (`threadIdx.x`) 必须访问连续的内存地址（即目标矩阵的列元素）。在 Tiled 块的读取中，`A` 和 `B` 经常展现不同的物理跨度。假设 `A 分块` 维度为 $128 \times 8$，`B 分块` 为 $8 \times 128$，则由于它们所在的矩阵均按行主序 (Row-Major) 存储：
 
- 读取 `A` 分块时关心的是行向复用（使线程沿着短边的 $8$ 个元素加载，即横向步进小）。
- 读取 `B` 分块时关心的是列向复用（使同一个 Warp 横向覆盖一整段连续的长边 $128$ 列块）。

具体的逻辑坐标划分公式如下：

- **矩阵 $A$ 的加载视图**：目标是填充 `TILE_A_ROWS` $\times$ `TILE_A_COLS` 的共享内存。

$$
A_{view}^{(y)} = \lfloor \frac{\text{threadIdx.x}}{TILE\_A\_COLS} \rfloor, \quad A_{view}^{(x)} = \text{threadIdx.x} \pmod{TILE\_A\_COLS}
$$

通过对列数求整除与求余，相邻的线程会访问相同的行和相邻的列，确保了对 $A$ 矩阵（按行优先存储）全局内存的连续读取。

- **矩阵 $B$ 的加载视图**：目标是填充 `TILE_A_COLS` $\times$ `TILE_B_COLS` 的共享内存。

$$
B_{view}^{(y)} = \lfloor \frac{\text{threadIdx.x}}{TILE\_B\_COLS} \rfloor, \quad B_{view}^{(x)} = \text{threadIdx.x} \pmod{TILE\_B\_COLS}
$$

同理，每 `TILE_B_COLS` 个连续的线程负责读取 $B$ 矩阵分块中的一整行，完全契合硬件的合并访问要求。

??? example "一维线程网格重塑 (Thread Reshaping) 示意"

    假设我们有一个极简的包含 $8$ 个线程的一维线程块（`threadIdx.x` 范围为 $0 \sim 7$），我们要用它去合并加载两种不同形状的局部阵列：

    **第一种：偏“瘦高”的 $A$ 分块 (设它为 $4 \times 2$)**，此时 `TILE_A_COLS = 2`。
    根据公式推求，这 $8$ 个连续的线性线程将被自然重塑为以下阵列：

    $$
    A_{view} =
    \begin{bmatrix}
    T_0 & T_1 \\
    T_2 & T_3 \\
    T_4 & T_5 \\
    T_6 & T_7
    \end{bmatrix}_{4 \times 2}
    $$

    **第二种：偏“扁平”的 $B$ 分块 (设它为 $2 \times 4$)**，此时 `TILE_B_COLS = 4`。
    按照对应的分块列数基准计算，同一批 $8$ 个线程在加载 $B$ 矩阵时，会被重新映射为另一种逻辑形态：

    $$
    B_{view} =
    \begin{bmatrix}
    T_0 & T_1 & T_2 & T_3 \\
    T_4 & T_5 & T_6 & T_7
    \end{bmatrix}_{2 \times 4}
    $$

    在这两种不同的形态维度下，最内侧的连续内存地址单元始终由**序号连续**的线程（如前者的 $T_0, T_1$ 与后者的 $T_0, T_1, T_2, T_3$）访问。这种基于索引运算的映射关系，有效确保了无论读取跨度差异极大的 $A$ 还是 $B$，硬件均能触发一致且高效的连续合并访存（Memory Coalescing）。

**2. 负责计算与输出的全局坐标**

在计算阶段，当前线程块需要生成一个 $64 \times 64$ 尺寸的输出矩阵 $C$ 分块（共 $4096$ 个元素）。由 $512$ 个线程分担，每个线程计算同一列上的 $8$ 个连续行元素。

- **列坐标 (`col`)**：连续的线程被分配到连续的列，保证了最终写回矩阵 $C$ 时可以合并访存。
- **行坐标 (`row`)**：表示 $512$ 个线程在纵向被分成了 $8$ 个线程组。每个线程组负责 $8$ 行，组与组之间的行偏移为 $0, 8, 16, \dots, 56$。

在内层点积循环中，存放乘加结果的局部数组的相对偏移量恰好与矩阵 $A$ 共享分块的行索引严密对齐。

???+ tip "一维寄存器分块代码"

    ```cpp title="gemm4"
    --8<-- "learning/cuda/kernel/gemm4.cu"
    ```

### 二维寄存器分块

上述的一维分块仅在一个维度展开，寄存器复用率仍有提升空间。引入外积的计算模式，二维寄存器分块 (2D Register Tiling) 让每个线程负责计算 `COARSE_Y * COARSE_X` 的二维输出子块。单线程只需将对应的 `Register_A` 列片段和 `Register_B` 行片段分别加载一次，即可在其内部执行 `COARSE_Y * COARSE_X` 次乘加运算。

$$
Arithmetic Intensity \propto \frac{(COARSE\_Y \times COARSE\_X)}{(COARSE\_Y + COARSE\_X)}
$$

该比例反映了计算次数与访存次数的比值。增加分块边界可以有效提高计算密度，向 Compute-bound 方向靠拢。

??? example "外积 (Outer Product) 覆盖输出子块示意"

    在最内层的循环结构中，每一个线程的寄存器数组被分为两组：分别保持长为 $COARSE\_Y$ 的列向量片段 `regA` 与长为 $COARSE\_X$ 的行向量片段 `regB`。它们被利用执行局部外积：

    $$
    \begin{bmatrix}
    \text{regA}_0 \\
    \text{regA}_1 \\
    \vdots \\
    \text{regA}_{Y-1}
    \end{bmatrix}
    \times
    \begin{bmatrix}
    \text{regB}_0 & \text{regB}_1 & \dots & \text{regB}_{X-1}
    \end{bmatrix}
    =
    \begin{bmatrix}
    A_0 B_0 & A_0 B_1 & \dots & A_0 B_{X-1} \\
    A_1 B_0 & A_1 B_1 & \dots & A_1 B_{X-1} \\
    \vdots & \vdots & \ddots & \vdots \\
    A_{Y-1} B_0 & A_{Y-1} B_1 & \dots & A_{Y-1} B_{X-1}
    \end{bmatrix}
    $$
    
    单线程仅需将其加载入寄存器一次（执行 $X+Y$ 次读取），便能在局部完成整个维度的外积操作（即 $X \times Y$ 次 FMA 计算）并直接累加进局部二维寄存器中。通过提升操作并发数并减少内存读取量，从根本上突破了物理内存带宽的瓶颈。

#### 优化的核心机制

- **二维寄存器复用 (2D Register Tiling)**：每个线程负责计算输出矩阵 $C$ 中的一个二维子块（对于 $8 \times 8$ 设置，对应 $64$ 个元素）。最内层循环中，线程从 Shared Memory 分别读取元素至两个寄存器数组，并执行嵌套乘加运算。此机制下，执行 $64$ 次乘加仅需从 Shared Memory 加载 $16$ 个元素，大幅降低了共享内存的带宽压力与读取延迟。
- **分离数据加载与计算维度 (Thread Decoupling)**：采用二维分块后，完成子块计算所需的总线程数大幅减少。例如，$128 \times 128$ 分块总计需要计算 $16384$ 个元素，若每线程处理 $64$ 个元素，则只需 $256$ 个线程。然而对应的 $A$ 和 $B$ 共享分块各有 $2048$ 个元素，少量线程无法在单次指令内加载完毕。通过引入步长（Stride）循环机制，赋予单个线程执行多轮连续寻址读取的能力，从而将内存加载与乘加计算的线程网格相互独立。

#### 坐标与索引映射逻辑

- **线程规模与步长计算**：根据分块与粗化因子推断所需的总线程数 $N_{threads}$。由于线程总量少于待加载数据量，步长量定义了每个线程在多轮共享内存填充时纵向跨越的行数：

$$
Stride_A = \frac{N_{threads}}{TILE\_A\_COLS}, \quad Stride_B = \frac{N_{threads}}{TILE\_B\_COLS}
$$

- **全局内存加载的视图坐标**：核心要求在于确保连续的计算线程访问连续的数据列地址。一维线程在加载阶段通过如下坐标运算实现对应数据的定位：

$$
A_{view}^{(y)} = \lfloor \frac{\text{threadIdx.x}}{TILE\_A\_COLS} \rfloor, \quad A_{view}^{(x)} = \text{threadIdx.x} \pmod{TILE\_A\_COLS}
$$

$$
B_{view}^{(y)} = \lfloor \frac{\text{threadIdx.x}}{TILE\_B\_COLS} \rfloor, \quad B_{view}^{(x)} = \text{threadIdx.x} \pmod{TILE\_B\_COLS}
$$

在加载矩阵 $A$ 时，以计算出的 $A_{view}^{(y)}$ 为基础偏移，线程在循环中每次向下跨越 $Stride_A$ 行，而横坐标 $A_{view}^{(x)}$ 保持不变，始终维持连续横向读取的最优内存合并访问。矩阵 $B$ 的读取同理，利用列方向对齐进行跨越。

- **计算与输出的全局坐标**：在计算阶段，一维线程索引被映射为对应输出子块的二维定位基准：

$$
row = COARSE\_Y \times \lfloor \frac{\text{threadIdx.x}}{TILE\_B\_COLS / COARSE\_X} \rfloor
$$

$$
col = COARSE\_X \times \left( \text{threadIdx.x} \pmod{TILE\_B\_COLS / COARSE\_X} \right)
$$

藉由上述逻辑网格坐标转换，可精确定出该线程负责的输出子矩阵的起始行与列偏移。

- **结果写回至全局内存**：乘加循环结束后，利用所求得的基础定位坐标 (`row`, `col`) 与嵌套循环的二维增量索引，将寄存器数组中缓存的所有运算结果逐一归写至对应全局内存坐标。

??? example "二维加载步长 (Stride Loading) 示意"

    假设在 $128 \times 128$ 分块、$8 \times 8$ 二维寄存器粗化的配置中，总计使用 $256$ 个线程。
    而 $A$ 矩阵对应的 Shared Memory 分块尺寸为 $128 \times 8$（共 $1024$ 个元素）。
    
    由于总线程数远小于单次需加载的数据量，线程网格必须在列方向循环向下“步进”（Stride）。由于 $A$ 的列宽为 $8$（`TILE_A_COLS`），这一批 $256$ 个一维线程在视觉上构成了一个 $32 \times 8$ 的二维拾取网格：

    $$
    A_{load\_grid} =
    \begin{bmatrix}
    T_0 & T_1 & \dots & T_7 \\
    T_8 & T_9 & \dots & T_{15} \\
    \vdots & \vdots & \ddots & \vdots \\
    T_{248} & T_{249} & \dots & T_{255}
    \end{bmatrix}_{32 \times 8}
    $$
    
    在第一步加载时，这 $256$ 个线程从全局内存读取并覆盖了 $A$ 分块的前 $32$ 行。接下来由于步长 $Stride_A = 32$，该线程网格会在加载循环中纵向平移 $3$ 次，每次步进 $32$ 行，在维持极高读取合并率的前提下，完整填满容量达 $128$ 行的局部缓存区。

???+ tip "二维寄存器分块代码"

    ```cpp title="gemm5"
    --8<-- "learning/cuda/kernel/gemm5.cu"
    ```

### 向量化访存

由于 CUDA 设备的内存事务处理具有固定的请求宽度设计，若以单 `float` 单元为循环单位加载全局内存，将导致较低的数据总线利用率与较多的指令开销。

通过使用 `float4` 等内建向量数据类型拷贝，单条 `LGD.E.128` (128-bit global load) 即可将 4 个 float（16 Bytes）一次性加载至寄存器并转入 Shared Memory。这一转变精简了读取次数与地址寻址开销。

???+ tip "向量化访存代码"

    ```cpp title="gemm6"
    --8<-- "learning/cuda/kernel/gemm6.cu"
    ```

### 共享内存填充与重排

在利用共享内存缓存矩阵块时，由于 Shared Memory 物理上被划分为 32 个交错的存储体（Memory Banks），每个 Bank 每个时钟周期只能处理一次单字访问。如果同一个 Warp 中的多个线程同时访问同一个 Bank 中的不同地址，就会引发 Bank Conflict（存储体冲突），导致内存请求被串行化，成倍增加访问延迟。

#### 1. Bank Conflict 的产生

假设我们在 Shared Memory 中以普通的二维数组 `__shared__ float sh_A[64][32]` 存储某块数据。由于 `float` 占 4 字节，刚好一个跨度为 1 的索引增加对应下一个 Bank。由于列宽恰等于 32，矩阵中同一列的相邻两行元素（例如 `sh_A[0][0]` 与 `sh_A[1][0]`）将映射到完全相同的 Bank 0。

如果在计算阶段，某一个 Warp 内部的连续线程去读取同一列上的不同行数据，这 32 个地址请求将散落至同一个 Bank 内部排队碰撞，进而触发严重的 32-way Bank Conflict（最高级指令阻塞）。

#### 2. Padding 填充机制

优化的最简单直观方法是改变二维数组的内存步长，引入空间填充（Padding）。例如将声明修改为 `__shared__ float sh_A[64][32 + 1]`（即每一行末尾引入一列无效的垫片，打破原来的 32 的整数倍周期）：

??? example "Padding 错开 Bank 排位示意"

    当采用原始的 `[32]` 宽度时，寻址跨度使得每行首元素全都落在同一个 Bank 0 上：

    $$
    \begin{matrix}
    \text{行 } 0: & \textbf{B}_0 & B_1 & \dots & B_{31} \\
    \text{行 } 1: & \textbf{B}_0 & B_1 & \dots & B_{31} \\
    \text{行 } 2: & \textbf{B}_0 & B_1 & \dots & B_{31}
    \end{matrix}
    $$

    修改为 `[33]` 宽度（+1 Padding）后，由于每一行多占用了 1 个位置，下一行的起始 Bank 顺延错位偏移：

    $$
    \begin{matrix}
    \text{行 } 0: & \textbf{B}_0 & B_1 & \dots & B_{31} & | & pad(B_0) \\
    \text{行 } 1: & B_1 & B_2 & \dots & \textbf{B}_0 & | & pad(B_1) \\
    \text{行 } 2: & B_2 & B_3 & \dots & B_1 & | & pad(B_2)
    \end{matrix}
    $$
    
    在这种填充形态下，原本处于逻辑同列的元素 `sh_A[0][0]` (此归属物理 $B_0$)、`sh_A[1][0]` (此归属物理 $B_1$)、`sh_A[2][0]` (此归属物理 $B_2$) 便自然散列到了不同的 Bank 硬件中。当 Warp 发起纵向连续读取请求时，能够实现理论上的无冲突全并行载入。

#### 3. Swizzling 重排设计

尽管 Padding 能够高效解决规整行列的读取冲突，但它会额外占用宝贵的 Shared Memory 容量空间。更为高级且工业界常用的做法是采用异或交织（XOR Swizzling）。在存入和读取 Shared Memory 时，借助按位异或运算显式地打乱原有的线性映射：

$$
\text{offset} = \text{row} \times COL\_WIDTH + (\text{row} \oplus \text{col})
$$

通过异或变换，同一列内的不同行元素会被强制散列到随机但不重复的逻辑 Bank 编号上。

??? example "XOR Swizzling 错开 Bank 排位示意"

    为了简化说明，假设硬件只有 $4$ 个 Bank，局部矩阵的分块列宽刚好也为 $4$。

    **未重排前 (普通二维数组映射)**，同一列的元素由于周期相同，毫无悬念地全都会掉入同一个 Bank (例如第 0 列全属 $B_0$)：

    $$
    \begin{matrix}
    \text{行 } 0 (\text{row}=0): & \textbf{B}_0 & B_1 & B_2 & B_3 \\
    \text{行 } 1 (\text{row}=1): & \textbf{B}_0 & B_1 & B_2 & B_3 \\
    \text{行 } 2 (\text{row}=2): & \textbf{B}_0 & B_1 & B_2 & B_3 \\
    \text{行 } 3 (\text{row}=3): & \textbf{B}_0 & B_1 & B_2 & B_3 
    \end{matrix}
    $$

    **采用 XOR 重排后**，元素的逻辑落点依据 $\text{Bank} = \text{row} \oplus \text{col}$ 发生置换。由于异或可逆且无进位的特性，其散列保障了同一列内的 Bank 编号不会重复：

    $$
    \begin{matrix}
    \text{行 } 0 (\text{row}=00_2): & \textbf{B}_0 & B_1 & B_2 & B_3 \\
    \text{行 } 1 (\text{row}=01_2): & B_1 & \textbf{B}_0 & B_3 & B_2 \\
    \text{行 } 2 (\text{row}=10_2): & B_2 & B_3 & \textbf{B}_0 & B_1 \\
    \text{行 } 3 (\text{row}=11_2): & B_3 & B_2 & B_1 & \textbf{B}_0 
    \end{matrix}
    $$

    此时，若一个 Warp 发起对原逻辑分块**第 0 列的连续读取**，它将并行且独立地拾取底层的 $B_0, B_1, B_2, B_3$。我们在不消耗任何额外 Padding 开销的前提下，从纯数学逻辑上达成了零碰撞。

这在配合使用 `float4` 向量化访问时效果尤为关键，因为 128-bit 的存取面临更苛刻的寻址边界对齐问题。Swizzle 提供了一种纯代数重排机制。由于它不再依赖物理填充（无需插入无意义的 Pad 字节），因此能在打破数据位面的冲突对齐同时，最大限度地节省并消除 Shared Memory 的额外空间浪费。

### 线程块网格重排 (Thread Block Swizzling)

“Swizzling” 在 GPU 编程语境下是一个通用术语，指代对坐标或索引进行非线性映射变换。除了上述用于消除共享内存 Bank Conflict 的内部地址重排，在 CUTLASS 等高性能矩阵库中，还会广泛应用一种针对**线程块 (Thread Block)** 层级的调度 Swizzling 技术，其核心目的是提升 **L2 Cache 的缓存命中率 (Cache Hit Rate)**。

在执行 GEMM 算子时，如果我们按照默认的一维或基于行优先的二维 Grid 启动 Kernel，相邻启动的 Block 会顺着结果矩阵 $C$ 的行方向长距离推进。这会导致它们在计算时虽能复用矩阵 $A$ 的同一段行分块，但对矩阵 $B$ 的访问往往会在物理内存中产生极大的跳跃跨度（不断加载全新的列分块）。这很容易导致有限的 L2 Cache 快速被冲刷（Cache Eviction），当后续行逻辑开始计算时，往往又需要重新跨越全局内存读取相同的 $B$ 数据。

**Thread Block Swizzling 的核心思想**：通过对设备内置的 `blockIdx` 进行位运算变换，将原本在网格中线性伸展的执行顺序“折叠”成二维近邻的局部块（Tile）执行顺序。使得在时间上连续投入执行的 Block 簇，在映射的物理空间上同时贴近，进而能够聚拢复用 $A$ 和 $B$ 的共享缓存。

假设我们通过 API 启动了一个 `gridDim` 为 `(16, 1, 1)` 的一维度全卷展网格。为了实现空间局部性，按照 `log_tile = 2` (即目标聚合 Tile 维度为 $4 \times 4$) 施行简单的重排：

???+ note "Thread Block Swizzling 坐标映射代码"

    ```cpp title="block_swizzle.cu"
    int block_idx_x = blockIdx.x >> log_tile;
    int block_idx_y = (blockIdx.y << log_tile) + (blockIdx.x & ((1 << log_tile) - 1));
    ```

??? example "一次性网格执行顺序折叠示意"

    变换前（基于传统硬件下发的线性推进调度，空间局部性极差）：

    $$
    \begin{bmatrix}
    0 & 1 & 2 & 3 & 4 & 5 & 6 & 7 & 8 & 9 & 10 & 11 & 12 & 13 & 14 & 15
    \end{bmatrix}
    $$

    变换后（依据上面的公式按 $4 \times 4$ 空间局部聚拢后的物理等效分布）：

    $$
    \begin{bmatrix}
    0 & 4 & 8  & 12 \\
    1 & 5 & 9  & 13 \\
    2 & 6 & 10 & 14 \\
    3 & 7 & 11 & 15
    \end{bmatrix}
    $$

    通过这一重载，原本排在底层时间线上第 $0,1,2,3$ 顺位启动的计算任务，被物理上“聚拢”成了一个小方块结构，它们在计算时会集中覆盖 $A$ 和 $B$ 的对应局部领域。这种从一维“长条流水线”到二维“内聚方块”的执行流折叠，最大化避免了 L2 Cache 的无意义震荡，是最终逼近理论 Compute Bound 的关键工程拼图之一。

??? note "完整优化 Kernel 源码引用"

    下面集成了二维分块、寄存器粗化、步长加载机制以及维度防冲突等所有优化手段的通用矩阵乘加源码，也是前述各阶段核心思想的完整组合。

    ```cpp title="gemm.cu"
    --8<-- "learning/cuda/kernel/gemm.cu"
    ```

### 性能评测

??? note "性能评测 (RTX 6000)"

    在 NVIDIA RTX 6000 Ada Generation GPU 上对逐步优化的 Kernel 进行了性能测试。测试数据包含了矩阵尺寸从 128x128 到 1024x1024 的耗时、加速比、计算吞吐 (GFLOPS) 以及访存带宽 (GB/s)，并且都通过了数值验证。

    #### 耗时与加速比

    所有时间数据为预热 3 次后，重复执行 5 次的平均值 (毫秒)。可以看到在 1024x1024 大小下，性能获得了极大的提升。

    | Kernel | 128x128 (ms) | 256x256 (ms) | 512x512 (ms) | 1024x1024 (ms) | 相对 Base 加速比 (以 1024 计) |
    | --- | --- | --- | --- | --- | --- |
    | **GEMM 01 (Naive)** | 0.062 | 0.116 | 0.444 | 3.515 | 1.00x |
    | **GEMM 02 (Coalesced)** | 0.013 | 0.019 | 0.060 | 0.436 | 8.07x |
    | **GEMM 03 (Shared Mem)** | 0.011 | 0.016 | 0.049 | 0.344 | 10.21x |
    | **GEMM 04 (1D Reg)** | 0.017 | 0.024 | 0.043 | 0.129 | 27.21x |
    | **GEMM 05 (2D Reg)** | 0.019 | 0.032 | 0.059 | 0.113 | 30.99x |
    | **GEMM 06 (Vectorized)** | 0.018 | 0.023 | 0.041 | 0.079 | 44.47x |

    #### 计算吞吐与全局带宽

    计算吞吐 (GFLOPS) 表示设备每秒执行的浮点运算次数。利用更高效的寄存器重用与向量化内存加载，GFLOPS 有了质的飞跃。

    | Kernel | 128 GFLOPS | 256 GFLOPS | 512 GFLOPS | 1024 GFLOPS | 带宽 (GB/s, 1024) |
    | --- | --- | --- | --- | --- | --- |
    | **GEMM 01** | 67.821 | 289.091 | 603.943 | 610.994 | 35.800 |
    | **GEMM 02** | 327.578 | 1740.754 | 4453.926 | 4929.157 | 288.818 |
    | **GEMM 03** | 391.625 | 2121.040 | 5522.500 | 6237.166 | 365.459 |
    | **GEMM 04** | 246.930 | 1401.512 | 6308.202 | 16624.659 | 974.101 |
    | **GEMM 05** | 217.211 | 1035.317 | 4582.577 | 18931.770 | 1109.283 |
    | **GEMM 06** | 239.540 | 1430.063 | 6537.066 | 27171.918 | 1592.105 |

    > *注：上述测试中，较小尺寸 (如 128x128) 由于并行度未能占满整个设备，吞吐偏低；大尺寸 (1024x1024) 能够更好地展现硬件执行密集型计算的潜力。*

    #### 块大小与寄存器分配超参搜索

    针对二维分块及向量化版本 (`gemm6`)，在 $1024 \times 1024$ 矩阵尺寸下对 `TILE_A_ROWS (BM)`, `TILE_B_COLS (BN)`, `TILE_A_COLS (BK)` 以及 `COARSE_Y (TM)`, `COARSE_X (TN)` 进行了网格搜索，最优配置结果如下：

    | 配置参数 (BM x BN / BK / TM x TN) | 1024x1024 耗时 (ms) | 计算吞吐 (GFLOPS) | 相对 Base 加速比 |
    | --- | --- | --- | --- |
    | **64x64, BK=16, 4x4 (最优)** | 0.077 | 27968.873 | **1.94x** (基准为 128x128/8x8) |
    | **64x128, BK=16, 4x8** | 0.079 | 27151.992 | 1.88x |
    | **128x64, BK=16, 8x8** | 0.119 | 18024.149 | 1.25x |
    | **128x128, BK=16, 8x8** | 0.149 | 14448.676 | 1.00x |
    | **128x128, BK=16, 8x4** | 0.170 | 12615.915 | 0.87x |

    实验表明，不同的线程块配置会严重影响设备的占有率 (Occupancy) 与寄存器溢出 (Register Spilling) 的情况，进而影响实际计算吞吐。最佳的 `64x64, BK=16, 4x4` 配置兼顾了合理的并行度与寄存器负载均衡。

## 扩展与工程选型

综合考量数据复用水平、实现复杂度以及对底层 Tensor Core 特性的支持，以下是一级工程落地选型的对比参考表：

| 开发落地方案 | 数据复用水平 | 工程实现复杂度 | 算力单元 (Tensor Core) 调度 | 适用场景 |
| --- | --- | --- | --- | --- |
| **Naive GEMM** | 极低 | 极低 (无分块及共享存储逻辑) | 不可调用 | 仅用于验证基准线、底层索引原型推断 |
| **Tiled GEMM** | 较高 (依赖 Tiling 结构及共享内存/寄存器模型) | 中等 (需构建与维护复杂指针、Bank 及分块布局逻辑) | 可选择性配合 | 特定计算框架定制 kernel、深度介入缓存管理及共享内存计算时 |
| **WMMA** | 极高 | 较高 (包含基于 Fragment 数据形态和协同 API 的编写) | 原生利用 | 针对特定 Tile 切片要求与架构绑定的高吞吐算子编写 |
| **cuBLAS / cuBLASLt**| 极高 | 低 | 原生/自动最优化利用 | 计算框架底层基础推断及模型训练主力路径、工业通用落地需求 |

在完整的 GEMM 算子落地阶段，除了手写上述的标量与寄存器组合，通常还需要引入硬件级的计算加速结构与底层优化的矩阵库。

### Tensor Core 与 WMMA

现代 NVIDIA GPU 架构设计了专门负责矩阵计算的硬件单元 Tensor Core，其浮点吞吐上限远高于传统的 CUDA Core。在代码层面调用 Tensor Core 可以选用 WMMA（Warp Matrix Multiply Accumulate）API，其提供了 Warp 级别的矩阵乘加接口，将一个 Warp 组织成 Tensor Core 友好的小矩阵计算单元。

- **Fragment 数据结构**：通过定义 `wmma::matrix_a`、`wmma::matrix_b` 与 `wmma::accumulator` 类型的 Fragment，将分块数据加载到能够使用 Tensor Core 的寄存器簇内。
- **协同执行**：典型流程是先利用 `wmma::load_matrix_sync` 将矩阵 Tile 装载到 Fragment，再执行 `wmma::mma_sync` 完成以 Warp 为单位的数据内积运算，最后利用 `wmma::store_matrix_sync` 将 Accumulator 写回内存。

???+ note "代码示例：WMMA 结构骨架"

    ```cpp title="wmma_skeleton.cu"
    using namespace nvcuda;

    __global__ void wmma_gemm(const half* A, const half* B, float* C) {
        wmma::fragment<wmma::matrix_a, 16, 16, 16, half, wmma::row_major> a_frag;
        wmma::fragment<wmma::matrix_b, 16, 16, 16, half, wmma::col_major> b_frag;
        wmma::fragment<wmma::accumulator, 16, 16, 16, float> c_frag;

        wmma::fill_fragment(c_frag, 0.0f);
        wmma::load_matrix_sync(a_frag, A, 16);
        wmma::load_matrix_sync(b_frag, B, 16);
        wmma::mma_sync(c_frag, a_frag, b_frag, c_frag);
        wmma::store_matrix_sync(C, c_frag, 16, wmma::mem_row_major);
    }
    ```

此路径能带来极高的加速上限，但也伴随着更为严格的使用边界。Tile 形状和数据类型受支持的矩阵形态限制，数据布局和对齐要求更严格，手写 WMMA Kernel 的复杂度远高于普通 GEMM。因此，许多工程场景优先选择 cuBLAS 或 CUTLASS，而 WMMA 更适合作为定制 Kernel 的专项极限优化手段。

### cuBLAS 工程接口

由于手写极速 GEMM 必须应对特定硬件架构的 Bank 结构、配置占有率策略及底层的指令交错，工业级计算默认交由平台专业库（如 cuBLAS 或 CUTLASS）进行调度。除了内置的 Tiling、Coalescing 与硬件加速机制，库函数还提供了灵活的张量布局与批量处理接口。

#### Leading Dimension (`lda`, `ldb`, `ldc`)

在 API 调用中，`lda`/`ldb`/`ldc` 表征矩阵在物理存储上相邻“主方向”元素的跨度（Leading Dimension）。

当逻辑矩阵取自更大张量的子局部，或者实际物理布局存在 Padding 时，逻辑属性下的行列数与内存地址的真实跨距并不同。`ld` 提供了准确表达“内存里到底隔了多少距离”的手段。若相关参数配置错误，计算必定由于位移偏差产生灾难性错误。

??? example "数值示例：带有 Padding 扩充的行主序矩阵"

    假设逻辑上我们需要处理或者提取一个 $2 \times 3$ 的行主序小矩阵：
    
    $$
    A =
    \begin{bmatrix}
    1 & 2 & 3 \\
    4 & 5 & 6
    \end{bmatrix}
    $$
    
    在理想与紧凑的情况下，它在物理内存中的排布为：`[1, 2, 3, 4, 5, 6]`，此时它的主方向跨度依然是 $3$。
    
    但是，考虑到对齐（如 `float4` 加载约束）或者防止 Bank Conflict 进行的 Padding 处理，其在物理内存层面每行实际按 $4$ 个元素存储：
    
    $$
    A_{physical} =
    \begin{bmatrix}
    1 & 2 & 3 & \text{pad} \\
    4 & 5 & 6 & \text{pad}
    \end{bmatrix}
    $$
    
    此状态下对应的内存形为：`[1, 2, 3, pad, 4, 5, 6, pad]`。
    
    此时该矩阵的逻辑列数（参与计算用到的维度参数 $N$ 或 $K$）仍是 $3$；但其行主序的实际跨度是 $4$。对于这段在内存真实排布的数据，传递 API 时的 `lda` 的真实含义应定为 $4$。 

#### 行列主序映射与转置

cuBLAS 遵循 Fortran 风格，默认将输入按列主序处理。但许多现代 C/C++ 与深度学习框架习惯行主序布局。处理时的经典手法是不在内存中直接执行转置（造成额外开销），而是借助矩阵恒等式 $(AB)^T = B^T A^T$ 完成视图重组。

假设有两个行主序矩阵求 $C_{2 \times 4} = A_{2 \times 3} \times B_{3 \times 4}$。
由于将这三块行主序内存交由 cuBLAS 后自动获得列主序视角的 $C_{4 \times 2}^T = B_{4 \times 3}^T \cdot A_{3 \times 2}^T$。因此真正的常见调用策略是在参数上优先传 $B$，再传 $A$，从而达成物理存储上等同的内存落盘效果，既保持了语义又规避了数据硬转置的延迟代价。

??? example "行主序的换位映射示意 (避免硬转置)"

    **目标逻辑运算**是求取行排布的 $C_{2 \times 4} = A_{2 \times 3} \times B_{3 \times 4}$：
    
    $$
    \begin{bmatrix}
    C_{00} & C_{01} & C_{02} & C_{03} \\
    C_{10} & C_{11} & C_{12} & C_{13}
    \end{bmatrix}_{2 \times 4}
    =
    \begin{bmatrix}
    A_{00} & A_{01} & A_{02} \\
    A_{10} & A_{11} & A_{12}
    \end{bmatrix}_{2 \times 3}
    \times
    \begin{bmatrix}
    B_{00} & B_{01} & B_{02} & B_{03} \\
    B_{10} & B_{11} & B_{12} & B_{13} \\
    B_{20} & B_{21} & B_{22} & B_{23}
    \end{bmatrix}_{3 \times 4}
    $$
    
    当默认接受**列主序**规则的 `cuBLAS` 收到底层连续的一维内存缓冲后，它实际上将其理解为了对它们进行了转置的排列！为了避免在 CPU 或 GPU 之前对上面的源数据执行低效且繁琐真实的内存拷贝调整，我们转而利用 $B^T_{4 \times 3} \cdot A^T_{3 \times 2}$ 即会获得 $C_{4 \times 2}^T$：
    
    $$
    \begin{bmatrix}
    C_{00} & C_{10} \\
    C_{01} & C_{11} \\
    C_{02} & C_{12} \\
    C_{03} & C_{13}
    \end{bmatrix}_{4 \times 2}
    =
    \begin{bmatrix}
    B_{00} & B_{10} & B_{20} \\
    B_{01} & B_{11} & B_{21} \\
    B_{02} & B_{12} & B_{22} \\
    B_{03} & B_{13} & B_{23}
    \end{bmatrix}_{4 \times 3}
    \times
    \begin{bmatrix}
    A_{00} & A_{10} \\
    A_{01} & A_{11} \\
    A_{02} & A_{12}
    \end{bmatrix}_{3 \times 2}
    $$
    
    我们可以观察等号左边它认为算出来且列状堆叠落盘的 $C^T_{4 \times 2}$。此时将其平铺读出来：`[C00, C01, C02, C03, C10, C11, C12, C13]`，这简直跟我们一开始所期待的以行主序铺排存储的目标结果矩阵 $C_{2 \times 4}$ 一模一样。
    通过在 API 中按顺序先传入 $B$ 指针，再传 $A$ 指针，在不挪动任何字节的基础上，即可完美取得正确结果。

#### Batched 执行与混合精度

对于深层网络，常见有大量的相同形状子矩阵运算（如多头注意力机制、独立的样本组）。连续多次调用 `cuBLASGemm` 将导致巨大的 Kernel Launch 开销。

- **`cublasGemmStridedBatched`** 接口允许提交一组 GEMM 任务。用 `strideA`/`strideB`/`strideC` 告知相邻子矩阵批次的距离，实现合并 Launch 提速。
- **`cublasGemmEx`** 提供了灵活的混合精度支持。允许输入选用 `FP16`/`BF16` 而内部累加寄存使用 `FP32` 数据流，兼顾了提升模型训练吞吐量、优化 Tensor Core 负载并防止深层算子精度溢出的各项工程需求。
## 参考资料

- [6 Step Optimization of GEMMs in CUDA](https://www.rimikawrites.com/6-step-optimization-of-gemms-in-cuda/){target=_blank}
- [CUDA C++ Programming Guide: WMMA](https://docs.nvidia.com/cuda/cuda-c-programming-guide/){target=_blank}
- [cuBLAS Documentation](https://docs.nvidia.com/cuda/cublas/){target=_blank}
