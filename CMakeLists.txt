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
        CF/CityHash.cpp

        FASTA/FastaReader.h
        FASTA/FastaReader.cpp
        FASTA/FastaIterator.h
        FASTA/FastaIterator.cpp

        ArgParser/cxxopts.hpp
#        Tests/SimpleTest.cpp
        Tests/SimpleFastaTest.cpp
        )
