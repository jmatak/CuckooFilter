#include <iostream>
#include <assert.h>
#include <math.h>
#include <vector>
#include <bitset>

#include "CuckooFilter.cpp"

#define haszero(x) (((x)-0x01010101ULL) & (~(x)) & 0x80808080ULL)
#define half(x,n) ((x) ^ (0b00000001000000010000000100000001 * (n)))
#define op(n) (~0UL/255 * (n))
#define hasvalue(x,n) (haszero((x) ^ (~0UL/255 * (n))))

using namespace std;

int main() {
    size_t total_items = 64;
    CuckooFilter<size_t> filter(total_items);

    // Insert items to this cuckoo filter
    size_t num_inserted = 0;
    for (size_t i = 0; i < 64; i++, num_inserted++) {
        if (!filter.insertElement(i)) {
            break;
        }
    }

/*  uint8_t i1 = 1;
    uint8_t i2 = 2;
    uint8_t i3 = 3;
    uint8_t i4 = 4;

    uint32_t  v = 0;
    v += 1;
    v += (2 << 8);
    v += (4 << 16);
    v += (3 << 24);

    printf("%d\n", haszero(v) != 0);
    std::cout << std::bitset<32>(~0UL/255) << "\n";
    std::cout << std::bitset<32>(op(i2)) << "\n";
    std::cout << std::bitset<32>(half(v, i2)) << "\n";
    std::cout << std::bitset<32>(v) << "\n";
    std::cout << std::bitset<32>(half(v, i1)) << "\n";
    printf("%d\n", hasvalue(v, i1) != 0);
    printf("%d\n", hasvalue(v, i2) != 0);
    printf("%d\n", hasvalue(i3, v) != 0);
    printf("%d\n", hasvalue(i4, v) != 0);*/

    // Check if previously inserted items are in the filter, expected
    // true for all items
    for (size_t i = 0; i < num_inserted; i++) {
        printf("%lu\n", i);
        assert(filter.containsElement(i));
    }

    cout << filter.deleteElement(2);

    return 0;
}