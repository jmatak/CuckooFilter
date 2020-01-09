#include <iostream>
#include <fstream>
#include <vector>
#include "FastaIterator.h"
#include "FastaReader.h"

int main() {
    int kmers = 10;
    // TODO: Working with relative paths
    string fileName = "/home/josipm/Documents/Programello/FER9/BIOINF/Project/CuckooFilter/Data/ecoli_small.fna";
    FastaReader reader(fileName, kmers);
    FastaIterator iterator(&reader);


    int kmers_count = 0;
    std::vector<std::string> allKmers;


    while (iterator.hasNext()) {
        string kmere = iterator.next();
        std::cout << kmere << std::endl;
        if (kmers != kmere.size()) {
            std::cout << "Error has occured, kmere of different size!" << std::endl;
            exit(1);
        }
        allKmers.push_back(kmere);
        kmers_count++;
    }


    reader.restart();
    int kmers_count2 = 0;
    while (iterator.hasNext()) {
        string kmere = iterator.next();
        string kmere1 = allKmers.at(kmers_count2);

        if (kmere.compare(kmere1) != 0) {
            cout << kmers_count2 << " -> " << kmere << " and " << kmere1 << endl;
            break;
        }

        std::cout << kmere << std::endl;
        if (kmers != kmere.size()) {
            std::cout << "Error has occured, kmere of different size!" << std::endl;
            exit(1);
        }
        kmers_count2++;
    }


    std::cout << kmers_count << endl;
    std::cout << kmers_count2 << endl;


}