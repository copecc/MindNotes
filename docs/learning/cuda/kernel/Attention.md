---
title: Attention
tags:
  - CUDA
  - Kernel
  - Attention
---

# Attention

## 类型定位

大语言模型（LLM）中的特化算子集合，它主要针对 `Q, K, V` 的生成、注意力交互以及一系列衍生模块（特化投影、相对位置修正编码 RoPE）。这类算子的优化重点是融合计算与访存边界，降低内存带宽占用。

## 优化方法

- RoPE 等映射类通常可退化成 Elementwise 核进行高效注入。
- 对于 `Softmax_Attention` 这类原生 Attention 通常涉及到先发起 Gemm（Q/K 结合），然后使用 Reduction（Softmax 中找 Max，求经验分布和），以及再发起第二次 Gemm（与 V 结合）。核心优化手段侧重于将其保留在 `Shared Memory` 或寄存器中分块流式计算。

## 题目索引

- `rope_embedding`
- `fused_softmax_attention`
- `causal_mask_fusion`
- `grouped_query_attention`
- `paged_kv_cache_attention`
- `swiglu_mlp_block`

## rope_embedding

`RoPE`（旋转位置编码）针对输入的特征张量分别两两对相邻元素形成 $2D$ 复数域表示，根据当前的上下文位置 $pos$ 转动对应的角度。

$$
\begin{aligned}
\tilde{q}_0 &= q_0 \cos \theta - q_1 \sin \theta \\
\tilde{q}_1 &= q_1 \cos \theta + q_0 \sin \theta
\end{aligned}
$$

使用 Kernel 将其 Fusion，可以在生成 QK 查询结果时就以最细粒度的寄存器访问立刻覆盖。

???+ note "rope_embedding 实现"

    === "Basic"

        ```cpp title="rope_embedding_basic"
        --8<-- "learning/cuda/kernel/rope_embedding_basic.cu"
        ```

## fused_softmax_attention

融合版的简易 Scaled Dot-Product Attention，将 $Q \cdot K^T$、局部最大值与指数和的推演以及结合 $V$ 的操作合并在一个 Kernel 内。这是 FlashAttention 等分块并行方法的最初核心思想，显著减少了中间 $N \times N$ 分数矩阵访存写回全局显存的开销限制。

**Block 级修正数学推导：**

FlashAttention 的核心不仅是借用了 Online Softmax，更是将其升级成了 **块级（Block-wise）** 操作与 $V$ 的乘阶推演。设上一个块的状态为 $m^{(old)}, l^{(old)}, O^{(old)}$，处理新载入的块得到的特征是 $m^{(block)}, l^{(block)}, V^{(block)}$ 以及局部权重 $P^{(block)}$。由于全局最大值发生变化：

$$
m^{(new)} = \max(m^{(old)}, m^{(block)})
$$

新旧部分的概率衰积必须同时对标最新的 $m^{(new)}$：

$$
l^{(new)} = e^{m^{(old)} - m^{(new)}} l^{(old)} + e^{m^{(block)} - m^{(new)}} l^{(block)}
$$

而在 Attention 中，之前的输出 $O^{(old)}$ 是以旧分母和旧最大值归一化的，需要将其解缩放并套入新的全局量中，结合新块的贡献得出：

$$
O^{(new)} = \frac{l^{(old)} e^{m^{(old)} - m^{(new)}} O^{(old)} + e^{P^{(block)} - m^{(new)}} V^{(block)}}{l^{(new)}}
$$

这种依靠块与块之间严格代数等价递推的方式，允许数据分片流式进入 Shared Memory。

???+ note "fused_softmax_attention 实现" 

    === "Basic"

        ```cpp title="fused_softmax_attention_basic"
        --8<-- "learning/cuda/kernel/fused_softmax_attention_basic.cu"
        ```

## causal_mask_fusion

Decoder-only 模型在 Attention 预测时往往具有因果掩码遮蔽（下三角）：不能看到未来的特征映射。此 Kernel 把常在 PyTorch 中额外的 `masked_fill` 操作优化掉，由计算 $Q \cdot K$ 二维线程网格根据块号位置动态判决，如果超越因果视野界限，对超越因果视野的元素赋予极小负数。

???+ note "causal_mask_fusion 实现"

    === "Basic"

        ```cpp title="causal_mask_fusion_basic"
        --8<-- "learning/cuda/kernel/causal_mask_fusion_basic.cu"
        ```

## grouped_query_attention

随着生成式模型的变大，缓存历史 Key 与 Value 会产生极大的显存开销，多级共享查询注意力 (GQA) 取代原先等长对应的 Multi-Head Attention。为避免复制与广播 (Broadcast) K/V，在 CUDA 中通过寻址跨步复用读取，让一个 $K$ 特征并行被同一组的数个 $Q$ 线程获取并计算。

???+ note "grouped_query_attention 实现"

    === "Basic"

        ```cpp title="grouped_query_attention_basic"
        --8<-- "learning/cuda/kernel/grouped_query_attention_basic.cu"
        ```

## paged_kv_cache_attention

面向高并发在线推理场景的寻址算子，效仿操作系统的 Page Table，把逻辑上的 KV Cache 拆成定长的离散块（Paged KV Cache）。该算子揭示了当前推理系统如 vLLM 中解码时如何通过传入页表 `block_table` 完成跨物理不连续显存地址块的高效拼接。

???+ note "paged_kv_cache_attention 实现"

    === "Basic"

        ```cpp title="paged_kv_cache_attention_basic"
        --8<-- "learning/cuda/kernel/paged_kv_cache_attention_basic.cu"
        ```

## swiglu_mlp_block

LLaMa 等流行模型中的 FFN 被改为门控线性组合。它利用一层输出特征过一个 SiLU (Swish 线性变换)，与第二分支结合，构成 SwiGLU。在此操作中执行 Kernel 替换能够节省掉三次临时浮点指针开销与跨步。

$$
Y =  \text{SiLU}(X \cdot W_{gate}) \otimes (X \cdot W_{up})
$$

???+ note "swiglu_mlp_block 实现"

    === "Basic"

        ```cpp title="swiglu_mlp_block_basic"
        --8<-- "learning/cuda/kernel/swiglu_mlp_block_basic.cu"
        ```

## 边界与局限

自建的大型 Attention Kernel 如果只是拼接了 API，受限于 HBM 带宽读写往往表现很差，因此对于工业界应用，基本全面应用了充分考虑片上内存缓存管理的融合型库（FlashAttention 或 TensorRT 等）。
