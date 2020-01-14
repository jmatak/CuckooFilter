#include <stdint.h>
#include "../Utils/hash_function.h"
#include "../Utils/util.h"
#include "cuckoo_filter.h"

/**
 *
 * Dynamic Cuckoo Filter is a space-efficient probabilistic data structure that is used to test whether an
 * element is a member of a set, like a Bloom filter does. It uses additional structures for dynamic
 * surroundings. Basically, it keeps multiple cuckoo filter in a linked list.
 * False positive matches are possible, but false negatives are not – in other words,
 * a query returns either "possibly in set" or "definitely not
 * in set". Constructing Cuckoo Filter with specific table size, number of bits per fingerprint and number
 * of entries per bucket.
 *
 * @tparam element_type Working element type
 * @tparam entries_per_bucket Number of entries in bucket
 * @tparam bits_per_fp  Number of bits in fingerprint
 * @tparam fp_type Fingerprint type
 */
template<typename element_type=uint32_t,
         size_t entries_per_bucket=4,
         size_t bits_per_fp=8,
         typename fp_type=uint8_t>
class DynamicCuckooFilter{

private:
    // used for computing hash values
    HashFunction* hash_function_;

    // operations with bits
    BitManager<fp_type>* bit_manager_;

    // mask for extracting lower bits
    uint32_t fp_mask_;

    // table size per one cuckoo filter
    int cf_table_size_;

    // helper structure
    Victim victim_;

    // head cuckoo filter in linked list
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* head_cf_;
    // tail cuckoo filter in linked list
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* tail_cf_;
    // active cuckoo filter in linked list
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* active_cf_;

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
     * Sorts array of cuckoo filters that are not completely full in the
     * descending order regarding the number of elements stored in single
     * cuckoo table.
     *
     * @param cfq
     * @param count
     */
    void sort(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>** cfq,
              int count);

    /**
     * Removes cuckoo filter from linked list. Memory that this single cuckoo
     * filter occupied is now cleared.
     *
     * @param cf
     */
    void removeCF(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf);


public:
    // total number of stored elements
    size_t element_count;
    // number of cuckoo filters
    size_t cf_count;

    /**
      *
      * Dynamic Cuckoo Filter is a space-efficient probabilistic data structure that is used to test whether an
      * element is a member of a set, like a Bloom filter does. It uses additional structures for dynamic
      * surroundings. Basically, it keeps multiple cuckoo filter in a linked list.
      * False positive matches are possible, but false negatives are not – in other words,
      * a query returns either "possibly in set" or "definitely not
      * in set". Constructing Cuckoo Filter with specific table size, number of bits per fingerprint and number
      * of entries per bucket.
      *
      * @param max_table_size Maximum table size
      */
    DynamicCuckooFilter(uint32_t max_table_size);

    /**
     * Destructor that is in charge of memory clean-up.
     */
    ~DynamicCuckooFilter();

    /**
     * Retrieves single cuckoo filter that comes next
     * after given one in the stored linked list.
     *
     * @param cf
     * @return next cuckoo filter in linked list
     */
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>*
            nextCF(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf);

    /**
     * Attempt to store element cached in "victim" structure to one of cuckoo filters'
     * actual table.
     *
     * @param victim
     * @param cf
     */
    void storeVictim(Victim &victim,
                     CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf);

    /**
      * Inserting element into Dynamic Cuckoo Filter. In first pass, fingerprint and index are calculated,
      * proceeding with insertion with reallocation.
      *
      * @param element Element for insertion
      * @return True if element is inserted
      */
    bool insertElement(const element_type &element);

    /**
      *  Checking if element is contained in Dynamic Cuckoo Filter.
      *  Algorithm requires checking both primary and secondary index,
      *  if any of them contain fingerprint, returns true.
      *
      * @param element Element for deletion
      * @return True if item is contained
      */
    bool containsElement(const element_type &element);

    /**
     *  Deleting element from Cuckoo Filter. Algorithm requires checking both primary and secondary index,
     *  if any of them contain fingerprint, it is removed from structure.
     *
     * @param element Element for deletion
     * @return True if item is deleted
     */
    bool deleteElement(const element_type &element);

    /**
     * Tries to transfer elements from sparse cuckoo filters to almost full ones.
     * The ultimate goal is to clean very sparse filters to reduce total
     * number of cuckoo filters.
     * This method should be called by user every once in a while when inserting
     * a lot of elements.
     */
    void compact();

};


