#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "BitManager.cpp"


template<size_t bits_per_fp = 8, typename fp_type = uint8_t>
class CuckooTable {

    static const size_t bytes_per_bucket = (entries_per_bucket * bits_per_fp) / 8;

    struct Bucket {
        // TODO: possibly change to uint8_t*
        uint8_t data[bytes_per_bucket];
    };

private:
    BitManager<fp_type> *bit_manager;
    Bucket *buckets;

public:

    const size_t table_size;
    static const size_t entries_per_bucket = 4;
    static const uint32_t fp_mask = (1ULL << bits_per_fp) - 1;
    CuckooTable(size_t table_size);
    ~CuckooTable();
    size_t getTableSize() const;
    size_t maxNoOfElements();
    inline uint32_t getFingerprint(size_t i, size_t j);
    size_t fingerprintCount(size_t i) const;
    inline void insertFingerprint(size_t i, size_t j, uint32_t fp);
    inline bool replacingFingerprintInsertion(size_t i, uint32_t fp, bool eject, uint32_t &prev_fp);
    bool containsFingerprint(size_t i, uint32_t fp);
    bool containsFingerprint(size_t i1, size_t i2, uint32_t fp);
    bool deleteFingerprint(uint32_t fp, size_t i);
};


template<size_t bits_per_fp, typename fp_type>
CuckooTable<bits_per_fp, fp_type>::CuckooTable(const size_t table_size) : table_size(table_size) {
    buckets = new Bucket[table_size];
    memset(buckets, 0, CuckooTable::bytes_per_bucket * table_size); // set all bits to 0

    if (entries_per_bucket == 4 && bits_per_fp == 4) {
        bit_manager = new BitManager4<fp_type>();
    }
    else if (entries_per_bucket == 4 && bits_per_fp == 8) {
        bit_manager = new BitManager8<fp_type>();
    }
    else if (entries_per_bucket == 4 && bits_per_fp == 12) {
        bit_manager = new BitManager12<fp_type>();
    }
    else if (entries_per_bucket == 4 && bits_per_fp == 16) {
        bit_manager = new BitManager16<fp_type>();
    }
    else if (entries_per_bucket == 2 && bits_per_fp == 32) {
        bit_manager = new BitManager32<fp_type>();
    }
    else {
        // TODO: print error, throw exception
    }
}

template<size_t bits_per_fp, typename fp_type>
CuckooTable<bits_per_fp, fp_type>::~CuckooTable() {
    delete[] buckets;
    delete bit_manager;
}

template<size_t bits_per_fp, typename fp_type>
size_t CuckooTable<bits_per_fp, fp_type>::getTableSize() const {
    return table_size;
}

template<size_t bits_per_fp, typename fp_type>
size_t CuckooTable<bits_per_fp, fp_type>::maxNoOfElements() {
    return entries_per_bucket * table_size;
}

template<size_t bits_per_fp, typename fp_type>
inline uint32_t CuckooTable<bits_per_fp, fp_type>::getFingerprint(const size_t i, const size_t j) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t fp = bit_manager->read(j, bucket);
    return fp & fp_mask;
}

template<size_t bits_per_fp, typename fp_type>
size_t CuckooTable<bits_per_fp, fp_type>::fingerprintCount(const size_t i) const {
    size_t count = 0;
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) != 0) {
            count++;
        }
    }
    return count;
}

template<size_t bits_per_fp, typename fp_type>
void CuckooTable<bits_per_fp, fp_type>::insertFingerprint(const size_t i, const size_t j, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t efp = fp & fp_mask;
    bit_manager->write(j, bucket, efp);
}

template<size_t bits_per_fp, typename fp_type>
inline bool CuckooTable<bits_per_fp, fp_type>::replacingFingerprintInsertion(const size_t i, const uint32_t fp,
                                                                const bool eject, uint32_t &prev_fp) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) == 0) {
            insertFingerprint(i, j, fp);
            return true;
        }
    }

    if (eject) {
        size_t next = rand() % entries_per_bucket;
        prev_fp = getFingerprint(i, next);
        insertFingerprint(i, next, fp);
    }
    return false;
}

template<size_t bits_per_fp, typename fp_type>
bool CuckooTable<bits_per_fp, fp_type>::containsFingerprint(const size_t i, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    uint64_t val = *((uint64_t *)bucket);

    return bit_manager->hasvalue(val, fp);
}

template<size_t bits_per_fp, typename fp_type>
bool CuckooTable<bits_per_fp, fp_type>::containsFingerprint(const size_t i1, const size_t i2, const uint32_t fp) {
    const uint8_t *b1 = buckets[i1].data;
    const uint8_t *b2 = buckets[i2].data;

    uint64_t val1 = *((uint64_t *)b1);
    uint64_t val2 = *((uint64_t *)b2);

    return bit_manager->hasvalue(val1, fp) || bit_manager->hasvalue(val2, fp);
}

template<size_t bits_per_fp, typename fp_type>
bool CuckooTable<bits_per_fp, fp_type>::deleteFingerprint(const uint32_t fp, const size_t i) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) == fp) {
            insertFingerprint(i, j, 0);
            return true;
        }
    }
    return false;
}