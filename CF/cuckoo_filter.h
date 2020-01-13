#include <type_traits>
#include "cuckoo_table.h"
#include "../Utils/hash_function.h"
#include "../Utils/util.h"

#define KICKS_MAX_COUNT 500


/**
 *
 * Cuckoo filter is a space-efficient probabilistic data structure that is used to test whether an
 * element is a member of a set, like a Bloom filter does. False positive matches are possible, but
 * false negatives are not – in other words, a query returns either "possibly in set" or "definitely not
 * in set". Constructing Cuckoo Filter with specific table size, number of bits per fingerprint and number
 * of entries per bucket.
 *
 * @tparam element_type Working element type
 * @tparam entries_per_bucket Number of entries in bucket
 * @tparam bits_per_fp  Number of bits in fingerprint
 * @tparam fp_type Fingerprint type
 */
template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
class CuckooFilter {

private:
    // mask for extracting lower bits
    uint32_t fp_mask_;

    // table for storing elements' fingerprints
    CuckooTable<entries_per_bucket, bits_per_fp, fp_type> *table_;

    // number of stored elements
    size_t element_count_;

    // used for calculating hash values
    HashFunction *hash_function_;

    // helper structure
    Victim victim_;

    /**
     * Gets index from previously calculated hash value.
     *
     * @param hash_value Hash value
     * @return Index out of hash value
     */
    inline size_t getIndex(uint32_t hv) const;

    /**
     * Function for calculating fingerprint out of given hash value.
     *
     * @param hash_value Hash value
     * @return Fingerprint for saving from hash value
     */
    inline uint32_t fingerprint(uint32_t hash_value) const;

    /**
     * Method for calculating first index and fingerprint from element hash value.
     * Both arguments should be accessed by reference.
     *
     * @param item Item to store in filter
     * @param fp Fingerprint pointer
     * @param index Index pointer
     */
    inline void firstPass(const element_type &item, uint32_t *fp, size_t *index) const;

    /**
     * Calculating second index from previous index and calculated fingerprint
     *  $i2 = i1 \oplus hash(f)$\;
     *
     * @param index Previously calculated index
     * @param fp Element fingerprint
     * @return Secondary index calculated from fingerprint and previous index
     */
    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const;

    /**
     * Insertion of fingerprint fp on position index. Maximum tries are defined with KICKS_MAX_COUNT
     * constant.
     *
     * @param fp Fingerprint for insertion
     * @param index Position for insertion
     * @return True if element is inserted, false otherwise, or if item is kicked
     */
    bool insert(uint32_t fp, size_t index);

public:

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
    CuckooFilter(uint32_t max_table_size);

    /**
     * Destructor that is in charge of memory clean-up.
     */
    ~CuckooFilter();

    /**
     * Prints cuckoo table with fingerprints of all elements in hexadecimal format.
     */
    void print();

    /**
     * Inserting element into Cuckoo Filter. In first pass, fingerprint and index are calculated,
     * proceeding with insertion with reallocation.
     *
     * @param element Element for insertion
     * @return True if element is inserted
     */
    bool insertElement(element_type &element);

    /**
     *  Deleting element from Cuckoo Filter. Algorithm requires checking both primary and secondary index,
     *  if any of them contain fingerprint, it is removed from structure.
     *
     * @param element Element for deletion
     * @return True if item is deleted
     */
    bool deleteElement(const element_type &element);

    /**
     *  Checking if element is contained in Cuckoo Filter. Algorithm requires checking both primary and secondary index,
     *  if any of them contain fingerprint, returns true.
     *
     * @param element Element for deletion
     * @return True if item is contained
     */
    bool containsElement(element_type &element);

    /**
     * Calculates the percentage of free space in the table that the filter uses.
     * @tparam element_type
     * @tparam fp_type
     * @return percentage of free space in the cuckoo filter's table
     */
    double availability();

    /**
     * Retrieves total number of buckets in the table.
     * @return table size
     */
    size_t getTableSize();
};



template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::CuckooFilter(uint32_t max_table_size) {
    element_count_ = 0;
    this->fp_mask_ = (1ULL << bits_per_fp) - 1;
    size_t table_size = highestPowerOfTwo(max_table_size);

    table_ = new CuckooTable<entries_per_bucket, bits_per_fp, fp_type>(table_size, fp_mask_);
    hash_function_ = new HashFunction();
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
size_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::getIndex(uint32_t hash_value) const {
    // equivalent to modulo when number of buckets is a power of two
    return hash_value & (table_->getTableSize() - 1);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
uint32_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::fingerprint(uint32_t hash_value) const {
    uint32_t fingerprint = hash_value & fp_mask_;
    // make sure that fingerprint != 0
    fingerprint += (fingerprint == 0);
    return fingerprint;
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
inline void
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
firstPass(const element_type &item, uint32_t *fp, size_t *index) const {
    const u_int64_t hash_value = hash_function_->hash(item);
    *index = getIndex(hash_value >> 32);
    *fp = fingerprint(hash_value);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
uint32_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
indexComplement(const size_t index, const uint32_t fp) const {
    uint32_t hv = fingerprintComplement(index, fp);
    return getIndex(hv);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
insert(uint32_t fp, size_t index) {

    size_t curr_index = index;
    uint32_t curr_fp = fp;
    uint32_t prev_fp;

    for (int kicks = 0; kicks < KICKS_MAX_COUNT; kicks++) {
        bool eject = (kicks != 0);
        prev_fp = 0;
        if (table_->replacingFingerprintInsertion(curr_index, curr_fp, eject, prev_fp)) {
            this->element_count_++;
            return true;
        }
        if (eject) {
            curr_fp = prev_fp;
        }
        curr_index = indexComplement(curr_index, curr_fp);
    }

    victim_.index = curr_index;
    victim_.fp = curr_fp;
    return true;
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
insertElement(element_type &element) {
    size_t index;
    uint32_t fp;

    if (victim_.fp) return false;

    firstPass(element, &fp, &index);
    return this->insert(fp, index);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
deleteElement(const element_type &element) {
    uint32_t fp;
    size_t i1, i2;

    firstPass(element, &fp, &i1);

    if (table_->deleteFingerprint(fp, i1)) {
        this->element_count_--;
    } else {
        i2 = indexComplement(i1, fp);
        if (table_->deleteFingerprint(fp, i2)) {
            this->element_count_--;
        } else if (victim_.fp && fp == victim_.fp &&
                   (i1 == victim_.index || i2 == victim_.index)) {
            // element count remains unmodified, victim is not regarded as a part of the table
            victim_.fp = 0;
            return true;
        } else {
            return false;
        }
    }

    if (victim_.fp) {
        size_t index = victim_.index;
        uint32_t fp = victim_.fp;
        victim_.fp = 0;
        this->insert(fp, index);
    }

    return true;
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
containsElement(element_type &element) {
    uint32_t fp;
    size_t i1, i2;

    firstPass(element, &fp, &i1);
    if (table_->containsFingerprint(i1, fp)) {
        return true;
    }

    i2 = indexComplement(i1, fp);

    return table_->containsFingerprint(i2, fp) ||
           (victim_.fp && (fp == victim_.fp) && (i1 == victim_.index || i2 == victim_.index));
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::print() {
    table_->printTable();
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::~CuckooFilter() {
    delete table_;
    delete hash_function_;
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
double CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::availability() {
    size_t free = this->table_->getNumOfFreeEntries();
    size_t ts = this->table_->maxNoOfElements();
    return (free / ((double) ts)) * 100.;
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
size_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::getTableSize() {
    return this->table_->getTableSize();
}