template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
inline size_t DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
getIndex(uint32_t hv) const {
    // equivalent to modulo when number of buckets is a power of two
    return hv & (this->cf_table_size_ - 1);
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
inline uint32_t DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
fingerprint(uint32_t hash_value) const {
    uint32_t fingerprint = hash_value & fp_mask_;
    // make sure that fingerprint != 0
    fingerprint += (fingerprint == 0);
    return fingerprint;
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
inline void DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
firstPass(const element_type &item, uint32_t *fp, size_t *index) const {
    const uint64_t hash_value = hash_function_->hash(item);
    *index = getIndex(hash_value >> 32);
    *fp = fingerprint(hash_value);
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
inline uint32_t DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
indexComplement(const size_t index, const uint32_t fp) const {
    uint32_t hv = fingerprintComplement(index, fp);
    return getIndex(hv);
}


template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
DynamicCuckooFilter(uint32_t max_table_size) {
    this->fp_mask_ = (1ULL << bits_per_fp) - 1;
    this->cf_table_size_ = highestPowerOfTwo(max_table_size);

    if (entries_per_bucket == 4 && bits_per_fp == 4 && std::is_same<fp_type, uint8_t>::value) {
        bit_manager_ = new BitManager4<fp_type>();
    } else if (entries_per_bucket == 4 && bits_per_fp == 8 && std::is_same<fp_type, uint8_t>::value) {
        bit_manager_ = new BitManager8<fp_type>();
    } else if (entries_per_bucket == 4 && bits_per_fp == 12 && std::is_same<fp_type, uint16_t>::value) {
        bit_manager_ = new BitManager12<fp_type>();
    } else if (entries_per_bucket == 4 && bits_per_fp == 16 && std::is_same<fp_type, uint16_t>::value) {
        bit_manager_ = new BitManager16<fp_type>();
    } else if (entries_per_bucket == 2 && bits_per_fp == 32 && std::is_same<fp_type, uint32_t>::value) {
        bit_manager_ = new BitManager32<fp_type>();
    }  else {
        throw std::runtime_error("Invalid parameters.\n"
                                 "Supported parameter values for (entries_per_bucket, bits_per_fp, fp_type):\n"
                                 "{(4,  4, uint8_t),\n"
                                 " (4,  8, uint8_t),\n"
                                 " (4, 12, uint16_t),\n"
                                 " (4, 16, uint16_t),\n"
                                 " (2, 32, uint32_t)}\n");
    }


    hash_function_ = new HashFunction();

    active_cf_ = new CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>
            (this->cf_table_size_, this->bit_manager_, this->fp_mask_);
    head_cf_ = tail_cf_ = active_cf_;
    cf_count = 1;
    element_count = 0;
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
~DynamicCuckooFilter() {
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf_;
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* temp;
    while (cf) {
        temp = cf->next;
        delete cf;
        cf = temp;
    }

    delete bit_manager_;
    delete hash_function_;
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>*
DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
nextCF(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* next_cf;

    if(cf == tail_cf_) {
        next_cf = new CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>
                (cf_table_size_, bit_manager_, fp_mask_);
        active_cf_->next = next_cf;
        next_cf->prev = active_cf_;
        tail_cf_ = next_cf;
        cf_count++;
    }
    else {
        next_cf = active_cf_->next;
        if(next_cf->is_full){
            next_cf = nextCF(next_cf);
        }
    }
    return next_cf;
}


template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
void DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
storeVictim(Victim &victim, CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
    if (!cf->insertElement(victim.fp, victim.index, victim)){
        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* next_cf = nextCF(cf);
        storeVictim(victim, next_cf);
    }
}


template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
bool DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
insertElement(const element_type &element) {
    size_t index;
    uint32_t fp;

    firstPass(element, &fp, &index);

    if (active_cf_->is_full) {
        active_cf_ = nextCF(active_cf_);
    }

    if (active_cf_->insertElement(fp, index, victim_)) {
        this->element_count++;
    }
    else {
        storeVictim(victim_, head_cf_);
        this->element_count++;
    }

    return true;
}


template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
bool DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
containsElement(const element_type &element) {
    size_t i1, i2;
    uint32_t fp;

    firstPass(element, &fp, &i1);
    i2 = indexComplement(i1, fp);

    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf_;
    while (cf) {
        if (cf->containsElement(i1, i2, fp)) {
            return true;
        }
        else {
            cf = cf->next;
        }
    }
    return false;
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
bool DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
deleteElement(const element_type &element) {
    size_t i1, i2;
    uint32_t fp;

    firstPass(element, &fp, &i1);
    i2 = indexComplement(i1, fp);

    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf_;
    while (cf) {
        if (cf->deleteElement(i1, i2, fp)){
            this->element_count--;
            return true;
        }
        else{
            cf = cf->next;
        }
    }
    return false;
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
void DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
removeCF(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* prev = cf->prev;
    if (!prev){
        this->head_cf_ = cf->next;
    }
    else{
        prev->next = cf->next;
    }
    delete cf; // clear memory
    this->cf_count--;
}


template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
void DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::compact(){
    int sparse_cf_count = 0;
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf_;
    while (cf) {
        if (!cf->is_full) {
            sparse_cf_count++;
        }
        cf = cf->next;
    }
    if (sparse_cf_count == 0) return;

    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>** cfq  =
            new CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>*[sparse_cf_count];
    int j = 0;
    cf = head_cf_;
    while (cf) {
        if(!cf->is_full){
            cfq[j++] = cf;
        }
        cf = cf->next;
    }


    sort(cfq, sparse_cf_count);
    for (int i = 0; i < sparse_cf_count-1; i++) {
        for (int j = sparse_cf_count-1; j > i; j--) {
            cfq[i]->moveElements(cfq[j]);
            if (cfq[i]->is_empty) {
                this->removeCF(cfq[i]);
                this->cf_count--;
                break; // move to next cf
            }
        }
    }

    delete cfq;
}

template<typename element_type,
        size_t entries_per_bucket,
        size_t bits_per_fp,
        typename fp_type>
void DynamicCuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
sort(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>** cfq, int count){
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* temp;
    for (int i = 0; i < count-1; i++){
        for (int j = 0; j < count-1-i; j++){
            if(cfq[j]->element_count > cfq[j+1]->element_count){
                temp = cfq[j];
                cfq[j] = cfq[j+1];
                cfq[j+1] = temp;
            }
        }
    }
}