#include <stdint.h>
#include "hash.cpp"
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


template<typename element_type, typename fp_type>
class DynamicCuckooFilter{

private:
    HashFunction<element_type> hash_function;

    BitManager<fp_type>* bit_manager;
    const uint32_t fp_mask;
    const size_t bits_per_fp;
    const size_t entries_per_bucket;

    size_t element_count;
    size_t cf_count;

    int cf_capacity;
    int cf_table_size;

    Victim victim;

    CuckooFilter<element_type, fp_type>* head_cf;
    CuckooFilter<element_type, fp_type>* tail_cf;
    CuckooFilter<element_type, fp_type>* active_cf;

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

    DynamicCuckooFilter(uint32_t max_table_size, size_t bits_per_fp, size_t entries_per_bucket = 4) {
        this->bits_per_fp = bits_per_fp;
        this->fp_mask = (1ULL << bits_per_fp) - 1;
        this->entries_per_bucket = entries_per_bucket;

        cf_table_size = highestPowerOfTwo(max_table_size);
        // TODO: decide on capacity
        cf_capacity = size_t(0.9 * cf_table_size);

        // TODO: init BitManager, shared for all CFs
        if (entries_per_bucket == 4 && bits_per_fp == 4 && std::is_same<fp_type, uint8_t>::value) {
            bit_manager = new BitManager4<fp_type>();
        }
        else if (entries_per_bucket == 4 && bits_per_fp == 8 && std::is_same<fp_type, uint8_t>::value) {
            bit_manager = new BitManager8<fp_type>();
        }
        else if (entries_per_bucket == 4 && bits_per_fp == 12 && std::is_same<fp_type, uint16_t>::value) {
            bit_manager = new BitManager12<fp_type>();
        }
        else if (entries_per_bucket == 4 && bits_per_fp == 16 && std::is_same<fp_type, uint16_t>::value) {
            bit_manager = new BitManager16<fp_type>();
        }
        else if (entries_per_bucket == 2 && bits_per_fp == 32 && std::is_same<fp_type, uint32_t>::value) {
            bit_manager = new BitManager32<fp_type>();
        }
        else {
            // TODO: print error, throw exception
        }

        if (std::is_same<element_type, uint32_t>::value) {
            hash_function = new MultiplyShift();
        }
        else if (false) {
            // TODO: for other types
        }
        else {
            // TODO print error, throw exception
        }

        active_cf = new CuckooFilter<element_type, fp_type>(cf_table_size,
                                                            bit_manager,
                                                            bits_per_fp,
                                                            fp_mask,
                                                            entries_per_bucket);
        head_cf = tail_cf = active_cf;
        cf_count = 1;
        element_count = 0;
    }

    ~DynamicCuckooFilter() {
        CuckooFilter<element_type, fp_type>* cf = head_cf;
        CuckooFilter<element_type, fp_type>* temp;
        while (cf) {
            temp = cf->next;
            delete cf;
            cf = temp;
        }
        delete bit_manager;
    }

    CuckooFilter<element_type, fp_type>* nextCF(CuckooFilter<element_type, fp_type>* cf) {
        CuckooFilter<element_type, fp_type>* next_cf;

        // TODO: rewrite tail recursion as a loop, test this function
        if(cf == tail_cf) {
            next_cf = new CuckooFilter<element_type, fp_type>(cf_table_size,
                                                              bit_manager,
                                                              bits_per_fp,
                                                              fp_mask,
                                                              entries_per_bucket);
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


    void storeVictim(Victim &victim) {
        CuckooFilter<element_type, fp_type>* cf = nextCF(head_cf);
        if (!cf->insertElement(victim.index, victim.fp, victim)){
            cf = getNextCF(cf);
            storeVictim(victim);
        }
    }


    bool insertElement(const element_type &element) {
        size_t index;
        uint32_t fp;

        firstPass(element, &fp, &index);

        if (active_cf->is_full) {
            active_cf = nextCF(active_cf);
        }

        if (active_cf->insert(index, fp, victim)) {
            this->element_count++;
        }
        else {
            storeVictim(victim);
            this->element_count++;
        }

        return true;
    }


    bool containsElement(const element_type &element) {
        size_t i1, i2;
        uint32_t fp;

        firstPass(element, &fp, &i1);
        i2 = indexComplement(i1, fp);

        CuckooFilter<element_type, fp_type>* cf = head_cf;
        while (cf) {
            // TODO: calculate second index only if necessary, containsElement always calculates i2 again
            if (cf->containsElement(fp, i1, i2)) {
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
        i2 = indexComplement(i1, fp);

        CuckooFilter<element_type, fp_type>* cf = head_cf;
        while (cf) {
            // TODO: what is faster: del(fp, i1, i2) or (del(fp, i1) || del(fp, i2))
            if (cf->deleteElement(fp, i1, i2)){
                this->element_count--;
                return true;
            }
            else{
                cf = cf->next;
            }
        }
        return false;
    }


    void removeCF(CuckooFilter<element_type, fp_type>* cf) {
        CuckooFilter<element_type, fp_type>* prev = cf->prev;
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
        CuckooFilter<element_type, fp_type>* cf = head_cf;
        while (cf) {
            if (!cf->is_full) {
                sparse_cf_count++;
            }
            cf = cf->next;
        }
        if (sparse_cf_count == 0) return;

        CuckooFilter<element_type, fp_type>** cfq  = new CuckooFilter<element_type, fp_type>*[sparse_cf_count];
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

        // TODO: test this line
        delete cfq;
    }

    void sort(CuckooFilter<element_type, fp_type>** cfq, int count){
        CuckooFilter<element_type, fp_type>* temp;
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



/*
#include <iostream>
#include <assert.h>

using namespace std;

int main() {
    size_t total_items = 64;
    CuckooFilter<size_t> filter(total_items);

    size_t num_inserted = 0;
    for (size_t i = 0; i < 64; i++, num_inserted++) {
        if (!filter.insertElement(i)) {
            break;
        }
    }

    for (size_t i = 0; i < num_inserted; i++) {
        printf("%lu\n", i);
        assert(filter.containsElement(i));
    }

    cout << filter.deleteElement(2);
    return 0;
}*/
