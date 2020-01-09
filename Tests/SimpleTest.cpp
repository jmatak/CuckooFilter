//
// Created by patrik on 08. 01. 2020..
//


#include "../ArgParser/cxxopts.hpp"
#include "../CF/CuckooFilter.h"
#include <chrono>


/**
 *
 * @param tableSize
 * @param n elements from 0 to n will be inserted
 */
void test1(size_t tableSize, size_t n) {
//    size_t total_items = tableSize;
    CuckooFilter<size_t, uint16_t> filter(tableSize, 16, 4);

    //  inserting items to the cuckoo filter
    size_t num_inserted = 0;
    for (size_t i = 0; i < n; i++, num_inserted++) {
         if (!filter.insertElement(i)) {
            break;
        }
    };

    // check if all inserted items are in filter, expected to be true for all
    for (size_t i = 0; i < num_inserted; i++) {
        assert(filter.containsElement(i));
    }

    size_t total_queries = 0;
    size_t false_queries = 0;
    for (size_t i = n; i < 2 * n; i++) {
        if (filter.containsElement(i)) {
            false_queries++;
        }
        total_queries++;
    }

//    filter.print();

    std::cout << "availability: "
              << filter.availability() * 100 << "%\n";

    std::cout << "false positive rate is "
              << 100.0 * false_queries / total_queries << "%\n";

}


int main(int argc, char **argv) {
    size_t tableSize = 1048576;
//    size_t tableSize = 1024;
    size_t elements = tableSize;

    int n = 10;
    double total_time = 0.;

    for (size_t i = 0; i < n; ++i) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        test1(tableSize, elements);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        total_time += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    }

    std::cout << "Avg time = " << total_time / n
              << "[µs]" << std::endl;

    return 0;
}