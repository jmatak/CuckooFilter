#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>

#include "BitManager.cpp"


struct Bucket {
    uint8_t* data;
};


template<typename fp_type>
class CuckooTable {

public:
    size_t table_size;
    uint32_t fp_mask;

    CuckooTable(size_t, BitManager<fp_type>*, size_t, uint32_t, size_t);
    ~CuckooTable();
    size_t getTableSize() const;
    size_t maxNoOfElements();
    inline uint32_t getFingerprint(size_t, size_t);
    size_t fingerprintCount(size_t i) const;
    inline void insertFingerprint(size_t, size_t, uint32_t);
    inline bool replacingFingerprintInsertion(size_t, uint32_t, bool, uint32_t&);
    bool containsFingerprint(size_t, uint32_t);
    bool containsFingerprint(size_t, size_t, uint32_t);
    bool deleteFingerprint(const uint32_t, const size_t);
    bool deleteFingerprint(const uint32_t, const size_t, const size_t);

private:
    size_t bits_per_fp;
    size_t entries_per_bucket;
    size_t bytes_per_bucket;

    BitManager<fp_type>* bit_manager;
    Bucket* buckets;
};




template<typename fp_type>
CuckooTable<fp_type>::CuckooTable(size_t table_size,
                                  BitManager<fp_type>* bit_manager,
                                  size_t bits_per_fp,
                                  uint32_t fp_mask,
                                  size_t entries_per_bucket) {

    this->table_size = table_size;
    this->bit_manager = bit_manager;
    this->bits_per_fp = bits_per_fp;
    this->fp_mask = fp_mask;
    this->entries_per_bucket = entries_per_bucket;
    this->bytes_per_bucket = (entries_per_bucket * bits_per_fp) / 8;

    buckets = new Bucket[table_size];
    for(size_t i = 0; i < table_size; i++){
        buckets[i].data = new uint8_t[entries_per_bucket];
        memset(buckets[i].data, 0, bytes_per_bucket);
    }
    //memset(buckets, 0, CuckooTable::bytes_per_bucket * table_size); // set all bits to 0
}

template<typename fp_type>
CuckooTable<fp_type>::~CuckooTable() {
    delete[] buckets;
}

template<typename fp_type>
size_t CuckooTable<fp_type>::getTableSize() const {
    return table_size;
}

template<typename fp_type>
size_t CuckooTable<fp_type>::maxNoOfElements() {
    return entries_per_bucket * table_size;
}

template<typename fp_type>
inline uint32_t CuckooTable<fp_type>::getFingerprint(const size_t i, const size_t j) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t fp = bit_manager->read(j, bucket);
    return fp & fp_mask;
}

template<typename fp_type>
size_t CuckooTable<fp_type>::fingerprintCount(const size_t i) const {
    size_t count = 0;
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) != 0) {
            count++;
        }
    }
    return count;
}

template<typename fp_type>
void CuckooTable<fp_type>::insertFingerprint(const size_t i, const size_t j, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t efp = fp & fp_mask;
    bit_manager->write(j, bucket, efp);
}

template<typename fp_type>
inline bool CuckooTable<fp_type>::replacingFingerprintInsertion(const size_t i, const uint32_t fp,
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


template<typename fp_type>
bool CuckooTable<fp_type>::containsFingerprint(const size_t i, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    uint64_t val = *((uint64_t *)bucket);

    return bit_manager->hasvalue(val, fp);
}

template<typename fp_type>
bool CuckooTable<fp_type>::containsFingerprint(const size_t i1, const size_t i2, const uint32_t fp) {
    return
            bit_manager->hasvalue(
                    *((uint64_t *)buckets[i1].data), fp)
            ||
            bit_manager->hasvalue(
                    *((uint64_t *)buckets[i2].data), fp);

}

template<typename fp_type>
bool CuckooTable<fp_type>::deleteFingerprint(const uint32_t fp, const size_t i) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) == fp) {
            insertFingerprint(i, j, 0);
            return true;
        }
    }
    return false;
}


template<typename fp_type>
bool CuckooTable<fp_type>::deleteFingerprint(const uint32_t fp, const size_t i1, const size_t i2) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i1, j) == fp) {
            insertFingerprint(i1, j, 0);
            return true;
        }
        if (getFingerprint(i2, j) == fp) {
            insertFingerprint(i2, j, 0);
            return true;
        }
    }
    return false;
}