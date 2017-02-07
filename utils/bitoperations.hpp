#ifndef BITOPERATIONS_HPP_
#define BITOPERATIONS_HPP_

#include <stdint.h>

// ----- Basic bitwise operations ---------------------------------------------

template <typename TYPE> inline TYPE AND(TYPE byte, TYPE mask) {
    return (byte bitand mask);
}

template <typename TYPE> inline void AND_eq(TYPE byte, TYPE mask) {
    byte and_eq mask;
}

template <typename TYPE> inline TYPE OR(TYPE byte, TYPE mask) {
    return (byte bitor mask);
}

template <typename TYPE> inline void OR_eq(TYPE &byte, TYPE mask) {
    byte or_eq mask;
}

template <typename TYPE> inline TYPE XOR(TYPE byte, TYPE mask) {
    return (byte xor mask);
}

template <typename TYPE> inline void XOR_eq(TYPE &byte, TYPE mask) {
    byte xor_eq mask;
}

template <typename TYPE> inline TYPE LEFT(uint8_t byte, uint8_t amount) {
    return (byte << amount);
}

template <typename TYPE> inline void LEFT_eq(uint8_t &byte, uint8_t amount) {
    byte <<= amount;
}

template <typename TYPE> inline TYPE RIGHT(uint8_t byte, uint8_t amount) {
    return (byte >> amount);
}

template <typename TYPE> inline void RIGHT_eq(uint8_t &byte, uint8_t amount) {
    byte >>= amount;
}

template <typename TYPE> inline TYPE INVERT(TYPE byte) {
    return (compl(byte));
}

template <typename TYPE> inline void INVERT_eq(TYPE &byte) {
    byte = compl(byte);
}

// ----- Advanced bitwise operations ------------------------------------------

template <typename TYPE> inline TYPE clearBit(TYPE byte, TYPE bit) {
    return (byte bitand compl(LEFT<TYPE>(1, bit)));
}

template <typename TYPE> inline void clearBit_eq(TYPE &byte, TYPE bit) {
    byte bitand compl(LEFT<TYPE>(1, bit));
}

template <typename TYPE> inline TYPE setBit(TYPE byte, TYPE bit) {
    return (byte bitor (LEFT<TYPE>(1, bit)));
}

template <typename TYPE> inline void setBit_eq(TYPE &byte, TYPE bit) {
    byte or_eq (LEFT<TYPE>(1, bit));
}

template <typename TYPE> inline bool readBit(TYPE byte, TYPE bit) {
    return (byte bitand (LEFT<TYPE>(1, bit)));
}

#endif /* BITOPERATIONS_HPP_ */
