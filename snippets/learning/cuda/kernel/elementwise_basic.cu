#include <cuda_runtime.h>

__device__ __forceinline__ float relu_op(float x) {
    return x > 0.0f ? x : 0.0f;
}

__device__ __forceinline__ float leaky_relu_op(float x, float alpha) {
    return x >= 0.0f ? x : alpha * x;
}

__device__ __forceinline__ float sigmoid_op(float x) {
    return 1.0f / (1.0f + __expf(-x));
}

__device__ __forceinline__ float silu_op(float x) {
    return x * sigmoid_op(x);
}

__global__ void vector_add_kernel(const float *a, const float *b, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = a[idx] + b[idx];
    }
}

__global__ void relu_kernel(const float *input, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = relu_op(input[idx]);
    }
}

__global__ void leaky_relu_kernel(const float *input, float *output, float alpha, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = leaky_relu_op(input[idx], alpha);
    }
}

__global__ void sigmoid_kernel(const float *input, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = sigmoid_op(input[idx]);
    }
}

__global__ void silu_kernel(const float *input, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = silu_op(input[idx]);
    }
}

__global__ void swiglu_kernel(const float *value, const float *gate, float *output, int N) {
    // 获得当前线程的全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // 边界检查确保安全
    if (idx < N) {
        output[idx] = value[idx] * sigmoid_op(gate[idx]);
    }
}

void launch_vector_add(const float *a, const float *b, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    vector_add_kernel<<<blocks, threads>>>(a, b, output, N);
}

void launch_relu(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    relu_kernel<<<blocks, threads>>>(input, output, N);
}

void launch_leaky_relu(const float *input, float *output, float alpha, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    leaky_relu_kernel<<<blocks, threads>>>(input, output, alpha, N);
}

void launch_sigmoid(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    sigmoid_kernel<<<blocks, threads>>>(input, output, N);
}

void launch_silu(const float *input, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    silu_kernel<<<blocks, threads>>>(input, output, N);
}

void launch_swiglu(const float *value, const float *gate, float *output, int N) {
    if (N <= 0) {
        return;
    }

    int threads = 256;
    // 根据元素总数和线程块大小向上取整分配网格大小
    int blocks = (N + threads - 1) / threads;
    swiglu_kernel<<<blocks, threads>>>(value, gate, output, N);
}
