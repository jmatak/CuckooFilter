cmake_minimum_required(VERSION 3.15)
project(CuckooFilter)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-O3")

add_executable(CuckooFilter
        Demo/cf_demo.cpp

        Utils/bit_manager.h
        Utils/bit_manager.cpp

        Utils/hash_function.h
        Utils/hash_function.cpp
        Utils/city_hash.cpp
        Utils/murmur_hash3.h
        Utils/murmur_hash3.cpp
        )

add_executable(DynamicCuckooFilter
        Demo/dcf_demo.cpp

        Utils/bit_manager.h
        Utils/bit_manager.cpp

        Utils/hash_function.h
        Utils/hash_function.cpp
        Utils/city_hash.cpp
        Utils/murmur_hash3.h
        Utils/murmur_hash3.cpp
        )
