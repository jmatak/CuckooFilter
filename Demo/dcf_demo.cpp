#include "../ArgParser/cxxopts.hpp"
#include "../DCF/dynamic_cuckoo_filter.h"
#include <chrono>
#include <fstream>

static const size_t bits_per_fp = 16;
static const size_t entries_per_bucket = 4;
typedef size_t element_type;
typedef uint16_t fp_type;

int insertIntsInRange(DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    assert(from < to);

    size_t numInserted = 0;
    for (size_t i = from; i < to; i++, numInserted++) {
        if (!filter->insertElement(i)) {
            break;
        }
    }
    return numInserted;
}

template<typename fp_type>
void containsIntsInRange(DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    for (size_t i = from; i < to; i++) {
        assert(filter->containsElement(i));
    }
}

template<typename fp_type>
float getFPRate(DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
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

template<typename fp_type>
void deleteAll(DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, size_t from, size_t to) {
    for (size_t i = from; i < to; i++) {
        filter->deleteElement(i);
    }
}


int main(int argc, char **argv) {
    size_t tableSize = 40000;

    // elements inserted in the filter are from 0 to numOfElements
    size_t numOfElements = 100000;

    size_t false_positives = 50000;

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

    size_t bucketCount;

    std::cout << "Starting demo..." << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> filter(tableSize);
    bucketCount = filter.getTableSize();

    std::chrono::steady_clock::time_point insBegin = std::chrono::steady_clock::now();
    numInserted = insertIntsInRange(&filter, from, to);
    numInsertedTot += numInserted;
    std::chrono::steady_clock::time_point insEnd = std::chrono::steady_clock::now();
    insTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(insEnd - insBegin).count();

    std::chrono::steady_clock::time_point containsBegin = std::chrono::steady_clock::now();
    containsIntsInRange(&filter, from, numInserted);
    std::chrono::steady_clock::time_point containsEnd = std::chrono::steady_clock::now();
    contTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(containsEnd - containsBegin).count();

    fpRate = getFPRate(&filter, to, to + false_positives);
    fpRateTot += fpRate;

    std::chrono::steady_clock::time_point delBegin = std::chrono::steady_clock::now();
    deleteAll(&filter, from, numInserted);
    std::chrono::steady_clock::time_point delEnd = std::chrono::steady_clock::now();
    delTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(delEnd - delBegin).count();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    std::cout << std::endl;
    std::cout << "----------------------------------------" <<std::endl;
    std::cout << "Inserted: " << numInserted << "/" << numOfElements << std::endl;
    std::cout << "False positive rate: " << fpRate << std::endl;
    std::cout << "----------------------------------------" <<std::endl;

    std::cout << std::endl;
    std::cout << "Insertion (average) [µs]: " << insTotalTime << std::endl;
    std::cout << "Lookup (average) [µs]: " << contTotalTime << std::endl;
    std::cout << "Deletion (average) [µs]: " << delTotalTime << std::endl;
    std::cout << "Total time (ins + look + del) [µs]: " << delTotalTime << std::endl;

    std::ofstream myfile("./result_dcf.txt");
    if (myfile.is_open()) {


        myfile << "Dynamic Cuckoo Filter Demo" << std::endl;
        myfile << "===============================================" << std::endl;
        myfile << std::endl;
        myfile << "Number of buckets: " << bucketCount << std::endl;
        myfile << "Fingerprint size in bits: " << bits_per_fp << "\n";
        myfile << "Total number of elements: " << numOfElements << std::endl;
        myfile << "Number of inserted elements: " << numInsertedTot << std::endl;
        myfile << "Insertion score: " << numInsertedTot  << "/" << numOfElements << std::endl;
        myfile << "Number of false positives added: " << false_positives << std::endl;
        myfile << "False positive rate: " << fpRateTot << std::endl;

        myfile << std::endl;
        myfile << "--------------------COMPACTION ANALYSIS-----------------" << std::endl;
        size_t before = filter.cf_count;
        myfile << "Number of cuckoo filters before compaction: " << before << std::endl;
        filter.compact();
        size_t after = filter.cf_count;
        myfile << "Number of cuckoo filters after compaction: " << after << std::endl;

        myfile << std::endl;
        myfile << "--------------------TIME ANALYSIS-----------------------" << std::endl;
        myfile << "Insertion per element (average) [µs]: " << insTotalTime / numInsertedTot << std::endl;
        myfile << "Lookup per element (average) [µs]: " << contTotalTime / numInsertedTot << std::endl;
        myfile << "Deletion per element (average) [µs]: " << delTotalTime / numInsertedTot << std::endl;
        myfile << "Total time per element (ins + look + del, average) [µs]: " << totalTime / numInsertedTot << std::endl;


        myfile << "Insertion (average) [µs]: " << insTotalTime  << std::endl;
        myfile << "Lookup (average) [µs]: " << contTotalTime << std::endl;
        myfile << "Deletion (average) [µs]: " << delTotalTime << std::endl;
        myfile << "Total time (ins + look + del, average) [µs]: " << totalTime << std::endl;
        myfile.close();
    }

    std::cout << std::endl;
    std::cout << "Demo finished successfully."  << std::endl;
    std::cout << "Results stored to: './result_dcf.txt'" << std::endl;

    return 0;
}