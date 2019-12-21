#include "FastaReader.h"


/**
 * Constructor of reader from FASTA format. If file name is not valid, an exception is thrown, same happens
 * if k is not of satisfying size.
 *
 * @param fileName FASTA formatted file
 * @param k Size of k-mere.
 */
FastaReader::FastaReader(string fileName, int k) : fileName(std::move(fileName)), k(k) {
    if (k <= 0)
        throw std::runtime_error("K-meres should be of size larger than 0.");
    initialize();
}

/**
 * Starting initialization of stream buffer for online k-mere reading.
 */
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

/**
 * Initializing reader for new usage
 */
void FastaReader::restart() {
    initialize();
}

/**
 * Preparing buffer for next output. If buffer is of size less than provided k, buffer is cleared and input is finished.
 */
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

/**
 * If stream is finished, method throws an exception, otherwise returns next k-mere.
 * @return  Next k-mere in streams
 */
string FastaReader::nextKMere() {
    if (buffer.empty()) {
        throw std::runtime_error("There are no more k-meres in genome.");
    }
    currentKMere = buffer.substr(0, k);
    buffer = buffer.substr(1);
    prepareNext();
    return currentKMere;
}

/**
 * Checks if stream is finished or not.
 * @return True if stream is finished.
 */
bool FastaReader::isDone() {
    return buffer.empty();
}