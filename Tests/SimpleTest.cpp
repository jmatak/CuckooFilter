//
// Created by patrik on 08. 01. 2020..
//


#include "../ArgParser/cxxopts.hpp"
#include "../CF/CuckooFilter.h"




int main(int argc, char **argv) {
    size_t total_items = 100000;
    CuckooFilter<size_t, uint16_t> filter(total_items, 12, 4);

    //  inserting items to the cuckoo filter
    size_t num_inserted = 0;
    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        if (!filter.insertElement(i)) {
            break;
        }
    };

    // check if all inserted items are in filter, expected to be true for all
    for (size_t i = 0; i < total_items; i++, num_inserted++) {
        assert(filter.containsElement(i));
    }

    size_t total_queries = 0;
    size_t false_queries = 0;
    for (size_t i = total_items; i < 2 * total_items; i++) {
        if (filter.containsElement(i)) {
            false_queries++;
        }
        total_queries++;
    }


    std::cout << "false positive rate is "
              << 100.0 * false_queries / total_queries << "%\n";


    return 0;
}