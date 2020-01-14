#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <iostream>

#include "../Utils/bit_manager.h"


template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
class CuckooTable {

private:
    static const size_t bytes_per_bucket = (entries_per_bucket * bits_per_fp) / 8;
    // manipulation with bits
    BitManager<fp_type>* bit_manager;

    struct Bucket {
        uint8_t data[bytes_per_bucket];
    };

    // element storage
    Bucket* buckets;

public:
    // number of buckets
    size_t table_size;
    // mask for extracting lower bits
    uint32_t fp_mask;

    /**
     *
     * @param table_size
     * @param bit_manager
     * @param fp_mask
     */
    CuckooTable(size_t table_size, BitManager<fp_type>* bit_manager, uint32_t fp_mask);

    /**
     * Deleting all entries from cuckoo table and deleting bit manager.
     */
    ~CuckooTable();

    /**
     * Retrieves table size.
     * @return size of the table
     */
    size_t getTableSize() const;

    /**
      * Returning maximum number of elements stored in table.
      *
      * @return Maximum possible number of elements
      */
    size_t maxNoOfElements();

    /**
     *  Gets fingerprint in bucket i with entry position j
     *
     * @param i Index of bucket which stores element
     * @param j Index of entry in bucket
     * @return Fingerprint on position (i,j)
     */
    inline uint32_t getFingerprint(size_t i, size_t j);

    /**
     * Count of stored fingerprints in bucket.
     *
     * @param i Bucket index
     * @return Number of fingerprints stored in one bucket
     */
    size_t fingerprintCount(size_t i) const;

    /**
     * Inserting fingerprint in bucket with index i and entry index j.
     *
     * @param i  Index of bucket
     * @param j  Entry index
     * @param fp Fingerprint for insertion
     */
    inline void insertFingerprint(size_t i, size_t j, uint32_t fp);


    /**
     * Inserting fingerprint in bucket with index i and entry index j,
     * but checks if spot is empty before insertion.
     * Used for transfering elements from one filter to another.
     *
     * @param i  Index of bucket
     * @param j  Entry index
     * @param fp Fingerprint for insertion
     */
    inline bool insertFingerprintIfEmpty(const size_t i, const size_t j, const uint32_t fp);

    /**
     * Method for inserting element in table. If another element is being replaced, his fingerprint is stored.
     *
     * @param i Bucket index
     * @param fp Fingerprint for storing
     * @param eject True if element is being ejected from table
     * @param prev_fp Fingerprint of element being replaced
     * @return True if element is inserted without replacing
     */
    inline bool replacingFingerprintInsertion(size_t i, uint32_t j, bool eject, uint32_t& prev_fp);

    /**
     * Checking if bucket i contains fingerprint fp
     *
     * @param i Bucket index
     * @param fp Fingerprint for checking
     * @return  True if fingerprint is contained
     */
    bool containsFingerprint(size_t i, uint32_t fp);

    /**
     * Checking if fingerprint is contained in bucket with index i1 and i2.
     *
     * @param i1 First checking index
     * @param i2 Second checking index
     * @param fp Fingerprint to check
     * @return True if element is contained
     */
    bool containsFingerprint(size_t i1, size_t i2, uint32_t fp);

    /**
     * Deleting fingerprint from table. If fingerprint is not presented in certain bucket, returning false.
     *
     * @param fp  Fingerprint for deletion
     * @param i Index of bucket where fingerprint is stored.
     * @return True if element is deleted
     */
    bool deleteFingerprint(const size_t, const uint32_t);

    /**
     * Deleting fingerprint from table with both indices precomputed.
     *
     * @param i1
     * @param i2
     * @param fp
     * @return True if element is deleted
     */
    bool deleteFingerprint(const size_t i1, const size_t i2, const uint32_t fp);


};

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
CuckooTable(size_t table_size, BitManager<fp_type>* bit_manager, uint32_t fp_mask) {
    this->table_size = table_size;
    this->bit_manager = bit_manager;
    this->fp_mask = fp_mask;

    buckets = new Bucket[table_size];
    memset(buckets, 0, bytes_per_bucket * table_size); // set all bits to 0

   /*   // when buckets are allocated on heap
    *
    *   for(size_t i = 0; i < table_size; i++){
        buckets[i].data = new uint8_t[entries_per_bucket];
        memset(buckets[i].data, 0, bytes_per_bucket);
    }*/
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::~CuckooTable() {
    delete[] buckets;
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
size_t CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::getTableSize() const {
    return table_size;
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
size_t CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::maxNoOfElements() {
    return entries_per_bucket * table_size;
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
inline uint32_t CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
getFingerprint(const size_t i, const size_t j) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t fp = bit_manager->read(j, bucket);
    return fp & fp_mask;
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
size_t CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
fingerprintCount(const size_t i) const {
    size_t count = 0;
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) != 0) {
            count++;
        }
    }
    return count;
}


template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
bool CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
insertFingerprintIfEmpty(const size_t i, const size_t j, const uint32_t fp) {
    if (getFingerprint(i, j) == 0) {
        const uint8_t *bucket = buckets[i].data;
        uint32_t efp = fp & fp_mask;
        bit_manager->write(j, bucket, efp);
        return true;
    } else {
        return false;
    }
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
void CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
insertFingerprint(const size_t i, const size_t j, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    uint32_t efp = fp & fp_mask;
    bit_manager->write(j, bucket, efp);
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
inline bool CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
replacingFingerprintInsertion(const size_t i, const uint32_t fp,
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


template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
bool CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
containsFingerprint(const size_t i, const uint32_t fp) {
    const uint8_t *bucket = buckets[i].data;
    uint64_t val = *((uint64_t *)bucket);

    return bit_manager->hasvalue(val, fp);
}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
bool CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
containsFingerprint(const size_t i1, const size_t i2, const uint32_t fp) {
    return
            bit_manager->hasvalue(
                    *((uint64_t *)buckets[i1].data), fp)
            ||
            bit_manager->hasvalue(
                    *((uint64_t *)buckets[i2].data), fp);

}

template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
bool CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
deleteFingerprint(const size_t i, const uint32_t fp) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i, j) == fp) {
            insertFingerprint(i, j, 0);
            return true;
        }
    }
    return false;
}


template<typename fp_type, size_t entries_per_bucket, size_t bits_per_fp>
bool CuckooTable<fp_type, entries_per_bucket, bits_per_fp>::
deleteFingerprint(const size_t i1, const size_t i2, const uint32_t fp) {
    for (size_t j = 0; j < entries_per_bucket; j++) {
        if (getFingerprint(i1, j) == fp) {
            insertFingerprint(i1, j, 0);
            return true;
        }
        if (getFingerprint(i2, j) == fp) {
            insertFingerprint(i2, j, 0);
            return true;
        }
    }
    return false;
}