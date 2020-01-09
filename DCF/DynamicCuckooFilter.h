#ifndef CUCKOOFILTER_DYNAMICCUCKOOFILTER_H
#define CUCKOOFILTER_DYNAMICCUCKOOFILTER_H

#include "CuckooFilter.h"

template<typename element_type = uint32_t, typename fp_type = uint8_t>
class DynamicCuckooFilter {

private:
    size_t bits_per_fp;
    size_t entries_per_bucket;

    size_t element_count;
    size_t cf_count;

    int cf_table_size;

    HashFunction *hashFunction;

    Victim victim;

    CuckooFilter<element_type, fp_type>* head_cf;
    CuckooFilter<element_type, fp_type>* tail_cf;
    CuckooFilter<element_type, fp_type>* active_cf;

public:
    DynamicCuckooFilter(uint32_t max_table_size, size_t bits_per_fp = 8, size_t entries_per_bucket = 4);

    ~DynamicCuckooFilter();

    CuckooFilter<element_type, fp_type>* nextCF(CuckooFilter<element_type, fp_type>* cf);

    void storeVictim(Victim &victim);

    bool insertElement(element_type element);

    bool containsElement(element_type element);

    bool deleteElement(element_type element);

    void removeCF(CuckooFilter<element_type, fp_type>* cf);

    void compact();

    void sort(CuckooFilter<element_type, fp_type>** cfq, int count);

    uint32_t indexComplement(const size_t index, const uint32_t fp);

    void print();

};




#endif //CUCKOOFILTER_DYNAMICCUCKOOFILTER_H
