#include "Filter.cpp"

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

// TODO: set bits_per_fp as an attribute
template<typename element_type, size_t bits_per_fp = 8, typename HashFunction = MultiplyShift>
class DynamicCuckooFilter{

    static const uint32_t fp_mask = CuckooTable<bits_per_fp>::fp_mask;

private:

    size_t element_count;
    size_t cf_count;

    int cf_capacity;
    int cf_table_size;

    double false_positive;
    double single_false_positive;

    int fingerprint_size;


    Victim victim;

    CuckooFilter<element_type>* head_cf;
    CuckooFilter<element_type>* tail_cf;
    CuckooFilter<element_type>* active_cf;

    inline size_t getIndex(uint32_t hv) const {
        // equivalent to modulo when number of buckets is a power of two
        return hv & (this->cf_table_size - 1);
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

    //construction & distruction functions
    DynamicCuckooFilter(uint32_t max_table_size) {
        cf_table_size = highestPowerOfTwo(max_table_size);
        // TODO: decide on capacity
        cf_capacity = size_t(0.9 * cf_table_size);

        // TODO: init BitManager, shared for all CFs
        active_cf = new CuckooTable<bits_per_fp>(cf_table_size);
        head_cf = tail_cf = active_cf;
        cf_count = 1;
        element_count = 0;
    }

    ~DynamicCuckooFilter() {
        // TODO: delete all CFs
    }

    CuckooFilter<element_type>* nextCF(CuckooFilter<element_type>* cf) {
        CuckooFilter<element_type>* next_cf;

        // TODO: rewrite tail recursion as a loop
        if(cf == tail_cf) {
            next_cf = new CuckooTable<bits_per_fp>(cf_table_size);
            active_cf->next = next_cf;
            next_cf->prev = active_cf;
            tail_cf = next_cf;
            cf_count++;
        }
        else {
            next_cf = active_cf->next;
            if(next_cf->is_full){
                next_cf = nextCF(next_cf);
            }
        }
        return next_cf;
    }

    bool insertElement(const element_type &element) {
        size_t index;
        uint32_t fp;

        firstPass(element, &fp, &index);

        if (active_cf->is_full) {
            active_cf = nextCF(active_cf);
        }

        // TODO: insert with victim
        if (active_cf->insert(index, fp)) {
            this->element_count++;
        }
        else{
            failureHandle(victim);
            this->element_count++;
        }

        return true;
    }


    bool failureHandle(Victim &victim) {
        // TODO: implementation
        return false;
    }


    bool containsElement(const element_type &element) {
        size_t i1;
        uint32_t fp;

        firstPass(element, &fp, &i1);

        CuckooFilter<element_type>* cf = head_cf;
        while (cf) {
            // TODO: calculate second index only if necessary, containsElement always calculates i2 again
            if (cf->containsElement(fp, i1)) {
                return true;
            }
            else {
                cf = cf->next;
            }
        }
        return false;
    }

    bool deleteElement(const element_type &element) {
        size_t i1, i2;
        uint32_t fp;

        firstPass(element, &fp, &i1);

        CuckooFilter<element_type>* cf = head_cf;
        while (cf) {
            // TODO: calculate second index only if necessary, containsElement always calculates i2 again
            if (cf->containsElement(fp, i1)){
                if(cf->deleteElement(fp, i1)){
                    element_count--;
                    return true;
                }
            }
            else{
                cf = cf->next;
            }
        }
        return false;
    }


    void removeCF(CuckooFilter<element_type>* cf) {
        CuckooFilter<element_type>* prev = cf->prev;
        if (!prev){
            this->head_cf = cf->next;
        }
        else{
            prev->next = cf->next;
        }
        delete cf; // clear memory
        this->cf_count--;
    }


    void compact(){
        int sparse_cf_count = 0;
        CuckooFilter<element_type>* cf = head_cf;
        while (cf) {
            if (!cf->is_full) {
                sparse_cf_count++;
            }
            cf = cf->next;
        }
        if (sparse_cf_count == 0) return;

        CuckooFilter<element_type>** cfq  = new CuckooFilter<element_type>*[sparse_cf_count];
        int j = 0;
        cf = head_cf;
        while (cf) {
            if(!cf->is_full){
                cfq[j++] = cf;
            }
            cf = cf->next;
        }


        // TODO: implemented following instructions in DCF paper, there's room for improving time efficiency
        sort(cfq, sparse_cf_count);
        for (int i = 0; i < sparse_cf_count-1; i++) {
            for (int j = sparse_cf_count-1; j > i; j--) {
                cfq[i]->move_elements(cfq[j]);
                if (cfq[i]->is_empty) {
                    this->removeCF(cfq[i]);
                    break; // move to next cf
                }
            }
        }

    }

    void sort(CuckooFilter<element_type>** cfq, int count){
        CuckooFilter<element_type>* temp;
        // TODO: faster sort? probably wouldn't pay of because count ~ 10
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


};