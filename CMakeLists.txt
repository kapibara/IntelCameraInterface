cmake_minimum_required(VERSION 2.8)

project(IntelCameraInterface)

set(HOME ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_INSTALL_PREFIX ${HOME}/bin)

#openCV
set(OpenCV_DIR "C:/openCV/custom_build/")
find_package(OpenCV REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)


add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Multimedia_DEFINITIONS})

include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Multimedia_INCLUDE_DIRS})

#Intel PCSDK
set(IntelSDK_DIR "C:/Program Files (x86)/Intel/PCSDK")
set(IntelSDK_INCLUDE "${IntelSDK_DIR}/include/")
set(IntelSDK_LIB_DIR "${IntelSDK_DIR}/lib/Win32/")
set(IntelSDK_LIBS libpxc )

include_directories(${HOME}/include)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${IntelSDK_INCLUDE})
include_directories("C:/Data/Development/CPP/KinectQtRecorder/include")

link_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${IntelSDK_LIB_DIR})

add_subdirectory(${HOME}/src)

