#include "../ArgParser/cxxopts.hpp"
#include "../FASTA/fasta_reader.h"
#include "../FASTA/fasta_iterator.h"
#include "../CF/cuckoo_filter.h"
#include <chrono>
#include <sstream>
#include <fstream>


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
int insertKmers(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, FastaIterator *iterator) {

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

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void
containsKmers(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, FastaIterator *iterator,
              size_t numInserted) {
    std::size_t i = 0;
    while (iterator->hasNext() && i < numInserted) {
        string kmere = iterator->next();
        assert(filter->containsElement(kmere));
        i++;
    }
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
float
getFPRate(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, std::string randomKmersPath) {
    std::string line;

    std::ifstream randomKmersFile(randomKmersPath);


    size_t total_queries = 0;
    size_t false_queries = 0;
    while (std::getline(randomKmersFile, line)) {
        if (filter->containsElement(line)) {
            false_queries++;
        }
        total_queries++;
    }

    return 100.0 * ((float) false_queries) / ((float) total_queries);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void deleteAll(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type> *filter, FastaIterator *iterator,
               size_t numInserted) {

    std::size_t i = 0;
    while (iterator->hasNext() && i < numInserted) {
        string kmere = iterator->next();
        filter->deleteElement(kmere);
        i++;
    }
}

std::size_t getNumOfKmers(FastaIterator *iterator) {
    size_t n = 0;
    while (iterator->hasNext()) {
        iterator->next();
        n += 1;
    }
    return n;
}

int main(int argc, char **argv) {
    cxxopts::Options options("CuckooFilter", "Testing example for CF");
    options.add_options()
            ("f,file", "FASTA formatted file", cxxopts::value<std::string>())
            ("k,kmer_size", "K-mers size", cxxopts::value<int>()->default_value("10"))
            ("s,filter_size", "Filter size", cxxopts::value<int>()->default_value("1024"));
    auto result = options.parse(argc, argv);

//    std::string fileName = result["file"].as<std::string>();
//    int kmerSize = result["kmer_size"].as<int>();
//    size_t tableSize = result["filter_size"].as<int>();;

    std::string fileName = "/home/patrik/FAKS/3_SEM_DIPL/BIOINF/projekt_impl/CuckooFilter/Data/ecoli.fna";
    std::string randomKmersPath = "/home/patrik/FAKS/3_SEM_DIPL/BIOINF/projekt_impl/CuckooFilter/Data/random/random_ecoli_kmers_100.txt";
    int kmerSize = 100;
    size_t tableSize = 10000;

    FastaReader reader(fileName, kmerSize);
    FastaIterator iterator(&reader);

    // fingerprint size
    static const size_t fs = 12;
    // entries per bucket
    static const size_t epb = 4;

    int n = 1;

    double totalTime = 0.;
    double insTotalTime = 0.;
    double contTotalTime = 0.;
    double delTotalTime = 0.;

    size_t numInserted;
    float fpRate;

    size_t numOfKmers = getNumOfKmers(&iterator);
    reader.restart();

    for (size_t i = 0; i < n; ++i) {
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        CuckooFilter<string, epb, fs, uint16_t> filter(tableSize);

        std::chrono::steady_clock::time_point insBegin = std::chrono::steady_clock::now();
        numInserted = insertKmers(&filter, &iterator);
        std::chrono::steady_clock::time_point insEnd = std::chrono::steady_clock::now();
        insTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(insEnd - insBegin).count();
        reader.restart();

        fpRate = getFPRate(&filter, randomKmersPath);
        std::cout << "FPRate: " << fpRate << std::endl;

        std::chrono::steady_clock::time_point containsBegin = std::chrono::steady_clock::now();
        containsKmers(&filter, &iterator, numInserted);
        std::chrono::steady_clock::time_point containsEnd = std::chrono::steady_clock::now();
        contTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(containsEnd - containsBegin).count();
        reader.restart();

        std::chrono::steady_clock::time_point delBegin = std::chrono::steady_clock::now();
        deleteAll(&filter, &iterator, numInserted);
        std::chrono::steady_clock::time_point delEnd = std::chrono::steady_clock::now();
        delTotalTime += std::chrono::duration_cast<std::chrono::microseconds>(delEnd - delBegin).count();
        reader.restart();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        totalTime += std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        std::cout << i << ". iter" << std::endl;
        std::cout << "Inserted: " << numInserted << "/" << numOfKmers << std::endl;
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