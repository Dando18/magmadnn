/**
 * @file pow.cpp
 * @author Daniel Nichols
 * @version 0.1
 * @date 2019-06-10
 *
 * @copyright Copyright (c) 2019
 *
 */
#if defined(MAGMADNN_CMAKE_BUILD)
#include "magmadnn/config.h"
#endif
#include "math/pow.h"

namespace magmadnn {
namespace math {

template <typename T>
void pow_cpu(Tensor<T> *x, int power, Tensor<T> *out) {
    T *x_ptr = x->get_ptr();
    T *out_ptr = out->get_ptr();
    unsigned int size = out->get_size();

    for (unsigned int i = 0; i < size; i++) {
        /* TODO : support different precisions for pow */
        out_ptr[i] = std::pow((T) x_ptr[i], (T) power);
    }
}

template <>
void pow_cpu(Tensor<int> *x, int power, Tensor<int> *out) {
    int *x_ptr = x->get_ptr();
    int *out_ptr = out->get_ptr();
    unsigned int size = out->get_size();

    for (unsigned int i = 0; i < size; i++) {
        out_ptr[i] = (int) std::pow((float) x_ptr[i], (float) power);
    }
}
template void pow_cpu(Tensor<float> *x, int power, Tensor<float> *out);
template void pow_cpu(Tensor<double> *x, int power, Tensor<double> *out);

template <typename T>
void pow(Tensor<T> *x, int power, Tensor<T> *out) {
    if (out->get_memory_type() == HOST) {
        pow_cpu(x, power, out);
    }
#if defined(MAGMADNN_HAVE_CUDA)
    else {
        pow_device(x, power, out);
    }
#endif
}

// template <>
// void pow(Tensor<int> *x, int power, Tensor<int> *out) {
//     if (out->get_memory_type() == HOST) {
//         int *x_ptr = x->get_ptr();
//         int *out_ptr = out->get_ptr();
//         unsigned int size = out->get_size();

//         for (unsigned int i = 0; i < size; i++) {
//             out_ptr[i] = (int) std::pow((float) x_ptr[i], (float) power);
//         }
//     }
// #if defined(_HAS_CUDA_)
//     else {
//         pow_device(x, power, out);
//     }
// #endif
// }
template void pow(Tensor<int> *x, int power, Tensor<int> *out);
template void pow(Tensor<float> *x, int power, Tensor<float> *out);
template void pow(Tensor<double> *x, int power, Tensor<double> *out);

}  // namespace math
}  // namespace magmadnn
