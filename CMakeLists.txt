cmake_minimum_required(VERSION 3.15)
project(CuckooFilter)

set(CMAKE_CXX_STANDARD 17)

add_executable(CuckooFilter
        DCF/BitManager.cpp
        DCF/BitManager.h
        DCF/CuckooFilter.cpp
        DCF/CuckooFilter.h
        DCF/CuckooTable.cpp
        DCF/CuckooTable.h
        DCF/HashFunction.cpp
        DCF/HashFunction.h
        DCF/CityHash.cpp
        DCF/DynamicCuckooFilter.cpp
        DCF/DynamicCuckooFilter.h

        FASTA/FastaReader.h
        FASTA/FastaReader.cpp
        FASTA/FastaIterator.h
        FASTA/FastaIterator.cpp
#        FASTA/FastaTest.cpp

#        Tests
#        Tests/SimpleTest.cpp
#        Tests/SimpleFastaTest.cpp
        Tests/SimpleDynamicTest.cpp

        ArgParser/cxxopts.hpp
        )
