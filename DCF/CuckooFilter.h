#ifndef CUCKOOFILTER_CUCKOOFILTER_H
#define CUCKOOFILTER_CUCKOOFILTER_H

#include <type_traits>
#include "CuckooTable.h"
#include "HashFunction.h"

#define KICKS_MAX_COUNT 500

static size_t highestPowerOfTwo(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    v >>= 1;
    return v;
}

struct Victim {
    uint32_t fp = 0;
    size_t index = 0;
};

template<typename element_type, typename fp_type = uint8_t>
class CuckooFilter {
private:
    uint32_t fp_mask{};
    CuckooTable<fp_type> *table;
    HashFunction *hash_function;
    size_t capacity;

    inline size_t getIndex(uint32_t hv) const;

    inline uint32_t fingerprint(uint32_t hash_value) const;

    inline void firstPass(const element_type &item, uint32_t *fp, size_t *index) const;

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const;

    void refreshOnDelete();

    void refreshOnInsert();

public:

    Victim victim;

    size_t element_count;
    CuckooFilter<element_type, fp_type> *prev = NULL;

    CuckooFilter<element_type, fp_type> *next = NULL;
    bool is_full = false;

    bool is_empty = true;

    CuckooFilter(uint32_t max_table_size, size_t bits_per_fp = 8, size_t entries_per_bucket = 4, HashFunction *hashFunction= nullptr);

    ~CuckooFilter();

    void print();

    bool insertElement(element_type &element);

    bool insert(uint32_t fp, size_t index);

    bool deleteElement(const element_type &element);

    bool containsElement(element_type &element);


    double availability();

    void moveElements(CuckooFilter<element_type, fp_type> *cf);


};


#endif