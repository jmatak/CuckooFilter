#include "hash.cpp"
#include "Table.cpp"

#define kicks_max_count 500

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

template<typename element_type, size_t bits_per_fp = 8, typename HashFunction = MultiplyShift>
class CuckooFilter {

    static const uint32_t fp_mask = CuckooTable<>::fp_mask;

    struct Victim {
        uint32_t fp = 0;
        size_t index;
    };


private:
    uint32_t fingerprintFunction(element_type element);

    CuckooTable<bits_per_fp> *table;
    size_t element_count;
    HashFunction hash_function;
    Victim victim;

    inline size_t getIndex(uint32_t hv) const {
        // equivalent to modulo when number of buckets is a power of two
        return hv & (table->getTableSize() - 1);
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

    bool insert(uint32_t fp, size_t index) {
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
        return true;
    }



public:
    explicit CuckooFilter(uint32_t max_table_size) : element_count(0), hash_function(), victim() {
        size_t table_size = highestPowerOfTwo(max_table_size);
        table = new CuckooTable<bits_per_fp>(table_size);
    }

    bool insertElement(element_type &element) {
        size_t index;
        uint32_t fp;

        if (victim.fp) return false;

        firstPass(element, &fp, &index);
        return this->insert(fp, index);
    }

    bool deleteElement(const element_type &element) {
        uint32_t fp;
        size_t i1, i2;

        firstPass(element, &fp, &i1);
        // TODO: this could be calculated only if necessary
        i2 = indexComplement(i1, fp);

        if (table->deleteFingerprint(fp, i1) || table->deleteFingerprint(fp, i2)) {
            this->element_count--;
        }
        else if (victim.fp && fp == victim.fp &&
                 (i1 == victim.index || i2 == victim.index)) {
            // element count -> upon agreement
            victim.fp = 0;
            return true;
        }
        else {
            return false;
        }

        if (victim.fp) {
            size_t index = victim.index;
            uint32_t fp =  victim.fp;
            victim.fp = 0;
            // element count -> upon agreement
            this->insert(fp, index);
        }

        return true;
    }

    bool containsElement(element_type &element) {
        uint32_t fp;
        size_t i1, i2;

        firstPass(element, &fp, &i1);
        i2 = indexComplement(i1, fp);

        // TODO: remove after debugging
        assert(i1 == indexComplement(i2, fp));

        bool match = victim.fp && (fp == victim.fp) && (i1 == victim.index || i2 == victim.index);
        return match || table->containsFingerprint(i1, i2, fp);
    }

    ~CuckooFilter() {
        delete table;
    }
};



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
}