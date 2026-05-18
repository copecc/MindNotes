---
title: Normalization
tags:
  - CUDA
  - Kernel
  - Normalization
---

# Normalization

## 类型定位

Normalization（归一化）是深度学习中典型的复合算子（Operator Fusion），它通常包含一次 Reduction 后紧跟着一次 Elementwise 的广播放缩计算。包含典型的 `LayerNorm`, `BatchNorm`, `RMSNorm`。由于深度学习中间层的带宽瓶颈十分明显，将计算方差/均值与最后的放缩合并在一个 kernel 里能够节省大量访存。

## 优化方法

- 每个 Block 负责处理样本层级数据的一个独立子集（例如独立的一行 Feature）。
- Thread 首先分别执行自身负责元素的特征抽取，通过 Shared Memory 或者 Warp Shuffle 快速归约计算出均值和方差。
- `__syncthreads()` 同步后，共享求得的归一化系数（通常为方差倒数 `rsqrt`），各个线程再执行 elementwise 对应元素的修正覆盖。
- 使用 `float4` 等宽位存取以及 Grid-Stride Loop 可进一步提升内存访问效率。

## 题目索引

- `layer_norm`
- `rms_norm`
- `batch_norm_2d`

## layer_norm

`LayerNorm` 是一种广泛应用于 Transformer 等模型中的归一化技术，负责在特征或通道维度上将张量缩放至零均值与单位方差。通过在每个样本内独立计算，它有效减弱了由于序列长度变化造成的梯度不稳定。

计算均值与方差：

$$
\mu = \frac{1}{H}\sum_{i=0}^{H-1} x_i, \quad  \sigma^2 = \frac{1}{H}\sum_{i=0}^{H-1} (x_i - \mu)^2
$$

应用缩放与平移参数：

$$
y_i = \gamma_i \frac{x_i - \mu}{\sqrt{\sigma^2 + \epsilon}} + \beta_i
$$

???+ note "layer_norm 实现"

    === "Basic"

        ```cpp title="layer_norm_basic"
        --8<-- "learning/cuda/kernel/layer_norm_basic.cu"
        ```

## rms_norm

`RMSNorm` 简化了传统的 `LayerNorm` 取消了求均值及平移的操作，只计算均方根来进行数值放缩。它也是 LLaMA 系列大模型的标准组件。

首先计算均方根特征方差逆：

$$
V = \frac{1}{\sqrt{\frac{1}{N}\sum_{i=0}^{N-1} x_i^2 + \epsilon}}
$$

随后进行权重缩放：

$$
y_i = (x_i \cdot V) \cdot w_i
$$

???+ note "rms_norm 实现"

    === "Basic"

        ```cpp title="rms_norm_basic"
        --8<-- "learning/cuda/kernel/rms_norm_basic.cu"
        ```

## batch_norm_2d

`BatchNorm` 常用于计算机视觉卷积神经网络，沿着 Batch 及空间分辨率维度对每个单独的 Channel 提取全局分布特征。这会导致单个 Kernel 需要跨步读取图像内存（NCHW 布局），引发非合并访存，从而对线程配置及归约策略有独特要求。

均值与方差计算针对当前通道路由下的所有 $N \times H \times W$ 个样本。

$$
y_{n,c,h,w} = \gamma_c \frac{x_{n,c,h,w} - \mu_c}{\sqrt{\sigma_c^2 + \epsilon}} + \beta_c
$$

???+ note "batch_norm_2d 实现"

    === "Basic"

        ```cpp title="batch_norm_2d_basic"
        --8<-- "learning/cuda/kernel/batch_norm_2d_basic.cu"
        ```

## 边界与局限

如果网络处理的隐藏层维度 $N$ 能够放进一个单 Block 限制 (一般 $N \le 1024$) 效率较高；当特征过大需要跨 Block 处理一行时，则会引入二次归约同步与额外的访存开销。
