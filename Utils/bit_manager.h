#ifndef CUCKOOFILTER_BIT_MANAGER_H
#define CUCKOOFILTER_BIT_MANAGER_H

#include <stdint.h>
#include <stdlib.h>

// http://www-graphics.stanford.edu/~seander/bithacks.html
/**
 * Class for managing bits in memory location.
 * @tparam fp_type
 */
template<typename fp_type>
class BitManager {
public:
    virtual bool hasvalue(uint64_t value, uint32_t fp) = 0;

    virtual uint32_t read(size_t pos, const uint8_t *p) = 0;

    virtual void write(size_t pos, const uint8_t *p, uint32_t fp) = 0;
};

/**
 * Class for managing bits of length 4 in memory location.
 * @tparam fp_type
 */
template<typename fp_type = uint8_t>
class BitManager4 : public BitManager<fp_type> {
public:
    bool hasvalue(uint64_t value, uint32_t fp);

    uint32_t read(size_t pos, const uint8_t *p);

    void write(size_t pos, const uint8_t *p, uint32_t fp);
};

/**
 * Class for managing bits of length 8 in memory location.
 * @tparam fp_type
 */
template<typename fp_type = uint8_t>
class BitManager8 : public BitManager<fp_type> {
public:
    bool hasvalue(uint64_t value, uint32_t fp);

    uint32_t read(size_t pos, const uint8_t *p);

    void write(size_t pos, const uint8_t *p, uint32_t fp);
};


/**
 * Class for managing bits of length 12 in memory location.
 * @tparam fp_type
 */
template<typename fp_type = uint16_t>
class BitManager12 : public BitManager<fp_type> {
public:

    bool hasvalue(uint64_t value, uint32_t fp);

    uint32_t read(size_t pos, const uint8_t *p);

    void write(size_t pos, const uint8_t *p, uint32_t fp);
};

/**
 * Class for managing bits of length 16 in memory location.
 * @tparam fp_type
 */
template<typename fp_type = uint16_t>
class BitManager16 : public BitManager<fp_type> {
public:

    bool hasvalue(uint64_t value, uint32_t fp);

    uint32_t read(size_t pos, const uint8_t *p);

    void write(size_t pos, const uint8_t *p, uint32_t fp);
};

/**
 * Class for managing bits of length 32 in memory location.
 * @tparam fp_type
 */
template<typename fp_type = uint32_t>
class BitManager32 : public BitManager<fp_type> {
public:
    bool hasvalue(uint64_t value, uint32_t fp);

    uint32_t read(size_t pos, const uint8_t *p);

    void write(size_t pos, const uint8_t *p, uint32_t fp);
};


#endif
