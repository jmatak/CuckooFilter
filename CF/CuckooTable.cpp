#include <string.h>
#include <stdint.h>

// http://www-graphics.stanford.edu/~seander/bithacks.html
#define zero8bit(x) (((x)-0x01010101ULL) & (~(x)) & 0x80808080ULL)
#define true8bit(x, n) (zero8bit((x) ^ (~0UL/255 * (n))))


template<size_t bits_per_fp = 8>
class CuckooTable {

    static const size_t entries_per_bucket = 4;
    static const size_t bytes_per_bucket = entries_per_bucket * bits_per_fp / 8;
    static const uint32_t fp_mask = (1ULL << bits_per_fp) - 1;

    struct Bucket {
        // TODO: generic type, bits & entries per bucket
        uint8_t data[entries_per_bucket];
    };

private:
    const size_t table_size;
    Bucket *buckets;

public:
    explicit CuckooTable(size_t table_size);
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


template<size_t bits_per_fp>
CuckooTable<bits_per_fp>::CuckooTable(const size_t table_size) : table_size(table_size) {
    buckets = new Bucket[table_size];
    // TODO: generic number of bits per bucket
    memset(buckets, 0, CuckooTable::bytes_per_bucket * table_size); // set all bits to 0
}


template<size_t bits_per_fp>
CuckooTable<bits_per_fp>::~CuckooTable() {
    delete[] buckets;
}


template<size_t bits_per_fp>
size_t CuckooTable<bits_per_fp>::getTableSize() const {
    return table_size;
}

template<size_t bits_per_fp>
size_t CuckooTable<bits_per_fp>::maxNoOfElements() {
    return entries_per_bucket * table_size;
}


template<size_t bits_per_fp>
inline uint32_t CuckooTable<bits_per_fp>::getFingerprint(const size_t i, const size_t j) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t fp;
    bucket += j;
    // TODO: this is only for 8 bits per fingerprint, extend to other settings
    fp = *((uint8_t *)bucket);
    return fp & fp_mask;
}


template<size_t bits_per_fp>
size_t CuckooTable<bits_per_fp>::fingerprintCount(const size_t i) const {
    size_t count = 0;
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) != 0) {
            count++;
        }
    }
    return count;
}


template<size_t bits_per_fp>
void CuckooTable<bits_per_fp>::insertFingerprint(const size_t i, const size_t j, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    // extracted fingerprint
    uint32_t efp = fp & fp_mask;
    // TODO: this is only for 8 bits per fingerprint, extend to other settings
    ((uint8_t *)bucket)[j] = efp;
}


template<size_t bits_per_fp>
inline bool CuckooTable<bits_per_fp>::replacingFingerprintInsertion(const size_t i, const uint32_t fp,
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


template<size_t bits_per_fp>
bool CuckooTable<bits_per_fp>::containsFingerprint(const size_t i, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;

    uint32_t val = *((uint32_t *)bucket);

    // entries_per_bucket = 4 & bits_per_fp = 8
    return true8bit(val, fp);


/*    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) == fp) {
            return true;
        }
    }
    return false;*/
}


template<size_t bits_per_fp>
bool CuckooTable<bits_per_fp>::containsFingerprint(const size_t i1, const size_t i2, const uint32_t fp) {
    const uint8_t *b1 = buckets[i1].data;
    const uint8_t *b2 = buckets[i2].data;

    // TODO: change to 32
    uint32_t val1 = *((uint32_t *)b1);
    uint32_t val2 = *((uint32_t *)b2);

    // entries_per_bucket = 4 & bits_per_fp = 8
    return true8bit(val1, fp) || true8bit(val2, fp);


   /* for (size_t j = 0; j < entries_per_bucket; j++) {
        if ((getFingerprint(i1, j) == fp) || (getFingerprint(i2, j) == fp)) {
            return true;
        }
    }
    return false;*/
}


template<size_t bits_per_fp>
bool CuckooTable<bits_per_fp>::deleteFingerprint(const uint32_t fp, const size_t i) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) == fp) {
            insertFingerprint(i, j, 0);
            return true;
        }
    }
    return false;
}