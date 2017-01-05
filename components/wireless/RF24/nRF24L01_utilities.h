#ifndef NRF24L01_UTILITIES_H_
#define NRF24L01_UTILITIES_H_

static inline uint8_t bitwiseAND(uint8_t byte, uint8_t mask) {
    return (byte & mask);
}

static inline void bitwiseAND_r(uint8_t &byte, uint8_t mask) {
    byte &= mask;
}

static inline uint8_t bitwiseOR(uint8_t byte, uint8_t mask) {
    return (byte | mask);
}

static inline void bitwiseOR_r(uint8_t &byte, uint8_t mask) {
    byte |= mask;
}

static inline void clearBit(uint8_t &byte, uint8_t bit) {
    byte &= ~(1 << bit);
}

static inline bool readBit(uint8_t byte, uint8_t bit) {
    return (byte & (1 << bit));
}

static inline void setBit(uint8_t &byte, uint8_t bit) {
    byte |= (1 << bit);
}

static inline void shiftLeft(uint8_t &byte, uint8_t amount) {
    byte <<= amount;
}

static inline void shiftRight(uint8_t &byte, uint8_t amount) {
    byte >>= amount;
}

#endif /* NRF24L01_UTILITIES_H_ */
