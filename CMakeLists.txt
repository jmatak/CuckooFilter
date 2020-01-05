cmake_minimum_required(VERSION 3.15)
project(CuckooFilter)

set(CMAKE_CXX_STANDARD 17)

add_executable(CuckooFilter
        CF/BitManager.cpp
        CF/BitManager.h
        CF/CuckooFilter.cpp
        CF/CuckooFilter.h
        CF/CuckooTable.cpp
        CF/CuckooTable.h
        CF/HashFunction.cpp
        CF/HashFunction.h
        CF/main.cpp

        FASTA/FastaIterator.cpp
        FASTA/FastaIterator.h
        FASTA/FastaReader.cpp
        FASTA/FastaReader.h

        ArgParser/cxxopts.hpp
        )
