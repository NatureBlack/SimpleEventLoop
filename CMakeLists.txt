cmake_minimum_required(VERSION 2.8)

project(EventLoop)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -pthread")

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
include_directories(
${INCLUDE_DIR}
)

add_subdirectory(src)

add_executable(testEventLoop test.cpp)
target_link_libraries(testEventLoop ${PROJECT_NAME})
