cmake_minimum_required(VERSION 3.15)
project(CuckooFilter)

set(CMAKE_CXX_STANDARD 14)

add_executable(CuckooFilter FASTA/FastaTest.cpp FASTA/FastaIterator.cpp FASTA/FastaReader.cpp FASTA/FastaReader.h FASTA/FastaIterator.h)