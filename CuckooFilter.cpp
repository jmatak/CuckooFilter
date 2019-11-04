#include "CuckooFilter.h"

template<typename element_type, typename fp_type>
CuckooFilter<element_type, fp_type>::CuckooFilter(size_t tableSize) {

}

template<typename element_type, typename fp_type>
fp_type CuckooFilter<element_type, fp_type>::fingerprintFunction(element_type element) {
    return nullptr;
}

template<typename element_type, typename fp_type>
void CuckooFilter<element_type, fp_type>::insertElement(element_type element) {

}

template<typename element_type, typename fp_type>
void CuckooFilter<element_type, fp_type>::deleteElement(element_type element) {

}

template<typename element_type, typename fp_type>
bool CuckooFilter<element_type, fp_type>::contains(element_type element) {
    return false;
}
