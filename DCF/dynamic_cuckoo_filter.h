#include <stdint.h>
#include "../Utils/hash_function.h"
#include "../Utils/util.h"
#include "cuckoo_filter.h"


template<typename element_type=uint32_t,
         size_t entries_per_bucket=4,
         size_t bits_per_fp=8,
         typename fp_type=uint8_t>
class DynamicCuckooFilter{

private:
    HashFunction* hash_function;

    BitManager<fp_type>* bit_manager;
    uint32_t fp_mask;

    int cf_table_size;

    Victim victim;

    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* head_cf;
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* tail_cf;
    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* active_cf;

    inline size_t getIndex(uint32_t hv) const {
        // equivalent to modulo when number of buckets is a power of two
        return hv & (this->cf_table_size - 1);
    }

    inline uint32_t fingerprint(uint32_t hash_value) const {
        uint32_t fingerprint = hash_value & fp_mask;
        // make sure that fingerprint != 0
        fingerprint += (fingerprint == 0);
        return fingerprint;
    }

    inline void firstPass(const element_type &item, uint32_t *fp, size_t *index) const {
        const uint64_t hash_value = hash_function->hash(item);
        *index = getIndex(hash_value >> 32);
        *fp = fingerprint(hash_value);
    }

    inline uint32_t indexComplement(const size_t index, const uint32_t fp) const {
        uint32_t hv = fingerprintComplement(index, fp);
        return getIndex(hv);
    }

public:
    size_t element_count;
    size_t cf_count;

    DynamicCuckooFilter(uint32_t max_table_size) {
        this->fp_mask = (1ULL << bits_per_fp) - 1;
        this->cf_table_size = highestPowerOfTwo(max_table_size);

        if (entries_per_bucket == 4 && bits_per_fp == 4 && std::is_same<fp_type, uint8_t>::value) {
            bit_manager = new BitManager4<fp_type>();
        } else if (entries_per_bucket == 4 && bits_per_fp == 8 && std::is_same<fp_type, uint8_t>::value) {
            bit_manager = new BitManager8<fp_type>();
        } else if (entries_per_bucket == 4 && bits_per_fp == 12 && std::is_same<fp_type, uint16_t>::value) {
            bit_manager = new BitManager12<fp_type>();
        } else if (entries_per_bucket == 4 && bits_per_fp == 16 && std::is_same<fp_type, uint16_t>::value) {
            bit_manager = new BitManager16<fp_type>();
        } else if (entries_per_bucket == 2 && bits_per_fp == 32 && std::is_same<fp_type, uint32_t>::value) {
            bit_manager = new BitManager32<fp_type>();
        }  else {
            throw std::runtime_error("Invalid parameters.\n"
                                     "Supported parameter values for (entries_per_bucket, bits_per_fp, fp_type):\n"
                                     "{(4,  4, uint8_t),\n"
                                     " (4,  8, uint8_t),\n"
                                     " (4, 12, uint16_t),\n"
                                     " (4, 16, uint16_t),\n"
                                     " (2, 32, uint32_t)}\n");
        }


        hash_function = new HashFunction();

        active_cf = new CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>
                            (this->cf_table_size, this->bit_manager, this->fp_mask);
        head_cf = tail_cf = active_cf;
        cf_count = 1;
        element_count = 0;
    }

    ~DynamicCuckooFilter() {
        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf;
        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* temp;
        while (cf) {
            temp = cf->next;
            delete cf;
            cf = temp;
        }

        delete bit_manager;
        delete hash_function;
    }

    CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* nextCF(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* next_cf;

        if(cf == tail_cf) {
            next_cf = new CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>
                        (cf_table_size, bit_manager, fp_mask);
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


    void storeVictim(Victim &victim, CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
        if (!cf->insertElement(victim.fp, victim.index, victim)){
            CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* next_cf = nextCF(cf);
            storeVictim(victim, next_cf);
        }
    }


    bool insertElement(const element_type &element) {
        size_t index;
        uint32_t fp;

        firstPass(element, &fp, &index);

        if (active_cf->is_full) {
            active_cf = nextCF(active_cf);
        }

        if (active_cf->insertElement(fp, index, victim)) {
            this->element_count++;
        }
        else {
            storeVictim(victim, head_cf);
            this->element_count++;
        }

        return true;
    }


    bool containsElement(const element_type &element) {
        size_t i1, i2;
        uint32_t fp;

        firstPass(element, &fp, &i1);
        i2 = indexComplement(i1, fp);

        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf;
        while (cf) {
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

        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf;
        while (cf) {
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


    void removeCF(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf) {
        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* prev = cf->prev;
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
        CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>* cf = head_cf;
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
        cf = head_cf;
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

    void sort(CuckooFilter<element_type, entries_per_bucket, bits_per_fp, fp_type>** cfq, int count){
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


};



//#include <iostream>
//#include <assert.h>
//
//using namespace std;
//
//int main() {
//    uint32_t total_items = 64;
//    DynamicCuckooFilter<size_t, uint8_t> filter(total_items, 8, 4);
//
//    size_t num_inserted = 0;
//    for (size_t i = 0; i < 64; i++, num_inserted++) {
//        if (!filter.insertElement(i)) {
//            break;
//        }
//    }
//
//    for (size_t i = 0; i < num_inserted; i++) {
//        printf("%lu\n", i);
//        assert(filter.containsElement(i));
//    }
//
//    cout << filter.deleteElement(68);
//    return 0;
//}
