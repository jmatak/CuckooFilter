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
    size_t tableSize = 1000000;

    //Elements inserted in the filter are from 0 to numOfElements
    size_t numOfElements = tableSize;

    int n = 30;

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

    for (size_t i = 0; i < n; ++i) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> filter(tableSize);

        std::chrono::steady_clock::time_point insBegin = std::chrono::steady_clock::now();
        numInserted = insertIntsInRange(&filter, from, to);
        numInsertedTot += numInserted;
        std::chrono::steady_clock::time_point insEnd = std::chrono::steady_clock::now();
        insTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(insEnd - insBegin).count();

        std::chrono::steady_clock::time_point containsBegin = std::chrono::steady_clock::now();
        containsIntsInRange(&filter, from, numInserted);
        std::chrono::steady_clock::time_point containsEnd = std::chrono::steady_clock::now();
        contTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(containsEnd - containsBegin).count();

        fpRate = getFPRate(&filter, to, 2 * to);
        fpRateTot += fpRate;

        std::chrono::steady_clock::time_point delBegin = std::chrono::steady_clock::now();
        deleteAll(&filter, from, numInserted);
        std::chrono::steady_clock::time_point delEnd = std::chrono::steady_clock::now();
        delTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(delEnd - delBegin).count();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        std::cout << i << ". iter" << std::endl;
        std::cout << "Inserted: " << numInserted << "/" << numOfElements << std::endl;
        std::cout << "false positive rate is "
                  << fpRate << "%\n";
    }

    std::ofstream myfile("/home/josip/CLionProjects/CF/Tests/test4_1000000.txt");
    if (myfile.is_open()) {
        myfile
                << "# format -> '#' marks comment, 's' table size, 'fs' fingerprint size, 'ne' num of elements inserted to the table, 'n' num of iterations conducted in the test, 'u' unit of measurement,\n";
        myfile
                << "# 'ni' marks avg number of inserted elements, 'fp' the false positive rate,\n";
        myfile << "# 'i' marks avg insertion time, 'l' avg lookup time and 'd' avg deletion time\n";
        myfile << "s " << tableSize << "\n";
        myfile << "fs " << bits_per_fp << "\n";
        myfile << "ne " << numOfElements << "\n";
        myfile << "n " << n << "\n";
        myfile << "ni " << numInsertedTot / ((float) n) << "\n";
        myfile << "fp " << fpRateTot / ((float) n) << "\n";
        myfile << "u [µs]" << "\n";
        myfile << "i " << insTotalTime / n << "\n";
        myfile << "l " << contTotalTime / n << "\n";
        myfile << "d " << delTotalTime / n << "\n";
        myfile.close();
    }

    std::cout << "\nAvg insertion time: " << insTotalTime / n
              << "[µs] (for " << numOfElements << " elements)" << std::endl;
    std::cout << "Avg lookup time: " << contTotalTime / n
              << "[µs] (for " << numOfElements << " elements)" << std::endl;
    std::cout << "Avg deletion time: " << delTotalTime / n
              << "[µs] (for " << numOfElements << " elements)" << std::endl;
    std::cout << "Avg time = " << totalTime / n
              << "[µs]" << std::endl;

    return 0;
}