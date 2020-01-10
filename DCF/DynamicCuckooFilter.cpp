#include "DynamicCuckooFilter.h"
#include "iostream"

template<typename element_type, typename fp_type>
DynamicCuckooFilter<element_type, fp_type>::DynamicCuckooFilter(uint32_t max_table_size, size_t bits_per_fp,
                                                                size_t entries_per_bucket) {

    this->cf_table_size = highestPowerOfTwo(max_table_size);
    this->bits_per_fp = bits_per_fp;
    this->entries_per_bucket = entries_per_bucket;
    this->hashFunction = new HashFunction();

    active_cf = new CuckooFilter<element_type, fp_type>(cf_table_size, bits_per_fp, entries_per_bucket, hashFunction);
    head_cf = tail_cf = active_cf;
    cf_count = 1;
    element_count = 0;

}

template<typename element_type, typename fp_type>
DynamicCuckooFilter<element_type, fp_type>::~DynamicCuckooFilter() {
    CuckooFilter<element_type, fp_type> *cf = head_cf;
    CuckooFilter<element_type, fp_type> *temp;
    while (cf) {
        temp = cf->next;
        delete cf;
        cf = temp;
    }
}

template<typename element_type, typename fp_type>
CuckooFilter<element_type, fp_type> *
DynamicCuckooFilter<element_type, fp_type>::nextCF(CuckooFilter<element_type, fp_type> *cf) {
    CuckooFilter<element_type, fp_type> *next_cf;

    // TODO: rewrite tail recursion as a loop, test this function
    if (cf == tail_cf || active_cf->next == nullptr) {
        next_cf = new CuckooFilter<element_type, fp_type>(this->cf_table_size,
                                                          this->bits_per_fp,
                                                          this->entries_per_bucket,
                                                          hashFunction);
        active_cf->next = next_cf;
        next_cf->prev = active_cf;
        tail_cf = next_cf;
        cf_count++;
    } else {
        next_cf = active_cf->next;
        if (next_cf->is_full) {
            next_cf = nextCF(next_cf);
        }
    }
    return next_cf;
}

template<typename element_type, typename fp_type>
void DynamicCuckooFilter<element_type, fp_type>::removeCF(CuckooFilter<element_type, fp_type> *cf) {
    CuckooFilter<element_type, fp_type> *prev = cf->prev;
    if (!prev) {
        this->head_cf = cf->next;
    } else {
        prev->next = cf->next;
    }
    delete cf; // clear memory
    this->cf_count--;
}

template<typename element_type, typename fp_type>
bool DynamicCuckooFilter<element_type, fp_type>::insertElement(element_type element) {
    if (active_cf->is_full) {
        active_cf = nextCF(active_cf);
    }

    if (active_cf->insertElement(element)) {
        this->element_count++;
    } else {
        storeVictim(active_cf->victim);
        this->element_count++;
    }

    return true;
}

template<typename element_type, typename fp_type>
bool DynamicCuckooFilter<element_type, fp_type>::containsElement(element_type element) {
    auto *cf = head_cf;
    while (cf) {
        // TODO: calculate second index only if necessary & only for the first cf
        if (cf->containsElement(element)) {
            return true;
        } else {
            cf = cf->next;
        }
    }
    return false;
}

template<typename element_type, typename fp_type>
bool DynamicCuckooFilter<element_type, fp_type>::deleteElement(element_type element) {
    CuckooFilter<element_type, fp_type> *cf = head_cf;
    while (cf) {
        // TODO: what is faster: del(fp, i1, i2) or (del(fp, i1) || del(fp, i2))
        if (cf->deleteElement(element)) {
            this->element_count--;
            return true;
        } else {
            cf = cf->next;
        }
    }
    return false;
}

template<typename element_type, typename fp_type>
void DynamicCuckooFilter<element_type, fp_type>::storeVictim(Victim &victim) {
    auto *cf = nextCF(active_cf);
    if (!cf->insert(victim.fp, victim.index)) {
        active_cf = active_cf->next;
        storeVictim(cf->victim);
    }
}

template<typename element_type, typename fp_type>
void DynamicCuckooFilter<element_type, fp_type>::sort(CuckooFilter<element_type, fp_type> **cfq, int count) {
    CuckooFilter<element_type, fp_type> *temp;
    // TODO: faster sort? probably wouldn't pay of because count ~ 10
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - 1 - i; j++) {
            if (cfq[j]->element_count > cfq[j + 1]->element_count) {
                temp = cfq[j];
                cfq[j] = cfq[j + 1];
                cfq[j + 1] = temp;
            }
        }
    }
}

template<typename element_type, typename fp_type>
void DynamicCuckooFilter<element_type, fp_type>::print() {
    auto p = head_cf;

    int no_filter = 1;
    while (p != nullptr) {
        std::cout << "FILTER NUMBER: " << no_filter++ << " " << std::endl;
        std::cout << p->availability() << std::endl;
        p->print();
        p = p->next;
    }
}


template
class DynamicCuckooFilter<uint16_t, uint8_t>;

template
class DynamicCuckooFilter<unsigned long, unsigned short>;


template
class DynamicCuckooFilter<uint16_t, uint16_t>;