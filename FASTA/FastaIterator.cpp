#include "FastaReader.h"
#include "FastaIterator.h"


FastaIterator::FastaIterator(FastaReader *reader) : reader(reader) {
    reader->restart();
}

string FastaIterator::next() {
    return reader->nextKMere();
}

bool FastaIterator::hasNext() {
    return !reader->isDone();
}
