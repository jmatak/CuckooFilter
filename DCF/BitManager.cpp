#include "BitManager.h"

template<typename fp_type>
bool BitManager4<fp_type>::hasvalue(uint64_t value, uint32_t fp) {
    uint64_t neg = value ^(0x1111ULL * fp);
    return (neg - 0x1111ULL) & (~neg) & 0x8888ULL;
}

template<typename fp_type>
uint32_t BitManager4<fp_type>::read(size_t pos, const uint8_t *p) {
    p += (pos >> 1);
    return *((fp_type *) p) >> ((pos & 1) << 2);
}

template<typename fp_type>
void BitManager4<fp_type>::write(size_t pos, const uint8_t *p, uint32_t fp) {
    p += (pos >> 1);
    if ((pos & 1) == 0) {
        *((fp_type *) p) &= 0xf0;
        *((fp_type *) p) |= fp;
    } else {
        *((fp_type *) p) &= 0x0f;
        *((fp_type *) p) |= (fp << 4);
    }
}

template<typename fp_type>
bool BitManager8<fp_type>::hasvalue(uint64_t value, uint32_t fp) {
    uint64_t neg = value ^(0x01010101ULL * fp);
    return (neg - 0x01010101ULL) & (~neg) & 0x80808080ULL;
}

template<typename fp_type>
uint32_t BitManager8<fp_type>::read(size_t pos, const uint8_t *p) {
    p += pos;
    fp_type bits = *((fp_type *) p);
    return bits;
}

template<typename fp_type>
void BitManager8<fp_type>::write(size_t pos, const uint8_t *p, uint32_t fp) {
    ((fp_type *) p)[pos] = fp;
}

template<typename fp_type>
bool BitManager12<fp_type>::hasvalue(uint64_t value, uint32_t fp) {
    uint64_t neg = value ^(0x001001001001ULL * (fp));
    return (neg - 0x001001001001ULL) & (~neg) & 0x800800800800ULL;
}

template<typename fp_type>
uint32_t BitManager12<fp_type>::read(size_t pos, const uint8_t *p) {
    p += pos + (pos >> 1);
    return *((fp_type *) p) >> ((pos & 1) << 2);
}

template<typename fp_type>
void BitManager12<fp_type>::write(size_t pos, const uint8_t *p, uint32_t fp) {
    p += (pos + (pos >> 1));
    if ((pos & 1) == 0) {
        ((uint16_t *) p)[0] &= 0xf000;
        ((fp_type *) p)[0] |= fp;
    } else {
        ((fp_type *) p)[0] &= 0x000f;
        ((fp_type *) p)[0] |= (fp << 4);
    }
}

template<typename fp_type>
bool BitManager16<fp_type>::hasvalue(uint64_t value, uint32_t fp) {
    uint64_t neg = value ^(0x0001000100010001ULL * (fp));
    return (neg - 0x0001000100010001ULL) & (~neg) & 0x8000800080008000ULL;
}

template<typename fp_type>
uint32_t BitManager16<fp_type>::read(size_t pos, const uint8_t *p) {
    p += (pos << 1);
    fp_type bits = *((fp_type *) p);
    return bits;
}

template<typename fp_type>
void BitManager16<fp_type>::write(size_t pos, const uint8_t *p, uint32_t fp) {
    ((fp_type *) p)[pos] = fp;
}

template<typename fp_type>
bool BitManager32<fp_type>::hasvalue(uint64_t value, uint32_t fp) {
    uint64_t neg = value ^(0x0000000100000001ULL * (fp));
    return (neg - 0x0000000100000001ULL) & (~neg) & 0x8000000080000000ULL;
}

template<typename fp_type>
uint32_t BitManager32<fp_type>::read(size_t pos, const uint8_t *p) {
    p += (pos << 2);
    fp_type bits = *((fp_type *) p);
    return bits;
}

template<typename fp_type>
void BitManager32<fp_type>::write(size_t pos, const uint8_t *p, uint32_t fp) {
    ((fp_type *) p)[pos] = fp;
}

template
class BitManager4<uint8_t>;

template
class BitManager8<uint8_t >;

template
class BitManager12<uint16_t>;

template
class BitManager16<uint16_t>;

template
class BitManager32<uint32_t>;