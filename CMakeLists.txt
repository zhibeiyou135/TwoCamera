cmake_minimum_required(VERSION 3.10)
project(TwoCamera LANGUAGES CXX)

# 强制使用系统 Qt 和库
list(APPEND CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt5")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 更新C++标准版本以兼容LibTorch 2.2.1
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译器标志以确保LibTorch兼容性
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# 设置Boost库的配置
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_USE_DEBUG_RUNTIME FALSE)



# 查找系统 Qt
find_package(Qt5 COMPONENTS Widgets Concurrent REQUIRED)

# 查找Boost库并指定版本，确保与Metavision使用相同版本
find_package(Boost 1.71 EXACT REQUIRED COMPONENTS system filesystem timer)

# 处理 QRC 文件
file(GLOB_RECURSE QRC_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.qrc)
qt5_add_resources(QRC_FILES ${QRC_SOURCE_FILES})

# 其他依赖
find_package(OpenCV 4.2 REQUIRED COMPONENTS core highgui)
find_package(MetavisionSDK COMPONENTS core driver ui REQUIRED)
find_package(HDF5 COMPONENTS CXX REQUIRED)
find_package(OpenMP REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMODBUS REQUIRED libmodbus)

# 头文件路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${OpenCV_INCLUDE_DIRS}
    ${LIBMODBUS_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    "/opt/MVS/include"  # 添加海康 SDK 头文件路径
)

# 添加链接目录
link_directories(
    ${Boost_LIBRARY_DIRS}
)

# 源文件集合
aux_source_directory(widgets/cameracontrols CAMERA_CONTROLS)
aux_source_directory(widgets/showresult SHOW_RESULT_SOURCES_ALL)

# 排除InferResultWidget
set(SHOW_RESULT_SOURCES "")
foreach(SRC ${SHOW_RESULT_SOURCES_ALL})
    if(NOT SRC MATCHES "InferResultWidget")
        list(APPEND SHOW_RESULT_SOURCES ${SRC})
    endif()
endforeach()

aux_source_directory(widgets MAIN_WIDGET_SOURCES)
aux_source_directory(socket SOCKET)

set(ALL_WIDGET_SOURCES ${CAMERA_CONTROLS} ${MAIN_WIDGET_SOURCES} ${SHOW_RESULT_SOURCES} ${SOCKET})

# 显式添加新增的PlaybackWidget文件
set(ADDITIONAL_SOURCES
    widgets/cameracontrols/PlaybackWidget.cpp
    widgets/cameracontrols/PlaybackWidget.h
)

# 可执行文件
add_executable(TwoCamera
    main.cpp
    mainwindow.cpp
    mainwindow.h
    camera/CameraCapture.cpp
    camera/CameraCapture.h
    camera/CamerasDetector.cpp
    camera/CropConfig.h
    camera/DVDisplayConfig.cpp
    camera/DVDisplayConfig.h
    camera/RecordingConfig.cpp
    camera/RecordingConfig.h
    dvs/DVSDataSource.cpp 
    dvs/DVSDataSource.h 
    dvs/EventStruct.h 
    dvs/DVSBiasConfig.h
    CountTimer.cpp CountTimer.h
    camera/CameraLogger.cpp camera/CameraLogger.h PlaybackReader.cpp PlaybackReader.h
    camera/FileSaveManager.cpp camera/FileSaveManager.h
    camera/DetectionSessionManager.cpp camera/DetectionSessionManager.h
    camera/CameraConfig.cpp camera/CameraConfig.h
    dvs/DvsDataSegment.h BlockingQueue.h 
    socket/modbussocket.cpp socket/modbussocket.h
    ${QRC_FILES}
    ${ALL_WIDGET_SOURCES}
    ${ADDITIONAL_SOURCES}
)

# 添加 infer 子项目并链接
add_subdirectory(infer)

# 链接库
target_link_libraries(TwoCamera PRIVATE
    Qt5::Widgets
    Qt5::Concurrent
    infer  # 直接链接子项目目标
    pthread
    usb-1.0
    MetavisionSDK::core
    MetavisionSDK::driver
    MetavisionSDK::ui
    HDF5::HDF5
    X11
    ${Boost_LIBRARIES}
    /opt/MVS/lib/64/libMvCameraControl.so
    ${OpenCV_LIBS}
    ${LIBMODBUS_LIBRARIES}
    /usr/lib/x86_64-linux-gnu/libffi.so.7
    /usr/lib/x86_64-linux-gnu/libstdc++.so.6
    -ltiff
    -lcairo
    -luuid
)