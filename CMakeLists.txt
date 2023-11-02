cmake_minimum_required(VERSION 3.4)

# ===================================================================
# PROJECT CONFIG
# ===================================================================
project(flexiv_aidk VERSION 1.2)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "CMake build type" FORCE)
endif()

# Set static library
message("OS: ${CMAKE_SYSTEM_NAME}")
message("Processor: ${CMAKE_SYSTEM_PROCESSOR}")
set(AIDK_STATIC_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/lib/libflexiv_aidk.a")

# ===================================================================
# PROJECT DEPENDENCIES
# ===================================================================
# pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ===================================================================
# PROJECT LIBRARIES
# ===================================================================
# Create an INTERFACE library with no source file to compile
add_library(${PROJECT_NAME} INTERFACE)

# Create an alias of the library using flexiv namespace, to imitate the install
# target which uses flexiv namespace.
add_library(flexiv::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)

target_link_libraries(${PROJECT_NAME} INTERFACE ${AIDK_STATIC_LIBRARY}
                                                Threads::Threads)

# Use moderate compiler warning option
if(CMAKE_HOST_UNIX)
  target_compile_options(${PROJECT_NAME} INTERFACE -Wall -Wextra)
else()
  target_compile_options(${PROJECT_NAME} INTERFACE /W1)
endif()

# Install the INTERFACE library
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FlexivInstallLibrary.cmake)
FlexivInstallLibrary()
