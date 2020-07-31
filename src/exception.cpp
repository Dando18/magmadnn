#include "magmadnn/exception.h"

#if defined(MAGMADNN_HAVE_MKLDNN)
#include "dnnl.hpp"
#endif

namespace magmadnn {

#if defined(MAGMADNN_HAVE_MKLDNN)
std::string DnnlError::get_error(int64 error_code) {
#define MAGMADNN_REGISTER_DNNL_ERROR(error_name)        \
    if (error_code == static_cast<int64>(error_name)) { \
        return #error_name;                             \
    }
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_success);
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_out_of_memory);
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_invalid_arguments);
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_unimplemented);
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_iterator_ends);
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_runtime_error);
    MAGMADNN_REGISTER_DNNL_ERROR(dnnl_not_required);
    return "Unknown error";

#undef MAGMADNN_REGISTER_DNNL_ERROR
}
#endif

#if defined(MAGMADNN_HAVE_CUDA)
std::string CudaError::get_error(int64 error_code) {
    std::string name = cudaGetErrorName(static_cast<cudaError>(error_code));
    std::string message = cudaGetErrorString(static_cast<cudaError>(error_code));
    return name + ": " + message;
}
#endif

#if defined(MAGMADNN_HAVE_CUDA)
std::string CublasError::get_error(int64 error_code) {
#define MAGMADNN_REGISTER_CUBLAS_ERROR(error_name)      \
    if (error_code == static_cast<int64>(error_name)) { \
        return #error_name;                             \
    }
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_SUCCESS);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_NOT_INITIALIZED);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_ALLOC_FAILED);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_INVALID_VALUE);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_ARCH_MISMATCH);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_MAPPING_ERROR);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_EXECUTION_FAILED);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_INTERNAL_ERROR);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_NOT_SUPPORTED);
    MAGMADNN_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_LICENSE_ERROR);
    return "Unknown error";

#undef MAGMADNN_REGISTER_CUBLAS_ERROR
}
#endif

#if defined(MAGMADNN_HAVE_CUDA)
std::string CudnnError::get_error(int64 error_code) {
#define MAGMADNN_REGISTER_CUDNN_ERROR(error_name)       \
    if (error_code == static_cast<int64>(error_name)) { \
        return #error_name;                             \
    }
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_SUCCESS);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_NOT_INITIALIZED);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_ALLOC_FAILED);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_INVALID_VALUE);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_ARCH_MISMATCH);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_MAPPING_ERROR);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_EXECUTION_FAILED);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_INTERNAL_ERROR);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_NOT_SUPPORTED);
    MAGMADNN_REGISTER_CUDNN_ERROR(CUDNN_STATUS_LICENSE_ERROR);
    return "Unknown error";

#undef MAGMADNN_REGISTER_CUDNN_ERROR
}
#endif

}  // namespace magmadnn
