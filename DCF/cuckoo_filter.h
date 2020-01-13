#include "cuckoo_table.h"

#define kicks_max_count 500

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
class CuckooFilter {

private:
    CuckooTable<fp_type, entries_per_bucket, bits_per_fp>* table;
    size_t capacity;

    inline size_t getIndex(uint32_t hv) const;

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const;

    inline void refreshOnDelete();

    inline void refreshOnInsert();

public:

    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* prev = NULL;
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* next = NULL;

    bool is_full = false;
    bool is_empty = true;

    size_t element_count;

    explicit CuckooFilter(uint32_t table_size,
                          BitManager<fp_type>* bit_manager,
                          uint32_t fp_mask);

    bool insertElement(uint32_t fp, size_t index, Victim &victim);

    bool deleteElement(uint32_t fp, size_t i1);

    bool deleteElement(uint32_t fp, size_t i1, size_t i2);

    bool containsElement(uint32_t fp, size_t i1);

    bool containsElement(uint32_t fp, size_t i1, size_t i2);

    void replacementInsert(size_t i, size_t j, uint32_t fp);

    void moveElements(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf);

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
deleteElement(uint32_t fp, size_t i1) {
    bool deletion =
            table->deleteFingerprint(fp, i1)
            ||
            table->deleteFingerprint(fp, indexComplement(i1, fp));

    if (deletion) {
        this->refreshOnDelete();
    }

    return deletion;
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
deleteElement(uint32_t fp, size_t i1, size_t i2) {
    bool deletion =
            table->deleteFingerprint(fp, i1)
            ||
            table->deleteFingerprint(fp, i2);

    // TODO: redundant checking, send CF object to deleteFingerprint function
    if (deletion) {
        this->refreshOnDelete();
    }

    return deletion;
}

template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
containsElement(uint32_t fp, size_t i1) {
    size_t i2 = indexComplement(i1, fp);
    return table->containsFingerprint(i1, i2, fp);
}


template<typename element_type, size_t entries_per_bucket, size_t bits_per_fp, typename fp_type>
bool CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>::
containsElement(uint32_t fp, size_t i1, size_t i2) {
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