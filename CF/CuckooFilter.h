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
    uint32_t fp;
    size_t index;
};


/**
 * /**
 *
 * A cuckoo filter is a space-efficient probabilistic data structure that is used to test whether an
 * element is a member of a set, like a Bloom filter does. False positive matches are possible, but
 * false negatives are not – in other words, a query returns either "possibly in set" or "definitely not
 * in set". Constructing Cuckoo Filter with specific table size, number of bits per fingerprint and number
 * of entries per bucket.
 *
 *
 * @tparam element_type Working element type
 * @tparam entries_per_bucket Number of entries in bucket
 * @tparam bits_per_fp  Number of bits in fingerprint
 * @tparam fp_type Fingerprint type
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
class CuckooFilter {
private:
    uint32_t fp_mask{};
    CuckooTable<entries_per_bucket, bits_per_fp, fp_type> *table;
    size_t element_count;
    HashFunction *hash_function;
    Victim victim;

    inline size_t getIndex(uint32_t hv) const;

    inline uint32_t fingerprint(uint32_t hash_value) const;

    inline void firstPass(const element_type &item, uint32_t *fp, size_t *index) const;

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const;

    bool insert(uint32_t fp, size_t index);


public:

    size_t total_kicks = 0;

    CuckooFilter(uint32_t max_table_size);

    ~CuckooFilter();

    void print();

    bool insertElement(element_type &element);

    bool deleteElement(const element_type &element);

    bool containsElement(element_type &element);

    double availability();

    size_t getTableSize();
};


/**
 *
 * A cuckoo filter is a space-efficient probabilistic data structure that is used to test whether an
 * element is a member of a set, like a Bloom filter does. False positive matches are possible, but
 * false negatives are not – in other words, a query returns either "possibly in set" or "definitely not
 * in set". Constructing Cuckoo Filter with specific table size, number of bits per fingerprint and number
 * of entries per bucket.
 *
 * @param max_table_size Maximum table size
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::CuckooFilter(uint32_t max_table_size) {
    element_count = 0;
    victim = {0, 0};
    this->fp_mask = (1ULL << bits_per_fp) - 1;
    size_t table_size = highestPowerOfTwo(max_table_size);

    table = new CuckooTable<entries_per_bucket, bits_per_fp, fp_type>(table_size, fp_mask);
    hash_function = new HashFunction();
}

/**
 * Gets index from previously calculated hash value.
 *
 * @param hash_value Hash value
 * @return Index out of hash value
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
size_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::getIndex(uint32_t hash_value) const {
    // equivalent to modulo when number of buckets is a power of two
    return hash_value & (table->getTableSize() - 1);
}

/**
 * Function for calculating fingerprint out of given hash value.
 *
 * @param hash_value Hash value
 * @return Fingerprint for saving from hash value
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
uint32_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::fingerprint(uint32_t hash_value) const {
    uint32_t fingerprint = hash_value & fp_mask;
    // make sure that fingerprint != 0
    fingerprint += (fingerprint == 0);
    return fingerprint;
}

/**
 * Method for calculating first index and fingerprint from element hash value.
 * Both arguments should be accessed by reference.
 *
 * @param item Item to store in filter
 * @param fp Fingerprint pointer
 * @param index Index pointer
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
inline void
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::firstPass(const element_type &item, uint32_t *fp,
                                                                                size_t *index) const {
    const u_int64_t hash_value = hash_function->hash(item);

    *index = getIndex(hash_value >> 32);
    *fp = fingerprint(hash_value);
}

/**
 * Calculating second index from previous index and calculated fingerprint
 *  $i2 = i1 \oplus hash(f)$\;
 *
 * @param index Previously calculated index
 * @param fp Element fingerprint
 * @return Secondary index calculated from fingerprint and previous index
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
uint32_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::indexComplement(const size_t index,
                                                                                               const uint32_t fp) const {
    uint32_t hv = fingerprintComplement(index, fp);
    return getIndex(hv);
}

/**
 * Insertion of fingerprint fp on position index. Maximum tries are defined with KICKS_MAX_COUNT
 * constant.
 *
 * @param fp Fingerprint for insertion
 * @param index Position for insertion
 * @return True if element is inserted, false otherwise, or if item is kicked
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::insert(uint32_t fp, size_t index) {
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

/**
 * Inserting element into Cuckoo Filter. In first pass, fingerprint and index are calculated,
 * proceeding with insertion with reallocation,
 *
 * @param element Element for insertion
 * @return True if element is inserted
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::insertElement(element_type &element) {
    size_t index;
    uint32_t fp;

    if (victim.fp) return false;

    firstPass(element, &fp, &index);
    return this->insert(fp, index);
}

/**
 *  Deleting element from Cuckoo Filter. Algorithm requires checking both primary and secondary index,
 *  if any of them contain fingerprint, it is removed from structure
 *
 * @param element Element for deletion
 * @return True if item is deleted
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::deleteElement(const element_type &element) {
    uint32_t fp;
    size_t i1, i2;

    firstPass(element, &fp, &i1);
    // TODO: this could be calculated only if necessary

    if (table->deleteFingerprint(fp, i1)) {
        this->element_count--;
    } else {
        i2 = indexComplement(i1, fp);
        if (table->deleteFingerprint(fp, i2)) {
            this->element_count--;
        } else if (victim.fp && fp == victim.fp &&
                   (i1 == victim.index || i2 == victim.index)) {
            // element count -> upon agreement
            victim.fp = 0;
            return true;
        } else {
            return false;
        }
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

/**
 *  Checking if element is contained in Cuckoo Filter. Algorithm requires checking both primary and secondary index,
 *  if any of them contain fingerprint, returns true.
 *
 * @param element Element for deletion
 * @return True if item is contained
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::containsElement(element_type &element) {
    uint32_t fp;
    size_t i1, i2;

    firstPass(element, &fp, &i1);
    if (table->containsFingerprint(i1, fp)) {
        return true;
    }

    i2 = indexComplement(i1, fp);

    return table->containsFingerprint(i2, fp) ||
           (victim.fp && (fp == victim.fp) && (i1 == victim.index || i2 == victim.index));
}


/**
 * Prints cuckoo filter table where buckets are written in hexadecimal.
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::print() {
    table->printTable();
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::~CuckooFilter() {
    delete table;
    delete hash_function;
}

/**
 * Calculates the percentage of free space in the table that the filter uses.
 * @tparam element_type
 * @tparam fp_type
 * @return percentage of free space in the cuckoo filter's table
 */

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
double CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::availability() {
    size_t free = this->table->getNumOfFreeEntries();
    size_t ts = this->table->maxNoOfElements();

    return (free / ((double) ts)) * 100.;
}


/**
 * Gets size of table from filter.
 *
 * @return Table size
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
size_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::getTableSize() {
    return this->table->getTableSize();
}

#endif