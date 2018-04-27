cmake_minimum_required(VERSION 3.7)
project(chip8)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories(include)

add_executable(
  test_chip8
  test/test_utils.cpp
  src/utils.cpp
  include/utils.hpp
  )
