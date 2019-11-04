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
    explicit CuckooTable(size_t table_size);

    ~CuckooTable();

    size_t getTableSize() const;

    size_t maxNoOfElements();

    inline fp_type getFingerprint(size_t i, size_t j);

    size_t fingerprintCount(size_t i) const;

    inline void insertFingerprint(size_t i, size_t j, fp_type fp);

    inline bool replacingFingerprintInsertion(size_t i, fp_type fp, bool eject, fp_type &prev_fp);

    bool containsFingerprint(size_t i, fp_type fp);

    bool deleteFingerprint(size_t i, fp_type fp);
};


#endif

