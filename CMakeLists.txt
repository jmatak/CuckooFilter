cmake_minimum_required(VERSION 3.15)
project(CuckooFilter)

set(CMAKE_CXX_STANDARD 17)

add_executable(CuckooFilter
        #        Utils/BitManager.cpp
        #        Utils/BitManager.h
        #        CF/CuckooFilter.h
        #        CF/CuckooTable.h

        DCF/BitManager.cpp
        DCF/BitManager.h
        DCF/CuckooFilter.cpp
        DCF/CuckooFilter.h
        DCF/CuckooTable.cpp
        DCF/CuckooTable.h
        DCF/DynamicCuckooFilter.cpp
        DCF/DynamicCuckooFilter.h
#                DCF/HashFunction.cpp
#                DCF/HashFunction.h

        FASTA/FastaReader.h
        FASTA/FastaReader.cpp
        FASTA/FastaIterator.h
        FASTA/FastaIterator.cpp


        Utils/HashFunction.cpp
        Utils/HashFunction.h
        Utils/CityHash.cpp


        ArgParser/cxxopts.hpp
        #        Tests/SimpleTest.cpp
        #        Tests/SimpleFastaTest.cpp
        Tests/SimpleDynamicTest.cpp
        #        Tests/FastaDynamicTest.cc
        #        Tests/PrintKmersToFile.cc
        #        FASTA/FastaTest.cpp
        #        Tests/SimpleFastaTest2.cpp
        )
