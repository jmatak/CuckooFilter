#include "../ArgParser/cxxopts.hpp"
#include "../CF/CuckooFilter.h"
#include <chrono>
#include <iostream>
#include <fstream>


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
int insertIntsInRange(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    assert(from < to);

    size_t numInserted = 0;
    for (size_t i = from; i < to; i++, numInserted++) {
        if (!filter->insertElement(i)) {
            break;
        }
    }
    return numInserted;
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void containsIntsInRange(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    for (size_t i = from; i < to; i++) {
        assert(filter->containsElement(i));
    }
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
float getFPRate(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    size_t total_queries = 0;
    size_t false_queries = 0;
    for (size_t i = from; i < to; i++) {
        if (filter->containsElement(i)) {
            false_queries++;
        }
        total_queries++;
    }
    return 100.0 * false_queries / total_queries;
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void deleteAllInRange(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    for (size_t i = from; i < to; i++) {
        filter->deleteElement(i);
    }
}


int main(int argc, char **argv) {




        size_t tableSize = 100000;
//    size_t tableSize = 32768;

    //Elements inserted in the filter are from 0 to numOfElements
    size_t numOfElements = tableSize;
//    size_t numOfElements = 32768;

    int n = 10;

    static const size_t fs = 16;

    double totalTime = 0.;
    double insTotalTime = 0.;
    double contTotalTime = 0.;
    double delTotalTime = 0.;

    size_t from = 0;
    size_t to = numOfElements;

    size_t numInserted;
    float numInsertedTot = 0;

    float fpRate;
    float fpRateTot = 0;

    double availabilityTot = 0.;

    for (size_t i = 0; i < n; ++i) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        CuckooFilter<size_t, 4, fs, uint16_t> filter(tableSize);

        std::chrono::steady_clock::time_point insBegin = std::chrono::steady_clock::now();
        numInserted = insertIntsInRange(&filter, from, to);
        std::chrono::steady_clock::time_point insEnd = std::chrono::steady_clock::now();
        numInsertedTot += numInserted;
        insTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(insEnd - insBegin).count();

        std::chrono::steady_clock::time_point containsBegin = std::chrono::steady_clock::now();
        containsIntsInRange(&filter, from, numInserted);
        std::chrono::steady_clock::time_point containsEnd = std::chrono::steady_clock::now();
        contTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(containsEnd - containsBegin).count();

        fpRate = getFPRate(&filter, to, 2 * to);
        fpRateTot += fpRate;
//        double availability = filter.availability();
//        availabilityTot += availability;

        std::chrono::steady_clock::time_point delBegin = std::chrono::steady_clock::now();
        deleteAllInRange(&filter, from, numInserted);
        std::chrono::steady_clock::time_point delEnd = std::chrono::steady_clock::now();
        delTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(delEnd - delBegin).count();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        std::cout << i << ". iter" << std::endl;
        std::cout << "Inserted: " << numInserted << "/" << numOfElements << std::endl;
        std::cout << "false positive rate is "
                  << fpRate << "%\n";
//        std::cout << "availability: "
//                  << availability << "%\n";
    }


    std::ofstream myfile("/home/patrik/FAKS/3_SEM_DIPL/BIOINF/projekt_impl/CuckooFilter/Tests/CF_time-size.txt");
    if (myfile.is_open()) {
        myfile
                << "# format -> '#' marks comment, 's' table size, 'fs' fingerprint size, 'ne' num of elements inserted to the table, 'n' num of iterations conducted in the test, 'u' unit of measurement,\n";
        myfile
                << "# 'ni' marks avg number of inserted elements, 'fp' the false positive rate and 'a' the avg percentage of free space in the table,\n";
        myfile << "# 'i' marks avg insertion time, 'l' avg lookup time and 'd' avg deletion time\n";
        myfile << "s " << tableSize << "\n";
        myfile << "fs " << fs << "\n";
        myfile << "ne " << numOfElements << "\n";
        myfile << "n " << n << "\n";
        myfile << "ni " << numInsertedTot / ((float) n) << "\n";
        myfile << "fp " << fpRateTot / ((float) n) << "\n";
        myfile << "a " << availabilityTot / ((float) n) << "\n";
        myfile << "u [µs]" << "\n";
        myfile << "i " << insTotalTime / n << "\n";
        myfile << "l " << contTotalTime / n << "\n";
        myfile << "d " << delTotalTime / n << "\n";
        myfile.close();
    }


    std::cout << "\nAvg insertion time: " << insTotalTime / n
              << "[µs] (for " << numInsertedTot / ((float) n) << " elements inserted in avg)" << std::endl;
    std::cout << "Avg lookup time: " << contTotalTime / n
              << "[µs] (for " << numInsertedTot / ((float) n) << " elements inserted in avg)" << std::endl;
    std::cout << "Avg deletion time: " << delTotalTime / n
              << "[µs] (for " << numInsertedTot / ((float) n) << " elements inserted in avg)" << std::endl;
    std::cout << "Avg time (all operations): " << totalTime / n
              << "[µs]" << std::endl;

}