cmake_minimum_required(VERSION 3.12)
project(vm)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(SEASTAR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/seastar)
#add_subdirectory(seastar)
#add_subdirectory(WAVM)
add_subdirectory(src)