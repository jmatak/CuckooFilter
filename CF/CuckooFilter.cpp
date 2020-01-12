#include <iostream>
#include "CuckooFilter.h"


template<typename element_type, typename fp_type>
CuckooFilter<element_type, fp_type>::CuckooFilter(uint32_t max_table_size,
                                                  size_t bits_per_fp,
                                                  size_t entries_per_bucket) {
    element_count = 0;
    victim = {0, 0};
    this->fp_mask = (1ULL << bits_per_fp) - 1;
    size_t table_size = highestPowerOfTwo(max_table_size);

    table = new CuckooTable<fp_type>(table_size, fp_mask);
    hash_function = new HashFunction();
}

template<typename element_type, typename fp_type>
size_t CuckooFilter<element_type, fp_type>::getIndex(uint32_t hv) const {
    // equivalent to modulo when number of buckets is a power of two
    return hv & (table->getTableSize() - 1);
}

template<typename element_type, typename fp_type>
uint32_t CuckooFilter<element_type, fp_type>::fingerprint(uint32_t hash_value) const {
    uint32_t fingerprint = hash_value & fp_mask;
    // make sure that fingerprint != 0
    fingerprint += (fingerprint == 0);
    return fingerprint;
}

template<typename element_type, typename fp_type>
inline void CuckooFilter<element_type, fp_type>::firstPass(const element_type &item, uint32_t *fp, size_t *index) const {
    const u_int64_t hash_value = hash_function->hash(item);

    *index = getIndex(hash_value >> 32);
    *fp = fingerprint(hash_value);
}

template<typename element_type, typename fp_type>
uint32_t CuckooFilter<element_type, fp_type>::indexComplement(const size_t index, const uint32_t fp) const {
    uint32_t hv = fingerprintComplement(index, fp);
    return getIndex(hv);
}

template<typename element_type, typename fp_type>
bool CuckooFilter<element_type, fp_type>::insert(uint32_t fp, size_t index) {
    size_t curr_index = index;
    uint32_t curr_fp = fp;
    uint32_t prev_fp;

    for (int kicks = 0; kicks < KICKS_MAX_COUNT; kicks++) {
        total_kicks++;
        bool eject = (kicks != 0);
        prev_fp = 0;
        if (table->replacingFingerprintInsertion(curr_index, curr_fp, eject, prev_fp)) {
            this->element_count++;
            return true;
        }
        if (eject) {
            curr_fp = prev_fp;
        }
        curr_index = indexComplement(curr_index, curr_fp);
    }

    victim.index = curr_index;
    victim.fp = curr_fp;
    return true;
}

template<typename element_type, typename fp_type>
bool CuckooFilter<element_type, fp_type>::insertElement(element_type &element) {
    size_t index;
    uint32_t fp;

    if (victim.fp) return false;

    firstPass(element, &fp, &index);
    return this->insert(fp, index);
}

template<typename element_type, typename fp_type>
bool CuckooFilter<element_type, fp_type>::deleteElement(const element_type &element) {
    uint32_t fp;
    size_t i1, i2;

    firstPass(element, &fp, &i1);
    // TODO: this could be calculated only if necessary
    i2 = indexComplement(i1, fp);

    if (table->deleteFingerprint(fp, i1) || table->deleteFingerprint(fp, i2)) {
        this->element_count--;
    } else if (victim.fp && fp == victim.fp &&
               (i1 == victim.index || i2 == victim.index)) {
        // element count -> upon agreement
        victim.fp = 0;
        return true;
    } else {
        return false;
    }

    if (victim.fp) {
        size_t index = victim.index;
        uint32_t fp = victim.fp;
        victim.fp = 0;
        // element count -> upon agreement
        this->insert(fp, index);
    }

    return true;
}

template<typename element_type, typename fp_type>
bool CuckooFilter<element_type, fp_type>::containsElement(element_type &element) {
    uint32_t fp;
    size_t i1, i2;

    firstPass(element, &fp, &i1);
    i2 = indexComplement(i1, fp);

    // TODO: remove after debugging
    assert(i1 == indexComplement(i2, fp));

    bool match = victim.fp && (fp == victim.fp) && (i1 == victim.index || i2 == victim.index);
    return match || table->containsFingerprint(i1, i2, fp);
}


template<typename element_type, typename fp_type>
void CuckooFilter<element_type, fp_type>::print() {
    table->printTable();
}


template<typename element_type, typename fp_type>
CuckooFilter<element_type, fp_type>::~CuckooFilter() {
    delete table;
    delete hash_function;
}

/**
 * Calculates the percentage of free space in the table that the filter uses.
 * @tparam element_type
 * @tparam fp_type
 * @return percentage of free space in the cuckoo filter's table
 */

template<typename element_type, typename fp_type>
double CuckooFilter<element_type, fp_type>::availability() {
    size_t free = this->table->getNumOfFreeEntries();
    size_t ts = this->table->maxNoOfElements();

    return (free / ((double) ts)) * 100.;
}


template<typename element_type, typename fp_type>
size_t CuckooFilter<element_type, fp_type>::getTableSize() {
    return this->table->getTableSize();
}

template
class CuckooFilter<std::string>;

template
class CuckooFilter<std::size_t, std::uint8_t>;

template
class CuckooFilter<std::size_t, std::uint16_t>;

template
class CuckooFilter<std::size_t, std::uint32_t>;

template
class CuckooFilter<std::string, std::uint16_t>;