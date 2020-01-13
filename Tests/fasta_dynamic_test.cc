#include "../ArgParser/cxxopts.hpp"
#include "../DCF/dynamic_cuckoo_filter.h"
#include "../FASTA/fasta_reader.cpp"
#include "../FASTA/fasta_iterator.h"
#include <chrono>


template<typename fp_type>
int insertKmers(DynamicCuckooFilter<string, fp_type> *filter, FastaIterator *iterator) {

    int numOfInserted = 0;
    while (iterator->hasNext()) {
        string kmere = iterator->next();
        if (!filter->insertElement(kmere)) {
            break;
        }
        numOfInserted++;
    }

    return numOfInserted;
}

template<typename fp_type>
void containsKmers(DynamicCuckooFilter<string, fp_type> *filter, FastaIterator *iterator) {
    while (iterator->hasNext()) {
        string kmere = iterator->next();
        assert(filter->containsElement(kmere));
    }
}


template<typename fp_type>
void deleteAll(DynamicCuckooFilter<string, fp_type> *filter, FastaIterator *iterator) {
    while (iterator->hasNext()) {
        string kmere = iterator->next();
        filter->deleteElement(kmere);
    }
}


int main(int argc, char **argv) {
    cxxopts::Options options("CuckooFilter", "Testing example for CF");
    options.add_options()
            ("f,file", "FASTA formatted file", cxxopts::value<std::string>())
            ("k,kmer_size", "K-mers size", cxxopts::value<int>()->default_value("10"))
            ("s,filter_size", "Filter size", cxxopts::value<int>()->default_value("1024"));
    auto result = options.parse(argc, argv);

    std::string fileName = result["file"].as<std::string>();
    int kmerSize = result["kmer_size"].as<int>();
    size_t tableSize = result["filter_size"].as<int>();;

    FastaReader reader(fileName, kmerSize);
    FastaIterator iterator(&reader);

    int n = 10;

    double totalTime = 0.;
    double insTotalTime = 0.;
    double contTotalTime = 0.;
    double delTotalTime = 0.;

    size_t numInserted;
    float fpRate;


    for (size_t i = 0; i < n; ++i) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        DynamicCuckooFilter<string, uint16_t> filter(tableSize, 16, 4);

        std::chrono::steady_clock::time_point insBegin = std::chrono::steady_clock::now();
        numInserted = insertKmers(&filter, &iterator);
        std::chrono::steady_clock::time_point insEnd = std::chrono::steady_clock::now();
        insTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(insEnd - insBegin).count();
        reader.restart();

        std::chrono::steady_clock::time_point containsBegin = std::chrono::steady_clock::now();
        containsKmers(&filter, &iterator);
        std::chrono::steady_clock::time_point containsEnd = std::chrono::steady_clock::now();
        contTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(containsEnd - containsBegin).count();
        reader.restart();

        std::chrono::steady_clock::time_point delBegin = std::chrono::steady_clock::now();
        deleteAll(&filter, &iterator);
        std::chrono::steady_clock::time_point delEnd = std::chrono::steady_clock::now();
        delTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(delEnd - delBegin).count();
        reader.restart();

// TODO: implement
//  fpRate = getFPRate();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        std::cout << i << ". iter" << std::endl;
        std::cout << "Inserted: " << numInserted << std::endl;
//        std::cout << "false positive rate is "
//                  << fpRate << "%\n";
    }

    std::cout << "\nAvg insertion time: " << insTotalTime / n
              << "[µs] (for " << numInserted << " elements)" << std::endl;
    std::cout << "Avg lookup time: " << contTotalTime / n
              << "[µs] (for " << numInserted << " elements)" << std::endl;
    std::cout << "Avg deletion time: " << delTotalTime / n
              << "[µs] (for " << numInserted << " elements)" << std::endl;
    std::cout << "Avg time = " << totalTime / n
              << "[µs]" << std::endl;

    return 0;
}