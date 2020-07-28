/**
 * @file memorymanager.cpp
 * @author Daniel Nichols
 * @version 0.1
 * @date 2019-02-07
 *
 * @copyright Copyright (c) 2019
 */
#include "memory/memorymanager.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>

#if defined(MAGMADNN_CMAKE_BUILD)
#include "magmadnn/config.h"
#endif
#include "magmadnn/utilities_internal.h"
#if defined(MAGMADNN_HAVE_CUDA)
#include "memory/memory_internal_device.h"
#endif

namespace magmadnn {

template <typename T>
MemoryManager<T>::MemoryManager(unsigned int size, memory_t mem_type, device_t device_id)
    : mem_type(mem_type), size(size) {
    set_device(device_id);

    // initialize based on the chosen memory type
    switch (mem_type) {
        case HOST:
            init_host();
            break;
#if defined(MAGMADNN_HAVE_CUDA)
        case DEVICE:
            init_device();
            break;
        case MANAGED:
            init_managed();
            break;
        case CUDA_MANAGED:
            init_cuda_managed();
            break;
#endif
        default:
            fprintf(stderr, "Invalid memory type.\n");
    }
}

template <typename T>
void MemoryManager<T>::init_host() {
   // TODO: replace use of `malloc` with `new`
   host_ptr = (T*) std::malloc(size * sizeof(T));
}

#if defined(MAGMADNN_HAVE_CUDA)
template <typename T>
void MemoryManager<T>::init_device() {
   this->set_custream(nullptr);
   cudaErrchk(cudaMalloc((void**) &device_ptr, size * sizeof(T)));
}

template <typename T>
void MemoryManager<T>::init_managed() {
   this->set_custream(nullptr);
   // TODO: replace use of `malloc` with `new`
   host_ptr = (T*) std::malloc(size * sizeof(T));
   cudaErrchk(cudaMalloc((void**) &device_ptr, size * sizeof(T)));
}

template <typename T>
void MemoryManager<T>::init_cuda_managed() {
   this->set_custream(nullptr);
   cudaErrchk(cudaMallocManaged((void**) &cuda_managed_ptr, size * sizeof(T)));
}
#endif

template <typename T>
MemoryManager<T>::MemoryManager(const MemoryManager& that)
    : mem_type(that.mem_type), device_id(that.device_id), size(that.size) {
    this->copy_from(that);
}

template <typename T>
MemoryManager<T>& MemoryManager<T>::operator=(const MemoryManager<T>& that) {
    this->mem_type = that.mem_type;
    this->device_id = that.device_id;
    this->size = that.size;

    this->copy_from(that);

    return *this;
}

template <typename T>
MemoryManager<T>::~MemoryManager<T>() {
    switch (mem_type) {
        case HOST:
           // TODO: replace use of `free` with `delete`
           std::free(host_ptr);
            break;
#if defined(MAGMADNN_HAVE_CUDA)
        case DEVICE:
            cudaErrchk(cudaFree(device_ptr));
            break;
        case MANAGED:
           // TODO: replace use of `free` with `delete`
           std::free(host_ptr);
            break;
            cudaErrchk(cudaFree(device_ptr));
            break;
        case CUDA_MANAGED:
            cudaErrchk(cudaFree(cuda_managed_ptr));
            break;
#endif
    }
}

template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from(const MemoryManager<T>& src, unsigned int begin_idx,
                                             unsigned int copy_size) {
    assert(this->size >= copy_size);
    assert(src.size >= (begin_idx + copy_size));

    if (copy_size == 0) return (magmadnn_error_t) 0;

    if (src.mem_type == HOST) {
        return copy_from_host(src.host_ptr, begin_idx, copy_size);
    }
#if defined(MAGMADNN_HAVE_CUDA)
    else if (src.mem_type == DEVICE) {
        return copy_from_device(src.device_ptr, begin_idx, copy_size);
    } else if (src.mem_type == MANAGED) {
        return copy_from_managed(src.host_ptr, src.device_ptr, begin_idx, copy_size);
    } else if (src.mem_type == CUDA_MANAGED) {
        return copy_from_cudamanaged(src.cuda_managed_ptr, begin_idx, copy_size);
    }
#endif

    return (magmadnn_error_t) 1;
}

template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from(const MemoryManager<T>& src) {
    return copy_from(src, 0, size);
}

template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from(const MemoryManager<T>& src, unsigned int copy_size) {
    return copy_from(src, 0, copy_size);
}

template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from_host(T* src, unsigned int begin_idx, unsigned int copy_size) {
    switch (mem_type) {
        case HOST:
            // host --> host
            std::copy(src + begin_idx, (src + begin_idx) + copy_size, host_ptr);
            return (magmadnn_error_t) 0;
#if defined(MAGMADNN_HAVE_CUDA)
        case DEVICE:
            // host --> device
            cudaErrchk(
                  cudaMemcpyAsync(
                        device_ptr, src + begin_idx, copy_size * sizeof(T),
                        cudaMemcpyHostToDevice, this->get_custream()));
            cudaStreamSynchronize(this->get_custream());
            return (magmadnn_error_t) 0;
        case MANAGED:
            // host --> managed
            std::copy(src + begin_idx, (src + begin_idx) + copy_size, host_ptr);
            sync(false);
            return (magmadnn_error_t) 0;
        case CUDA_MANAGED:
            // host --> cmanaged
            std::copy(src + begin_idx, (src + begin_idx) + copy_size, cuda_managed_ptr);
            sync(false);
            return (magmadnn_error_t) 0;
#endif
    }

    return (magmadnn_error_t) 1;
}

#if defined(MAGMADNN_HAVE_CUDA)
template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from_device(T* src, unsigned int begin_idx, unsigned int copy_size) {
    magmadnn_error_t err = (magmadnn_error_t) 0;

    switch (mem_type) {
        case HOST:
            // device --> host
           cudaErrchk(
                 cudaMemcpyAsync(host_ptr, src + begin_idx, copy_size * sizeof(T), cudaMemcpyDeviceToHost, this->get_custream()));
            break;
        case DEVICE:
            // device --> device
            cudaErrchk(
                  cudaMemcpyAsync(device_ptr, src + begin_idx, copy_size * sizeof(T), cudaMemcpyDeviceToDevice, this->get_custream()));
            break;
        case MANAGED:
            // device --> managed
            cudaErrchk(
                  cudaMemcpyAsync(device_ptr, src + begin_idx, copy_size * sizeof(T), cudaMemcpyDeviceToDevice, this->get_custream()));
            sync(true);
            return err;
        case CUDA_MANAGED:
            // device --> cmanaged
            cudaErrchk(
                  cudaMemcpyAsync(cuda_managed_ptr, src + begin_idx, copy_size * sizeof(T), cudaMemcpyDeviceToDevice, this->get_custream()));
            sync(true);
            return err;
    }

    // TODO Make routine optionally asynchronous
    cudaStreamSynchronize(this->get_custream());

    return err;
}

template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from_managed(T* host_src, T* device_src, unsigned int begin_idx,
                                                     unsigned int copy_size) {
    switch (mem_type) {
        case HOST:
            // managed --> host
            std::copy(host_src + begin_idx, (host_src + begin_idx) + copy_size, host_ptr);
            return (magmadnn_error_t) 0;
        case DEVICE:
            // managed --> device
            cudaErrchk(
                  cudaMemcpyAsync(
                        device_ptr, device_src + begin_idx, copy_size * sizeof(T),
                        cudaMemcpyDeviceToDevice, this->get_custream()));
            cudaStreamSynchronize(this->get_custream());
            return (magmadnn_error_t) 0;
        case MANAGED:
            // managed --> managed
            std::copy(host_src + begin_idx, (host_src + begin_idx) + copy_size, host_ptr);
            sync(false);
            return (magmadnn_error_t) 0;
        case CUDA_MANAGED:
            // managed --> cmanaged
            std::copy(host_src + begin_idx, (host_src + begin_idx) + copy_size, cuda_managed_ptr);
            sync(false);
            return (magmadnn_error_t) 0;
    }

    return (magmadnn_error_t) 1;
}

template <typename T>
magmadnn_error_t MemoryManager<T>::copy_from_cudamanaged(T* src, unsigned int begin_idx, unsigned int copy_size) {
    switch (mem_type) {
        case HOST:
            // cmanaged --> host
            std::copy(src + begin_idx, (src + begin_idx) + copy_size, host_ptr);
            return (magmadnn_error_t) 0;
        case DEVICE:
            // cmanaged --> device
            cudaErrchk(
                  cudaMemcpyAsync(
                        device_ptr, src + begin_idx, copy_size * sizeof(T),
                        cudaMemcpyDeviceToDevice, this->get_custream()));
            cudaStreamSynchronize(this->get_custream());
            return (magmadnn_error_t) 0;
        case MANAGED:
            // cmanaged --> managed
            std::copy(src + begin_idx, (src + begin_idx) + copy_size, host_ptr);
            sync(false);
            return (magmadnn_error_t) 0;
        case CUDA_MANAGED:
            std::copy(src + begin_idx, (src + begin_idx) + copy_size, cuda_managed_ptr);
            sync(false);
            return (magmadnn_error_t) 0;
    }

    return (magmadnn_error_t) 1;
}
#endif

template <typename T>
magmadnn_error_t MemoryManager<T>::sync(bool gpu_was_modified) {
#if defined(MAGMADNN_HAVE_CUDA)
    cudaError_t err = (cudaError_t) 0;

    if (mem_type == CUDA_MANAGED) {
        // cudaErrchk(cudaDeviceSynchronize());
        cudaErrchk(
              cudaStreamSynchronize(this->get_custream()));
    } else if (mem_type == MANAGED) {
        if (gpu_was_modified) {
            cudaErrchk(
                  cudaMemcpyAsync(
                        host_ptr, device_ptr, size * sizeof(T),
                        cudaMemcpyDeviceToHost, this->get_custream()));
            
        } else {
            cudaErrchk(
                  cudaMemcpyAsync(
                        device_ptr, host_ptr, size * sizeof(T),
                        cudaMemcpyHostToDevice, this->get_custream()));
        }
        cudaStreamSynchronize(this->get_custream());
    }
    return (magmadnn_error_t) err;
#else

    // nothing to sync, no error
    return (magmadnn_error_t) 0;

#endif
}

template <typename T>
T MemoryManager<T>::get(unsigned int idx) const {
    assert(idx < size);

    T res;
    
    switch (mem_type) {
        case HOST:
            return host_ptr[idx];
#if defined(MAGMADNN_HAVE_CUDA)
        case DEVICE:
            // cudaErrchk( cudaSetDevice(device_id) );   /* TODO -- fix MagmaDNN's device selection system */
            // return internal::get_device_array_element(device_ptr, idx);
           cudaMemcpyAsync(
                 &res, &device_ptr[idx], sizeof(T),
                 cudaMemcpyDeviceToHost, this->get_custream());
           cudaStreamSynchronize(this->get_custream());
           return res;
        case MANAGED:
            return host_ptr[idx];
        case CUDA_MANAGED:
            return cuda_managed_ptr[idx];
#endif
    }
    return (T) 0;
}

template <typename T>
void MemoryManager<T>::set(unsigned int idx, T val) {
    assert(idx < size);

    // note: don't sync on managed type memories
    switch (mem_type) {
        case HOST:
            host_ptr[idx] = val;
            break;
#if defined(MAGMADNN_HAVE_CUDA)
        case DEVICE:
            // cudaErrchk( cudaSetDevice(device_id) );   /* TODO -- fix MagmaDNN's device selection system */
            internal::set_device_array_element(device_ptr, idx, val);
            break;
        case MANAGED:
            host_ptr[idx] = val;
            // cudaErrchk( cudaSetDevice(device_id) );   /* TODO -- fix MagmaDNN's device selection system */
            internal::set_device_array_element(device_ptr, idx, val);
            break;
        case CUDA_MANAGED:
            cuda_managed_ptr[idx] = val;
            break;
#endif
    }
}

template <typename T>
magmadnn_error_t MemoryManager<T>::set_device(device_t device_id) {
#if defined(MAGMADNN_HAVE_CUDA)
    int n_devices = 0;
    cudaErrchk(cudaGetDeviceCount(&n_devices));
    if ((int) device_id >= n_devices) {
        fprintf(stderr, "invalid device id\n");
        return (magmadnn_error_t) 1;
    }
#endif

    this->device_id = device_id;
    return (magmadnn_error_t) 0;
}

template <typename T>
T* MemoryManager<T>::get_host_ptr() {
    return host_ptr;
}

#if defined(MAGMADNN_HAVE_CUDA)
template <typename T>
T* MemoryManager<T>::get_device_ptr() {
    return device_ptr;
}

template <typename T>
T* MemoryManager<T>::get_cuda_managed_ptr() {
    return cuda_managed_ptr;
}
#endif

template <typename T>
T* MemoryManager<T>::get_ptr() {
    switch (mem_type) {
        case HOST:
            return get_host_ptr();
#if defined(MAGMADNN_HAVE_CUDA)
        case DEVICE:
            return get_device_ptr();
        case MANAGED:
            // returns device by default for managed
            return get_device_ptr();
        case CUDA_MANAGED:
            return get_cuda_managed_ptr();
#endif
        default:
            return (T*) NULL;
    }
}

template <typename T>
magmadnn_error_t MemoryManager<T>::zero() {

   std::size_t sz = size * sizeof(T);

   switch (mem_type) {
    case HOST:
       std::memset(this->host_ptr, 0, sz);
       return (magmadnn_error_t) 0;
#if defined(MAGMADNN_HAVE_CUDA)
    case DEVICE:
       cudaMemsetAsync(this->device_ptr, 0, sz, this->custream_);
       return (magmadnn_error_t) 0;           
    case MANAGED:
       std::memset(this->host_ptr, 0, sz);
       cudaMemsetAsync(this->device_ptr, 0, sz, this->custream_);       
       cudaStreamSynchronize(this->custream_);
       return (magmadnn_error_t) 0;
    case CUDA_MANAGED:
       std::memset(this->host_ptr, 0, sz);
       cudaMemsetAsync(this->device_ptr, 0, sz, this->custream_);
       cudaStreamSynchronize(this->custream_);
       return (magmadnn_error_t) 0;
#endif
    }

    return (magmadnn_error_t) 1;

}

/* COMPILE FOR INT, FLOAT, AND DOUBLE */
template class MemoryManager<int>;
template class MemoryManager<float>;
template class MemoryManager<double>;

}  // namespace magmadnn
