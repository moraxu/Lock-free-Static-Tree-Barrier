cmake_minimum_required(VERSION 3.16)
project(StaticTreeBarrier)

set(CMAKE_CXX_STANDARD 11)

add_executable(StaticTreeBarrier test1.cpp STBarrier.h test2.cpp)