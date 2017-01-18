#ifndef BITOPERATIONS_HPP_
#define BITOPERATIONS_HPP_

#include <stdint.h>

static inline uint8_t bitwiseAND(uint8_t byte, uint8_t mask) {
    return (byte & mask);
}

static inline uint64_t bitwiseAND(uint64_t byte, uint64_t mask) {
    return (byte & mask);
}

static inline void bitwiseAND_r(uint8_t &byte, uint8_t mask) {
    byte &= mask;
}

static inline void bitwiseAND_r(uint64_t &byte, uint64_t mask) {
    byte &= mask;
}

static inline uint8_t bitwiseOR(uint8_t byte, uint8_t mask) {
    return (byte | mask);
}

static inline void bitwiseOR_r(uint8_t &byte, uint8_t mask) {
    byte |= mask;
}

static inline uint8_t clearBit(uint8_t &byte, uint8_t bit) {
    return (byte & ~(1 << bit));
}

static inline void clearBit_r(uint8_t &byte, uint8_t bit) {
    byte &= ~(1 << bit);
}

static inline bool readBit(uint8_t byte, uint8_t bit) {
    return (byte & (1 << bit));
}

static inline uint8_t setBit(uint8_t byte, uint8_t bit) {
    return (byte | (1 << bit));
}

static inline void setBit_r(uint8_t &byte, uint8_t bit) {
    byte |= (1 << bit);
}

static inline uint8_t shiftLeft(uint8_t &byte, uint8_t amount) {
    return (byte << amount);
}

static inline void shiftLeft_r(uint8_t &byte, uint8_t amount) {
    byte <<= amount;
}

static inline uint8_t shiftRight(uint8_t &byte, uint8_t amount) {
    return (byte >> amount);
}

static inline void shiftRight_r(uint8_t &byte, uint8_t amount) {
    byte >>= amount;
}

#endif /* BITOPERATIONS_HPP_ */
