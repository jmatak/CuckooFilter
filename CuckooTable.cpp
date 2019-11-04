
#include "CuckooTable.h"

template<typename fp_type>
CuckooTable<fp_type>::CuckooTable(const size_t table_size) : table_size(table_size) {
    buckets = new Bucket[table_size];
    memset(buckets, 0, sizeof(fp_type) * table_size); // set all bits to 0
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
inline fp_type CuckooTable<fp_type>::getFingerprint(const size_t i, const size_t j) {
    const fp_type *bucket = buckets[i].data;
    fp_type fingerprint = bucket[j];
    return fingerprint;
}

template<typename fp_type>
size_t CuckooTable<fp_type>::fingerprintCount(const size_t i) const {
    size_t count = 0;
    for (size_t j = 0; j < entries_per_bucket; ++j) {
        if (getFingerprint(i, j) != 0) {
            count++;
        }
    }
    return count;
}

template<typename fp_type>
void CuckooTable<fp_type>::insertFingerprint(const size_t i, const size_t j, const fp_type fp) {
    fp_type *bucket = buckets[i].data;
    bucket[j] = fp;
}

template<typename fp_type>
inline bool CuckooTable<fp_type>::replacingFingerprintInsertion(const size_t i, const fp_type fp,
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

template<typename fp_type>
bool CuckooTable<fp_type>::containsFingerprint(const size_t i, const fp_type fp) {
    for (size_t j = 0; j < entries_per_bucket; ++j) {
        if (getFingerprint(i, j) == fp) {
            return true;
        }
    }
    return false;
}

template<typename fp_type>
bool CuckooTable<fp_type>::deleteFingerprint(const size_t i, const fp_type fp) {
    for (size_t j = 0; j < entries_per_bucket; ++j) {
        if (getFingerprint(i, j) == fp) {
            insertFingerprint(i, j, 0);
            return true;
        }
    }
    return false;
}
