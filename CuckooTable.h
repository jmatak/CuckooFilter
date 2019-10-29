#ifndef BIOINF_CUCKOOTABLE_H
#define BIOINF_CUCKOOTABLE_H

#include <stdlib.h>


template<typename fp_type>
class CuckooTable {

    static const size_t entries_per_bucket = 2;

    struct Bucket {
        fp_type data[entries_per_bucket];
    };

private:
    const size_t table_size;
    Bucket *buckets;

public:
    CuckooTable(const size_t table_size) : table_size(table_size) {
        buckets = new Bucket[table_size];
        memset(buckets, 0, sizeof(fp_type)*table_size); // set all bits to 0
    }

    ~CuckooTable() {
        delete[] buckets;
    }

    size_t getTableSize() const {
        return table_size;
    }

    size_t maxNoOfElements() {
        return entries_per_bucket*table_size;
    }

    inline fp_type getFingerprint(const size_t i, const size_t j) {
        const fp_type *bucket = buckets[i].data;
        fp_type fingerprint = bucket[j];
        return fingerprint;
    }

    size_t fingerprintCount(const size_t i) const {
        size_t count = 0;
        for (size_t j = 0; j < entries_per_bucket; ++j) {
            if (getFingerprint(i, j) != 0) {
                count++;
            }
        }
        return count;
    }

    inline void insertFingerprint(const size_t i, const size_t j, const fp_type fp) {
        fp_type *bucket = buckets[i].data;
        bucket[j] = fp;
    }

    inline bool replacingFingerprintInsertion(const size_t i, const fp_type fp,
                                              const bool eject, fp_type &prev_fp) {
        for (size_t j = 0; j < entries_per_bucket; ++j) {
            if (getFingerprint(i, j) == 0) {
                insertFingerprint(i, j, fp);
                return true;
            }
        }

        if (eject) {
            size_t next = rand() % entries_per_bucket;
            prev_fp = getFingerprint(i, next);
        }
        return false;
    }

    bool containsFingerprint(const size_t i, const fp_type fp) {
        for (size_t j = 0; j < entries_per_bucket; ++j) {
            if (getFingerprint(i, j) == fp) {
                return true;
            }
        }
        return false;
    }

    bool deleteFingerprint(const size_t i, const fp_type fp) {
        for (size_t j = 0; j < entries_per_bucket; ++j) {
            if (getFingerprint(i, j) == fp) {
                insertFingerprint(i, j, 0);
                return true;
            }
        }
        return false;
    }

};


#endif

