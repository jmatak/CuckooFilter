#ifndef CUCKOOFILTER_FASTAITERATOR_H
#define CUCKOOFILTER_FASTAITERATOR_H

#include <string>
#include "FastaReader.h"

using namespace std;

/**
 * Default implementation of Iterator design pattern.
 * https://refactoring.guru/design-patterns/iterator
 *
 * @tparam T Type of value returned from iterator.
 */
template<typename T>
class Iterator {
public:
    typedef T value_type;

    virtual bool hasNext() = 0;

    virtual T next() = 0;
};

/**
 *  Implementation of Iterator pattern  through FastaReader class. Returns string of k-mere.
 */
class FastaIterator : public Iterator<string> {
    FastaReader *reader;
public:
    FastaIterator(FastaReader *reader);

    string next() override;

    bool hasNext() override;
};

#endif //CUCKOOFILTER_FASTAITERATOR_H
