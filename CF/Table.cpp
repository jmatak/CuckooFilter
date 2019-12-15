#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "BitManager.cpp"


struct Bucket {
    // TODO: stack vs heap?
    uint8_t* data;
};

template<typename fp_type>
class CuckooTable {

private:
    size_t entries_per_bucket = 4;
    size_t bytes_per_bucket = (entries_per_bucket * bits_per_fp) / 8;
    size_t table_size;
    size_t bits_per_fp;
    uint32_t fp_mask;

    BitManager<fp_type>* bit_manager;
    Bucket* buckets;

public:

    CuckooTable(size_t table_size, size_t bits_per_fp, size_t entries_per_bucket, uint32_t fp_mask);
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


template<typename fp_type>
CuckooTable<fp_type>::CuckooTable(const size_t table_size, size_t bits_per_fp,
                                  size_t entries_per_bucket, uint32_t fp_mask) {
    this->table_size = table_size;
    this->bits_per_fp = bits_per_fp;
    this->entries_per_bucket = entries_per_bucket;
    this->fp_mask = fp_mask;
    this->bytes_per_bucket = (entries_per_bucket * bits_per_fp) / 8;

    buckets = new Bucket[table_size];
    for(size_t i = 0; i < table_size; i++){
        buckets[i].data = new uint8_t[entries_per_bucket];
        memset(buckets[i].data, 0, bytes_per_bucket);
    }

    if (entries_per_bucket == 4 && bits_per_fp == 4 && std::is_same<fp_type, uint8_t>::value) {
        bit_manager = new BitManager4<fp_type>();
    }
    else if (entries_per_bucket == 4 && bits_per_fp == 8 && std::is_same<fp_type, uint8_t>::value) {
        bit_manager = new BitManager8<fp_type>();
    }
    else if (entries_per_bucket == 4 && bits_per_fp == 12 && std::is_same<fp_type, uint16_t>::value) {
        bit_manager = new BitManager12<fp_type>();
    }
    else if (entries_per_bucket == 4 && bits_per_fp == 16 && std::is_same<fp_type, uint16_t>::value) {
        bit_manager = new BitManager16<fp_type>();
    }
    else if (entries_per_bucket == 2 && bits_per_fp == 32 && std::is_same<fp_type, uint32_t>::value) {
        bit_manager = new BitManager32<fp_type>();
    }
    else {
        // TODO: print error, throw exception
    }

}

template<typename fp_type>
CuckooTable<fp_type>::~CuckooTable() {
    delete[] buckets;
    delete bit_manager;
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
    const uint8_t *b1 = buckets[i1].data;
    const uint8_t *b2 = buckets[i2].data;

    uint64_t val1 = *((uint64_t *)b1);
    uint64_t val2 = *((uint64_t *)b2);

    return bit_manager->hasvalue(val1, fp) || bit_manager->hasvalue(val2, fp);
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