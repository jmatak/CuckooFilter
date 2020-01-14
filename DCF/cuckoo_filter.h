#include "cuckoo_table.h"

#define kicks_max_count 500

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
    // table for storing elements' fingerprints
    CuckooTable<fp_type, entries_per_bucket, bits_per_fp>* table;
    // capacity for the filter, if it is exceeded, the filter is regarded as full
    size_t capacity;

    /**
     * Gets index from previously calculated hash value.
     *
     * @param hash_value Hash value
     * @return Index out of hash value
     */
    inline size_t getIndex(uint32_t hv) const;

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
     * Refreshes attributes that count total number of elements
     * and checks if filter is empty.
     */
    inline void refreshOnDelete();

    /**
     * Refreshes attributes that count total number of elements
     * and checks if filter is full.
     */
    inline void refreshOnInsert();

public:

    // pointer to previous cuckoo filter in linked list
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* prev = NULL;
    // pointer to next cuckoo filter in linked list
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* next = NULL;

    // true if filter is full, false otherwise
    bool is_full = false;
    // true if filter is empty, false otherwise
    bool is_empty = true;

    // number of stored elements
    size_t element_count;

    /**
     * A cuckoo filter is a space-efficient probabilistic data structure that is used to test whether an
     * element is a member of a set, like a Bloom filter does. False positive matches are possible, but
     * false negatives are not – in other words, a query returns either "possibly in set" or "definitely not
     * in set". Constructing Cuckoo Filter with specific table size, number of bits per fingerprint and number
     * of entries per bucket.
     *
     * @param max_table_size Maximum table size
     */
    explicit CuckooFilter(uint32_t table_size,
                          BitManager<fp_type>* bit_manager,
                          uint32_t fp_mask);

    /**
     * Inserting element into Cuckoo Filter. In first pass, fingerprint and index are calculated,
     * proceeding with insertion with reallocation.
     *
     * @param fp
     * @param index
     * @param victim
     * @return true if insertion is successfull, false otherwise
     */
    bool insertElement(uint32_t fp, size_t index, Victim &victim);

    /**
     * Checks if element is at given index i1 and deletes it if it is true.
     * To boost efficiency, it calculates the second index only if the
     * first one is not appropriate for the given fingerprint.
     *
     * @param i1
     * @param fp
     * @return true if deletion is successfull, false otherwise
     */
    bool deleteElement(size_t i1, uint32_t fp);

    /**
     * Deletes fingerprint at indices i1 or i2.
     *
     * @param i1
     * @param i2
     * @param fp
     * @return true if deletion is successfull, false otherwise
     */
    bool deleteElement(size_t i1, size_t i2, uint32_t fp);

    /**
     * Checks if element is at given index i1.
     * To boost efficiency, it calculates the second index only if the
     * first one is not appropriate for the given fingerprint.
     *
     * @param i1
     * @param fp
     * @return true if filter contains provided fingerprint
     */
    bool containsElement(size_t i1, uint32_t fp);

    /**
     * Checks if element is at given indices i1 or i2.
     *
     * @param i1
     * @param i2
     * @param fp
     * @return true if filter contains provided fingerprint
     */
    bool containsElement(size_t i1, size_t i2, uint32_t fp);

    /**
     * Delegation function.
     * Calls table's function replacementInsert with same parameters.
     *
     * @param i
     * @param j
     * @param fp
     */
    void replacementInsert(size_t i, size_t j, uint32_t fp);

    /**
     * Attempts to transfer elements of this cuckoo filter to
     * the table of cuckoo filter provided as argument.
     * 
     * @param cf
     */
    void moveElements(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf);

    /**
     * Destructor that is in charge of memory clean-up.
     */
    ~CuckooFilter();
};


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
size_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
getIndex(uint32_t hv) const {
    // equivalent to modulo when number of buckets is a power of two
    return hv & (table->table_size - 1);
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
inline uint32_t CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
indexComplement(const size_t index, const uint32_t fp) const {
    uint32_t hv = fingerprintComplement(index, fp);
    return getIndex(hv);
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
inline void CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
refreshOnDelete() {
    this->element_count--;
    if (this->element_count < this->capacity) {
        this->is_full = false;
    }
    if (this->element_count == 0) {
        this->is_empty = true;
    }
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
inline void CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
refreshOnInsert() {
    this->element_count++;
    if (this->element_count == this->capacity) {
        this->is_full = true;
    }
    if (this->element_count > 0) {
        this->is_empty = false;
    }
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
CuckooFilter(uint32_t table_size, BitManager<fp_type>* bit_manager, uint32_t fp_mask) {
    // TODO: capacity?
    capacity = size_t(0.9 * table_size);
    element_count = 0;
    table = new CuckooTable<fp_type, entries_per_bucket, bits_per_fp>(table_size, bit_manager, fp_mask);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
insertElement(uint32_t fp, size_t index, Victim &victim) {
    size_t curr_index = index;
    uint32_t curr_fp = fp;
    uint32_t prev_fp;

    for (int kicks = 0; kicks < kicks_max_count; kicks++) {
        bool eject = (kicks != 0);
        prev_fp = 0;
        if (table->replacingFingerprintInsertion(curr_index, curr_fp, eject, prev_fp)) {
            this->refreshOnInsert();
            return true;
        }
        if (eject) {
            curr_fp = prev_fp;
        }
        curr_index = indexComplement(curr_index, curr_fp);
    }

    victim.index = curr_index;
    victim.fp = curr_fp;
    return false;
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
deleteElement(size_t i1, uint32_t fp) {
    bool deletion =
            table->deleteFingerprint(i1, fp)
            ||
            table->deleteFingerprint(indexComplement(i1, fp), fp);

    if (deletion) {
        this->refreshOnDelete();
    }

    return deletion;
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
deleteElement(size_t i1, size_t i2, uint32_t fp) {
    bool deletion =
            table->deleteFingerprint(i1, fp)
            ||
            table->deleteFingerprint(i2, fp);

    if (deletion) {
        this->refreshOnDelete();
    }

    return deletion;
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
containsElement(size_t i1, uint32_t fp) {
    size_t i2 = indexComplement(i1, fp);
    return table->containsFingerprint(i1, i2, fp);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
containsElement(size_t i1, size_t i2, uint32_t fp) {
    return table->containsFingerprint(i1, i2, fp);
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
replacementInsert(size_t i, size_t j, uint32_t fp) {
    table->insertFingerprint(i, j, fp);
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
void CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
moveElements(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
    uint32_t fp;

    for(size_t i = 0; i < table->table_size; i++){
        for(int j = 0; j < table->entries_per_bucket; j++){
            fp = table->getFingerprint(i, j);

            if(fp){
                if(cf->is_full || this->is_empty) return;
                if(cf->table->insertFingerprintIfEmpty(i, j, fp)){
                    table->insertFingerprint(i, j, 0);
                    this->refreshOnDelete();
                    cf->refreshOnInsert();
                }
            }
        }
    }
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::~CuckooFilter() {
    delete table;
}