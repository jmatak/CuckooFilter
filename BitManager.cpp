#include <stdint.h>
#include <stdlib.h>

// http://www-graphics.stanford.edu/~seander/bithacks.html

template<typename fp_type>
class BitManager {
public:
    virtual bool hasvalue(uint64_t value, uint32_t fp) = 0;
    virtual uint32_t read(size_t pos, const uint8_t *p) = 0;
};


// entries_per_bucket = 4, bits_per_element = 4
template<typename fp_type = uint8_t>
class BitManager4: public BitManager<fp_type> {
public:
    inline bool hasvalue(uint64_t value, uint32_t fp) {
        uint64_t neg = value ^ (0x1111ULL * fp);
        return (neg - 0x1111ULL) & (~neg) & 0x8888ULL;
    }
    inline uint32_t read(size_t pos, const uint8_t *p) {
        p += (pos >> 1);
        return *((fp_type*)p) >> ((pos & 1) << 2);
    }
};


// entries_per_bucket = 4, bits_per_element = 8
template<typename fp_type = uint8_t>
class BitManager8: public BitManager<fp_type> {
public:
    inline bool hasvalue(uint64_t value, uint32_t fp) {
        uint64_t neg = value ^ (0x01010101ULL * fp);
        return (neg - 0x01010101ULL) & (~neg) & 0x80808080ULL;
    }
    inline uint32_t read(size_t pos, const uint8_t *p) {
        p += pos;
        fp_type bits = *((fp_type*)p);
        return bits;
    }
};

// entries_per_bucket = 4, bits_per_element = 12
template<typename fp_type = uint16_t>
class BitManager12: public BitManager<fp_type> {
public:
    inline bool hasvalue(uint64_t value, uint32_t fp) {
        uint64_t neg = value ^ (0x001001001001ULL * (fp));
        return (neg - 0x001001001001ULL) & (~neg) & 0x800800800800ULL;
    }
    inline uint32_t read(size_t pos, const uint8_t *p) {
        p += pos + (pos>>1);
        return *((fp_type*)p) >> ((pos & 1) << 2);
    }
};

// entries_per_bucket = 4, bits_per_element = 16
template<typename fp_type = uint16_t>
class BitManager16: public BitManager<fp_type> {
public:
    inline bool hasvalue(uint64_t value, uint32_t fp) {
        uint64_t neg = value ^ (0x0001000100010001ULL * (fp));
        return (neg - 0x0001000100010001ULL) & (~neg) & 0x8000800080008000ULL;
    }
    inline uint32_t read(size_t pos, const uint8_t *p) {
        p += (pos<<1);
        fp_type bits = *((fp_type*)p);
        return bits;
    }
};

// entries_per_bucket = 2, bits_per_element = 32
template<typename fp_type = uint32_t>
class BitManager32: public BitManager<fp_type> {
public:
    inline bool hasvalue(uint64_t value, uint32_t fp) {
        uint64_t neg = value ^ (0x0000000100000001ULL * (fp));
        return (neg - 0x0000000100000001ULL) & (~neg) & 0x8000000080000000ULL;
    }
    inline uint32_t read(size_t pos, const uint8_t *p) {
        p += (pos << 2);
        fp_type bits = *((fp_type*)p);
        return bits;
    }
};


/*// Test
#include <iostream>
#include <bitset>

using namespace std;

int main(int,char**)
{
    BitManager<uint8_t> *bc = new BitManager8<uint8_t>();

    uint32_t i1 = 1;
    uint32_t i2 = 2;
    uint32_t i3 = 3;
    uint32_t i4 = 4;


    uint64_t  v = 0;
    v += 1;
    v += (2 << 8);
    v += (4 << 16);
    v += (5 << 24);
    std::cout << bc->hasvalue(v, i1) << "\n";
    std::cout << bc->hasvalue(v, i2) << "\n";
    std::cout << bc->hasvalue(v, i3) << "\n";
    std::cout << bc->hasvalue(v, i4) << "\n";
    return 0;
}*/
