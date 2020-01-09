#ifndef CUCKOOFILTER_CUCKOOTABLE_H
#define CUCKOOFILTER_CUCKOOTABLE_H

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <type_traits>
#include <exception>

#include "BitManager.h"

struct Bucket {
    // TODO: stack vs heap?
    uint8_t *data;
};

template<typename fp_type>
class CuckooTable {

private:
    size_t entries_per_bucket = 4;
    size_t bytes_per_bucket = (entries_per_bucket * bits_per_fp) / 8;
    size_t table_size;
    size_t bits_per_fp;
    uint32_t fp_mask;

    BitManager<fp_type> *bit_manager;
    Bucket *buckets;

public:

    CuckooTable(size_t table_size, size_t bits_per_fp, size_t entries_per_bucket, uint32_t fp_mask);

    ~CuckooTable();

    size_t getTableSize();

    size_t maxNoOfElements();

    uint32_t getFingerprint(size_t i, size_t j);

    size_t fingerprintCount(size_t i);

    size_t freeEntries();

    void printTable();

    void insertFingerprint(size_t i, size_t j, uint32_t fp);

    bool replacingFingerprintInsertion(size_t i, uint32_t fp, bool eject, uint32_t &prev_fp);

    bool containsFingerprint(size_t i, uint32_t fp);

    bool containsFingerprint(size_t i1, size_t i2, uint32_t fp);

    bool deleteFingerprint(uint32_t fp, size_t i);
};


#endif //CUCKOOFILTER_CUCKOOTABLE_H