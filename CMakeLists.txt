cmake_minimum_required(VERSION 3.15)
project(CuckooTest)

set(CMAKE_CXX_STANDARD 14)

add_executable(CuckooTest CuckooTable.cpp CuckooTable.h Test.cpp CuckooFilter.cpp CuckooFilter.h CuckooHash.cpp CuckooHash.h)