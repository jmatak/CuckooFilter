#include <iostream>
#include "CuckooFilter.h"
#include "../FASTA/FastaReader.h"
#include "../FASTA/FastaIterator.h"
#include "../ArgParser/cxxopts.hpp"

using namespace std;

int main(int argc, char **argv) {
    cxxopts::Options options("CuckooFilter", "Testing example for CF");
    options.add_options()
            ("f,file", "FASTA formatted file", cxxopts::value<std::string>())
            ("k,kmer_size", "K-mers size", cxxopts::value<int>()->default_value("10"))
            ("s,filter_size", "Filter size", cxxopts::value<int>()->default_value("1024"));
    auto result = options.parse(argc, argv);

    string fileName = result["file"].as<string>();
    int kmers = result["kmer_size"].as<int>();
    size_t total_items = result["filter_size"].as<int>();;

    CuckooFilter<string> filter(total_items);
    FastaReader reader(fileName, kmers);
    FastaIterator iterator(&reader);

    while (iterator.hasNext()) {
        string kmere = iterator.next();
        if (!filter.insertElement(kmere)) {
            break;
        }
    }

    reader.restart();

    int no_kmers = 0, no_hits = 0;
    while (iterator.hasNext()) {
        string kmere = iterator.next();
        bool hit = filter.containsElement(kmere);
        no_kmers++;
        if (hit) no_hits++;
    }

    cout << no_hits << "/" << no_kmers << endl;
    cout << no_hits / (double) no_kmers << endl;

    return 0;
}