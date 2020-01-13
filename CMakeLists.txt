cmake_minimum_required(VERSION 3.15)
project(CuckooFilter)

set(CMAKE_CXX_STANDARD 17)

add_executable(CuckooFilter
        Utils/bit_manager.cpp
        Utils/bit_manager.h
        CF/cuckoo_filter.h
        CF/cuckoo_table.h

        #        DCF/BitManager.cpp
        #        DCF/BitManager.h
        #        DCF/CuckooFilter.cpp
        #        DCF/CuckooFilter.h
        #        DCF/CuckooTable.cpp
        #        DCF/CuckooTable.h
        #        DCF/DynamicCuckooFilter.cpp
        #        DCF/DynamicCuckooFilter.h
        #                        DCF/HashFunction.cpp
        #                        DCF/HashFunction.h

        FASTA/fasta_reader.h
        FASTA/fasta_reader.cpp
        FASTA/fasta_iterator.h
        FASTA/fasta_iterator.cpp


        Utils/hash_function.cpp
        Utils/hash_function.h
        Utils/city_hash.cpp
        Utils/murmur_hash3.cpp


        ArgParser/cxxopts.hpp
#                Tests/SimpleTest.cpp
        Tests/cf_fasta_test.cpp
#                Tests/hashfunTest.cc
        #        Tests/SimpleFastaTest.cpp
#        Tests/SimpleDynamicTest.cpp
        #        Tests/FastaDynamicTest.cc
        #        Tests/PrintKmersToFile.cc
        #        FASTA/FastaTest.cpp
        #        Tests/SimpleFastaTest2.cpp
        )
