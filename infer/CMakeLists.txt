cmake_minimum_required(VERSION 3.10)
project(infer LANGUAGES CXX)

# 更新C++标准版本以兼容LibTorch 2.2.1
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 检查编译器版本
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0")
        message(FATAL_ERROR "GCC version must be at least 7.0 for LibTorch 2.2.1")
    endif()
endif()

# 添加编译器标志以确保LibTorch兼容性
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")

# 添加额外的编译器定义来解决c10::optional问题
add_definitions(-DTORCH_API_INCLUDE_EXTENSION_H)
add_definitions(-DC10_USE_GLOG)
add_definitions(-DC10_DISABLE_LEGACY_IMPORT)

# 尝试解决optional问题的额外定义
add_definitions(-DUSE_C10_DISPATCHER=FULL_DISPATCHER)
add_definitions(-DTORCH_EXTENSION_NAME=infer)

set(CUDA_DIR "/usr/local/cuda-11.8")
set(NVINFER_INCLUDE_DIR "/home/pe/tensorrt/TensorRT-8.4.1.5.Linux.x86_64-gnu.cuda-11.6.cudnn8.4/TensorRT-8.4.1.5/include")
set(NVINFER_LIB_DIR "/home/pe/tensorrt/TensorRT-8.4.1.5.Linux.x86_64-gnu.cuda-11.6.cudnn8.4/TensorRT-8.4.1.5/targets/x86_64-linux-gnu/lib")
set(TORCHGPU_ENABLE True)

# 设置LibTorch路径
set(CMAKE_PREFIX_PATH "/home/pe/libtorch/libtorch-cxx11-abi-shared-with-deps-2.2.1+cu118/libtorch")

# 查找Qt包
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Widgets REQUIRED)

# 查找OpenCV
find_package(OpenCV 4.2 REQUIRED)

# 包含目录
include_directories(./)
include_directories(detect)
include_directories(${Qt5Concurrent_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

# 源文件
aux_source_directory(./ INFER_SOURCES)
aux_source_directory(./detect DETECT_SOURCES)

# CUDA和TensorRT库配置
if (DEFINED CUDA_DIR)
    if (DEFINED STATIC_INFER)
        set(INFER_LIB ${NVINFER_LIB_DIR}/libnvinfer_static.a ${NVINFER_LIB_DIR}/libnvonnxparser_static.a
                ${NVINFER_LIB_DIR}/libmyelin_compiler_static.a
                ${NVINFER_LIB_DIR}/libmyelin_executor_static.a
                ${NVINFER_LIB_DIR}/libmyelin_pattern_library_static.a
                ${NVINFER_LIB_DIR}/libmyelin_pattern_runtime_static.a
                ${NVINFER_LIB_DIR}/libonnx_proto.a
                ${CUDA_DIR}/lib/libcublas_static.a 
                ${CUDA_DIR}/lib/libcublasLt_static.a
                ${CUDA_DIR}/lib/libcudart_static.a 
                ${CUDA_DIR}/lib/libcudadevrt.a
                ${CUDA_DIR}/lib/libnvrtc.so
                ${CUDA_DIR}/lib/libculibos.a
                ${CUDA_DIR}/lib/libcusolver_static.a
                ${CUDA_DIR}/lib/libcufft_static.a
                ${CUDA_DIR}/lib/libnpps_static.a
                ${CUDA_DIR}/lib/libcufftw_static.a
                ${CUDA_DIR}/lib/libcusolver_static.a
                ${CUDA_DIR}/lib/libcusparse_static.a
                ${CUDA_DIR}/lib/libmetis_static.a
                ${CUDA_DIR}/lib/libnppc_static.a
                ${CUDNN_LIB_DIR}/libcudnn_ops_infer.so
                ${CUDNN_LIB_DIR}/libcudnn_static.a
                ${CUDA_LIBRARIES})
    else ()
        set(INFER_LIB ${NVINFER_LIB_DIR}/libnvinfer.so ${NVINFER_LIB_DIR}/libnvonnxparser.so
                ${CUDA_LIBRARIES})
    endif ()
elseif (DEFINED TFLITE)
    set(INFER_LIB ${TFLITE}/tensorflow/lite/tools/make/gen/linux_x86_64/lib/libtensorflow-lite.a)
else ()
    set(INFER_LIB)
endif ()

# LibTorch配置
if (DEFINED TORCHGPU_ENABLE)
    find_package(PythonInterp REQUIRED)
    
    # 尝试更兼容的LibTorch查找方式
    set(Torch_DIR "${CMAKE_PREFIX_PATH}/share/cmake/Torch")
    find_package(Torch REQUIRED)
    
    # 确保使用正确的编译标志
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
    
    # 包含LibTorch头文件
    include_directories(${TORCH_INCLUDE_DIRS})
    
    # 添加LibTorch库
    set(INFER_LIB ${INFER_LIB} ${TORCH_LIBRARIES})
    
    # 添加必要的定义
    add_definitions(-DTORCH_ENABLE)
    add_definitions(-DTORCH_API_INCLUDE_EXTENSION_H)
    
    message(STATUS "LibTorch found: ${TORCH_LIBRARIES}")
    message(STATUS "LibTorch include dirs: ${TORCH_INCLUDE_DIRS}")
endif()

# 创建库
add_library(infer ${INFER_SOURCES} ${DETECT_SOURCES})

# 设置目标属性
set_target_properties(infer PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# 链接库
target_link_libraries(infer PRIVATE 
    Qt5::Core 
    Qt5::Widgets
    Qt5::Concurrent
    ${OpenCV_LIBRARIES} 
    ${INFER_LIB}
)

# 添加编译器特定的目标属性
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(infer PRIVATE 
        -Wno-sign-compare
        -Wno-unused-parameter
        -Wno-deprecated-declarations
    )
endif()
