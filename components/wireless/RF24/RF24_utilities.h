#ifndef RF24_UTILITIES_H_
#define RF24_UTILITIES_H_

static inline void clearBit(uint8_t &byte, uint8_t bit) {
    byte &= ~(1 << bit);
}

static inline bool readBit(uint8_t byte, uint8_t bit) {
    return (byte & (1 << bit));
}

static inline void setBit(uint8_t &byte, uint8_t bit) {
    byte |= (1 << bit);
}

static inline void bitwiseAND(uint8_t &byte, uint8_t mask) {
    byte &= mask;
}

static inline void bitwiseOR(uint8_t &byte, uint8_t mask) {
    byte |= mask;
}

#endif /* RF24_UTILITIES_H_ */
