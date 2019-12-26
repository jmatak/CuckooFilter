#include <iostream>
#include <assert.h>
#include "Filter.cpp"
#include "../FASTA/FastaReader.h"
#include "../FASTA/FastaIterator.h"

using namespace std;

int main() {
    string fileName = "FULL_PATH";
    int kmers = 10;
    size_t total_items = 1024;


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