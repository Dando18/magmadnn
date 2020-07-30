/**
 * @file add.h
 * @author Daniel Nichols
 * @author Florent Lopez
 * @version 1.0
 * @date 2019-06-24
 *
 * @copyright Copyright (c) 2019
 */
#pragma once

#if defined(MAGMADNN_CMAKE_BUILD)
#include "magmadnn/config.h"
#endif
#include "tensor/tensor.h"

#if defined(MAGMADNN_HAVE_CUDA)
#include "cudnn.h"
#endif

namespace magmadnn {
namespace math {

template <typename T>
void add_in_place_cpu(T alpha, Tensor<T> *x, T beta, Tensor<T> *out);

template <typename T>
void add_in_place_cpu(T alpha, Tensor<T> *x, Tensor<T> *out);

template <typename T>
void add_in_place_cpu(Tensor<T> *x, Tensor<T> *out);

template <typename T>
void subtract_cpu(Tensor<T> *x, Tensor<T> *out);
   
template <typename T>
void add_in_place(T alpha, Tensor<T> *x, T beta, Tensor<T> *out);

template <typename T>
void add_in_place(T alpha, Tensor<T> *x, Tensor<T> *out);

#if defined(MAGMADNN_HAVE_CUDA)
template <typename T>
void add_in_place_device(cudnnHandle_t handle, T alpha, Tensor<T> *x, T beta, Tensor<T> *out);

template <typename T>
void add_in_place_device(cudnnHandle_t handle, Tensor<T> *x, Tensor<T> *out);
   
template <typename T>
void add_in_place_device(T alpha, Tensor<T> *x, T beta, Tensor<T> *out);

template <typename T>
void subtract_device(cudnnHandle_t handle, Tensor<T> *x, Tensor<T> *out);
#endif

}  // namespace math
}  // namespace magmadnn
