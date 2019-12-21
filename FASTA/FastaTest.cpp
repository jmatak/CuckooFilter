#include <iostream>
#include <fstream>
#include "FastaIterator.h"
#include "FastaReader.h"

int main() {
    int kmers = 10;
    // TODO: Working with relative paths
    string fileName = "FULL_PATH";
    FastaReader reader(fileName, kmers);
    FastaIterator iterator(&reader);

    while (iterator.hasNext()) {
        string kmere = iterator.next();
        std::cout << kmere << std::endl;
        if (kmers != kmere.size()) {
            std::cout << "Error has occured, kmere of different size!" << std::endl;
            exit(1);
        }
    }
}