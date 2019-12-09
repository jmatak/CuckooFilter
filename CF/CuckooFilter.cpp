#include "hash.cpp"
#include "CuckooTable.cpp"


const size_t kicks_max_count = 500;

static size_t highestPowerOfTwo(uint32_t v) {
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

    // TODO: fp_mask -> already defined in cuckoo table, fix imports
    static const uint32_t fp_mask = (1ULL << bits_per_fp) - 1;

    typedef struct {
        bool set;
        size_t index;
        uint32_t fp;
    } TargetCache;

private:
    uint32_t fingerprintFunction(element_type element);

    CuckooTable<bits_per_fp> *table;
    size_t element_count;

    HashFunction hash_function;
    TargetCache cache;

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

        cache.set = true;
        cache.index = curr_index;
        cache.fp = curr_fp;
        return true;
    }



public:
    explicit CuckooFilter(uint32_t max_table_size) : element_count(0), hash_function(), cache() {
        size_t table_size = highestPowerOfTwo(max_table_size);
        cache.set = false;
        table = new CuckooTable<bits_per_fp>(table_size);
    }

    bool insertElement(element_type &element) {
        size_t index;
        uint32_t fp;

        // check cache
        if (cache.set) return false;

        firstPass(element, &fp, &index);
        return this->insert(fp, index);
    }

    bool deleteElement(const element_type &element) {
        uint32_t fp;
        size_t i1, i2;

        firstPass(element, &fp, &i1);
        i2 = indexComplement(i1, fp);

        if (table->deleteFingerprint(fp, i1) || table->deleteFingerprint(fp, i2)) {
            this->element_count--;
        }
        else if (cache.set && fp == cache.fp &&
                 (i1 == cache.index || i2 == cache.index)) {
            // element count -> upon agreement
            cache.set = false;
            return true;
        }
        else {
            return false;
        }

        if (cache.set) {
            cache.set = false;
            size_t index = cache.index;
            uint32_t fp =  cache.fp;
            // element count -> upon agreement
            this->insert(fp, index);
        }

        return true;
    }

    bool containsElement(element_type &element) {
        uint32_t fp;
        size_t i1, i2;
        bool match;

        firstPass(element, &fp, &i1);
        i2 = indexComplement(i1, fp);

        // TODO: remove after debugging
        assert(i1 == indexComplement(i2, fp));

        match = cache.set && (fp == cache.fp) && (i1 == cache.index || i2 == cache.index);
        return match || table->containsFingerprint(i1, i2, fp);
    }

    ~CuckooFilter() {
        delete table;
    }
};
