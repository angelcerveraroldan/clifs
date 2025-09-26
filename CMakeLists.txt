cmake_minimum_required(VERSION 3.20)
project(clifs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)

pkg_check_modules(FUSE3 REQUIRED fuse3)

# Strict warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

add_executable(clifs src/main.cpp)

target_include_directories(clifs PRIVATE ${FUSE3_INCLUDE_DIRS})
target_link_libraries(clifs PRIVATE ${FUSE3_LINK_LIBRARIES})
target_compile_options(clifs PRIVATE ${FUSE3_CFLAGS_OTHER})
