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


struct Victim {
    uint32_t fp;
    size_t index;
};


template<typename element_type, typename fp_type = uint8_t>
class CuckooFilter {


private:
    uint32_t fp_mask;
    CuckooTable<fp_type>* table;
    size_t element_count;
    HashFunction<element_type>* hash_function;
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
        const u_int32_t hash_value = hash_function->hash(item);
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
    explicit CuckooFilter(uint32_t max_table_size, size_t bits_per_fp = 8, size_t entries_per_bucket = 4) {
        element_count = 0;
        victim = {0, 0};
        this->fp_mask = (1ULL << bits_per_fp) - 1;
        size_t table_size = highestPowerOfTwo(max_table_size);
        table = new CuckooTable<fp_type>(table_size, bits_per_fp, entries_per_bucket, fp_mask);

        if (std::is_same<element_type, uint32_t>::value) {
            hash_function = new MultiplyShift();
        }
        else if (false) {
            // TODO: for other types
        }
        else {
            // TODO print error, throw exception
        }
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
        delete hash_function;
    }
};



#include <iostream>
#include <assert.h>

using namespace std;

int main() {
    size_t total_items = 64;
    CuckooFilter<uint32_t> filter(total_items);

    uint32_t num_inserted = 0;
    for (uint32_t i = 0; i < 64; i++, num_inserted++) {
        if (!filter.insertElement(i)) {
            break;
        }
    }

    for (uint32_t i = 0; i < num_inserted; i++) {
        printf("%lu\n", i);
        assert(filter.containsElement(i));
    }

    cout << filter.deleteElement(2);
    return 0;
}