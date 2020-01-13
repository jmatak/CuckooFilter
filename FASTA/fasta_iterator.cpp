#include "fasta_reader.h"
#include "fasta_iterator.h"

/**
 * Constructor of FASTA format iterator. Takes instace of FastaReader and initializes it for new reading.
 *
 * @param reader Online implementation of FASTA format reader
 */
FastaIterator::FastaIterator(FastaReader *reader) : reader(reader) {
    reader->restart();
}

/**
 * Getting next k-mere from FastaReader
 *
 * @return Next k-mere string
 */
string FastaIterator::next() {
    return reader->nextKMere();
}

/**
 * Checks if there is next k-mere in input. If not, exception is thrown from FastaReader.
 *
 * @return True if stream is not finished.
 */
bool FastaIterator::hasNext() {
    return !reader->isDone();
}