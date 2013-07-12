cmake_minimum_required(VERSION 2.8)

project(make-refmovie CXX)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O -std=c++11 -stdlib=libc++")

add_executable(make-refmovie make-refmovie.cpp)
