#include "Table.cpp"

#define kicks_max_count 500

struct Victim {
    size_t index;
    uint32_t fp;
};

// TODO: remove class CuckooTable and transfer its utilities over here; maybe a bit faster, but ugly code
template<typename element_type, typename fp_type>
class CuckooFilter {

private:
    CuckooTable<fp_type>* table;
    size_t capacity;

    inline size_t getIndex(uint32_t hv) const {
        // equivalent to modulo when number of buckets is a power of two
        return hv & (table->table_size - 1);
    }

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const {
        uint32_t hv = fingerprintComplement(index, fp);
        return getIndex(hv);
    }

    inline void refreshOnDelete() {
        this->element_count--;
        if (this->element_count < this->capacity) {
            this->is_full = false;
        }
        if (this->element_count == 0) {
            this->is_empty = true;
        }
    }

    inline void refreshOnInsert() {
        this->element_count++;
        if (this->element_count == this->capacity) {
            this->is_full = true;
        }
        if (this->element_count > 0) {
            this->is_empty = false;
        }
    }


public:

    CuckooFilter<element_type, fp_type>* prev = NULL;
    CuckooFilter<element_type, fp_type>* next = NULL;

    bool is_full = false;
    bool is_empty = true;

    size_t element_count;

    explicit CuckooFilter(uint32_t table_size,
                          BitManager<fp_type>* bit_manager,
                          size_t bits_per_fp,
                          uint32_t fp_mask,
                          size_t entries_per_bucket) {
        // TODO: capacity?
        capacity = size_t(0.9 * table_size);
        element_count = 0;
        table = new CuckooTable<fp_type>(table_size, bit_manager, bits_per_fp, fp_mask, entries_per_bucket);
    }

    bool insertElement(uint32_t fp, size_t index, Victim &victim) {
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


    bool deleteElement(uint32_t fp, size_t i1) {
        bool deletion =
            table->deleteFingerprint(fp, i1)
            ||
            table->deleteFingerprint(fp, indexComplement(i1, fp));

        if (deletion) {
            this->refreshOnDelete();
        }

        return deletion;
    }

    bool deleteElement(uint32_t fp, size_t i1, size_t i2) {
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


    bool containsElement(uint32_t fp, size_t i1) {
        size_t i2 = indexComplement(i1, fp);
        return table->containsFingerprint(i1, i2, fp);
    }


    bool containsElement(uint32_t fp, size_t i1, size_t i2) {
        return table->containsFingerprint(i1, i2, fp);
    }


    bool replacementInsert(size_t i, size_t j, uint32_t fp) {
        table->insertFingerprint(i, j, fp);
    }

    void moveElements(CuckooFilter<element_type, fp_type>* cf) {
        uint32_t fp;

        for(size_t i = 0; i < table->table_size; i++){
            for(int j = 0; j < table->entries_per_bucket; j++){
                fp = table->getFingerprint(i, j);

                if(fp){
                    if(cf->is_full || this->is_empty) return;
                    if(cf->table->insertFingerprint(i, j, fp)){
                        table->insertFingerprint(i, j, 0);
                        this->refreshOnDelete();
                        cf->refreshOnInsert();
                    }
                }
            }
        }
    }

    ~CuckooFilter() {
        delete table;
    }
};
