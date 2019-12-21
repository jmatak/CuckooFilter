#include "FastaReader.h"


FastaReader::FastaReader(string fileName, int k) : fileName(std::move(fileName)), k(k) {
    if (k <= 0)
        throw std::runtime_error("K-meres should be of size larger than 0.");
    initialize();
}

void FastaReader::initialize() {
    currentPosition = new std::ifstream(fileName, std::ifstream::in);
    if (!currentPosition->is_open())
        throw std::runtime_error("Please provide a valid FASTA formatted file! Filename: " + fileName);

    while (true) {
        string line;
        // Reading input line
        if (!std::getline(*currentPosition, line)) break;
        // Skip empty till FASTA format with ">" appears
        if (line.empty()) continue;
        if (line[0] != '>') continue;

        identificator = line.substr(1);
        prepareNext();
        break;
    }
}

void FastaReader::restart() {
    initialize();
}

void FastaReader::prepareNext() {
    if (buffer.empty() || buffer.size() < k) {
        string line;
        while (buffer.size() < k) {
            if (!std::getline(*currentPosition, line)) {
                buffer.clear();
                break;
            }
            buffer += line;
        }
    }
}

string FastaReader::nextKMere() {
    if (buffer.empty()) {
        throw std::runtime_error("There are no more k-meres in genome.");
    }
    currentKMere = buffer.substr(0, k);
    buffer = buffer.substr(1);
    prepareNext();
    return currentKMere;
}

bool FastaReader::isDone() {
    return buffer.empty();
}


