cmake_minimum_required(VERSION 3.18)
project(test_aidk VERSION 1.2)

set(CMAKE_CXX_STANDARD 17)

# Set cmake module find path
set(FVR_AI_3RDPARTY_INSTALL_DIR $ENV{flexiv_thirdparty_install_dir})

# Add flexiv_ai_3rdparty install folder to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH ${FVR_AI_3RDPARTY_INSTALL_DIR})

# Find thirdparties
find_package(flexiv_aidk REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

# Add the executable target
add_executable(test_aidk_compute test_aidk_compute.cpp)
add_executable(test_aidk_compute_image test_aidk_compute_image.cpp)
add_executable(test_aidk_others test_aidk_others.cpp)

# Link the static library and any other necessary libraries
target_link_libraries(test_aidk_compute PRIVATE flexiv::flexiv_aidk)
target_link_libraries(test_aidk_compute_image PRIVATE flexiv::flexiv_aidk
                                                      opencv_imgcodecs)
target_link_libraries(test_aidk_others PRIVATE flexiv::flexiv_aidk)
