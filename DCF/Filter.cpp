#include "hash.cpp"
#include "Table.cpp"

#define kicks_max_count 500


struct Victim {
    uint32_t fp = 0;
    size_t index;
};


// TODO: bits_per_fp -> put in ctor
template<typename element_type, size_t bits_per_fp = 8, typename HashFunction = MultiplyShift>
class CuckooFilter {

    static const uint32_t fp_mask = CuckooTable<bits_per_fp>::fp_mask;

private:
    uint32_t fingerprintFunction(element_type element);


    CuckooTable<bits_per_fp>* table;
    HashFunction hash_function;

    size_t capacity;

    inline size_t getIndex(uint32_t hv) const {
        // equivalent to modulo when number of buckets is a power of two
        return hv & (table->table_size - 1);
    }

    inline uint32_t fingerprint(uint32_t hash_value) const {
        uint32_t fingerprint;
        fingerprint = hash_value & fp_mask;
        // make sure that fingerprint != 0
        fingerprint += (fingerprint == 0);
        return fingerprint;
    }

    inline void firstPass(const element_type &item, uint32_t *fp, size_t *index) const {
        const u_int32_t hash_value = hash_function(item);
        *index = getIndex(hash_value);
        *fp = fingerprint(hash_value);
    }

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const {
        uint32_t hv = fingerprintComplement(index, fp);
        return getIndex(hv);
    }



public:

    CuckooFilter<element_type>* prev = NULL;
    CuckooFilter<element_type>* next = NULL;

    bool is_full = false;
    bool is_empty = true;

    size_t element_count;

    explicit CuckooFilter(uint32_t table_size) : element_count(0), hash_function() {
        // TODO: capacity?
        capacity = size_t(0.9 * table_size);
        table = new CuckooTable<bits_per_fp>(table_size);
    }

    bool insertElement(uint32_t fp, size_t index, Victim &victim) {
        size_t curr_index = index;
        uint32_t curr_fp = fp;
        uint32_t prev_fp;

        for (int kicks = 0; kicks < kicks_max_count; kicks++) {
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
        return false;
    }


    bool deleteElement(uint32_t fp, size_t i1) {
        return
            table->deleteFingerprint(fp, i1)
            ||
            table->deleteFingerprint(fp, indexComplement(i1, fp));
    }

    bool deleteElement(uint32_t fp, size_t i1, size_t i2) {
        return
                table->deleteFingerprint(fp, i1)
                ||
                table->deleteFingerprint(fp, i2);
    }


    bool containsElement(uint32_t fp, size_t i1) {
        size_t i2 = indexComplement(i1, fp);
        return table->containsFingerprint(i1, i2, fp);
    }


    bool containsElement(uint32_t fp, size_t i1, size_t i2) {
        return table->containsFingerprint(i1, i2, fp);
    }


    void moveElements(CuckooFilter<element_type>* cf) {
        uint32_t fp;

        for(size_t i = 0; i < table->table_size; i++){
            for(int j = 0; j < table->entries_per_bucket; j++){
                fp = table->getFingerprint(i, j);
                if(fp){
                    if(cf->is_full || this->is_empty) return;
                    Victim victim = {0, 0};
                    if(cf->insertElement(i, fp, false, victim)){
                        table->insertFingerprint(i, j, 0);
                        this->element_count--;

                        if (this->element_count < this->capacity){
                            this->is_full = false;
                        }
                        if (this->counter == 0){
                            this->is_empty = true;
                        }

                        if (cf->counter == cf->capacity){
                            cf->is_full = true;
                        }
                        if (cf->element_count > 0){
                            cf->is_empty = false;
                        }
                    }
                }
            }
        }
    }

    ~CuckooFilter() {
        delete table;
    }
};
