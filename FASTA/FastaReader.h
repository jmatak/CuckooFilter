#ifndef CUCKOOFILTER_FASTAREADER_H
#define CUCKOOFILTER_FASTAREADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <utility>
#include <stdexcept>
#include <string>

using namespace std;

/**
 * Implementation of reader of FASTA format.
 * Idea for implementation comes from https://rosettacode.org/wiki/FASTA_format#C.2B.2B
 */
class FastaReader {
public:
    string fileName;
    int k;

    FastaReader(string fileName, int k);

    string nextKMere();

    bool isDone();

    void restart();

private:
    ifstream *currentPosition;
    string identificator;
    string currentKMere;
    string buffer;

    void prepareNext();

    void initialize();
};

#endif
