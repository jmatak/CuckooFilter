#include "../ArgParser/cxxopts.hpp"
#include "../CF/cuckoo_filter.h"
#include <chrono>
#include <iostream>
#include <fstream>


static const size_t bits_per_fp = 16;
static const size_t entries_per_bucket = 4;
typedef size_t element_type;
typedef uint16_t fp_type;


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
int insertIntsInRange(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from,
                      size_t to) {
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
void containsIntsInRange(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from,
                         size_t to) {
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
void
deleteAllInRange(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    for (size_t i = from; i < to; i++) {
        filter->deleteElement(i);
    }
}


int main(int argc, char **argv) {
    size_t tableSize = 40000;

    // elements inserted in the filter are from 0 to numOfElements
    size_t numOfElements = 100000;

    int n = 1;

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

    size_t bucketCount;

    std::cout << "Starting demo..." << std::endl;
    std::cout << std::endl;

    for (size_t i = 0; i < n; ++i) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> filter(tableSize);
        bucketCount = filter.getTableSize();
        std::cout << "Filter size in bytes: " << sizeof(filter) << std::endl;

        std::chrono::steady_clock::time_point insBegin = std::chrono::steady_clock::now();
        numInserted = insertIntsInRange(&filter, from, to);
        std::chrono::steady_clock::time_point insEnd = std::chrono::steady_clock::now();
        numInsertedTot += numInserted;
        insTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(insEnd - insBegin).count();

        std::chrono::steady_clock::time_point containsBegin = std::chrono::steady_clock::now();
        containsIntsInRange(&filter, from, numInserted);
        std::chrono::steady_clock::time_point containsEnd = std::chrono::steady_clock::now();
        contTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(containsEnd - containsBegin).count();

        fpRate = getFPRate(&filter, to, to + 50000);
        fpRateTot += fpRate;

        std::chrono::steady_clock::time_point delBegin = std::chrono::steady_clock::now();
        deleteAllInRange(&filter, from, numInserted);
        std::chrono::steady_clock::time_point delEnd = std::chrono::steady_clock::now();
        delTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(delEnd - delBegin).count();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        double availability = filter.availability();
        availabilityTot += availability;

        std::cout << std::endl;
        std::cout << "----------------------------------------" <<std::endl;
        std::cout << i+1 << ". iter" << std::endl;
        std::cout << "Inserted: " << numInserted << "/" << numOfElements << std::endl;
        std::cout << "False positive rate: " << fpRate << std::endl;
        std::cout << "----------------------------------------" <<std::endl;
    }

    std::cout << std::endl;
    std::cout << "Insertion (average) [µs]: " << insTotalTime / n << std::endl;
    std::cout << "Lookup (average) [µs]: " << contTotalTime / n << std::endl;
    std::cout << "Deletion (average) [µs]: " << delTotalTime / n << std::endl;
    std::cout << "Total time (ins + look + del) [µs]: " << delTotalTime / n << std::endl;

    std::ofstream myfile("./result_cf.txt");
    if (myfile.is_open()) {
        myfile << "Number of buckets: " << bucketCount << std::endl;
        myfile << "Fingerprint size in bits: " << fs << "\n";
        myfile << "Total number of elements: " << numOfElements << std::endl;
        myfile << "Number of inserted elements: " << numInsertedTot / ((float) n) << std::endl;
        myfile << "Iteration count: " << n << std::endl;
        myfile << "False positive rate: " << fpRateTot / ((float) n) << std::endl;
        myfile << "Availability: " << availabilityTot / ((float) n) << std::endl;
        myfile << "--------------------TIME ANALYSIS-----------------------" << std::endl;
        myfile << "Insertion (average) [µs]: " << insTotalTime / n << std::endl;
        myfile << "Lookup (average) [µs]: " << contTotalTime / n << std::endl;
        myfile << "Deletion (average) [µs]: " << delTotalTime / n << std::endl;
        myfile << "Total time (ins + look + del, average) [µs]: " << delTotalTime / n << std::endl;
        myfile.close();
    }

    std::cout << std::endl;
    std::cout << "Demo finished successfully." << insTotalTime  << std::endl;

}