#ifndef BITOPERATIONS_HPP_
#define BITOPERATIONS_HPP_

// ----- Basic bitwise operations ---------------------------------------------

template <typename TYPE>
inline TYPE AND(TYPE byte, TYPE mask) {
    return (byte bitand mask);
}

template <typename TYPE>
inline void AND_eq(TYPE &byte, TYPE mask) {
    byte and_eq mask;
}

template <typename TYPE>
inline TYPE OR(TYPE byte, TYPE mask) {
    return (byte bitor mask);
}

template <typename TYPE>
inline void OR_eq(TYPE &byte, TYPE mask) {
    byte or_eq mask;
}

template <typename TYPE>
inline TYPE XOR(TYPE byte, TYPE mask) {
    return (byte xor mask);
}

template <typename TYPE>
inline void XOR_eq(TYPE &byte, TYPE mask) {
    byte xor_eq mask;
}

template <typename TYPE>
inline TYPE LEFT(TYPE byte, char amount) {
    return (byte << amount);
}

template <typename TYPE>
inline void LEFT_eq(TYPE &byte, char amount) {
    byte <<= amount;
}

template <typename TYPE>
inline TYPE RIGHT(TYPE byte, char amount) {
    return (byte >> amount);
}

template <typename TYPE>
inline void RIGHT_eq(TYPE &byte, char amount) {
    byte >>= amount;
}

template <typename TYPE>
inline TYPE INVERT(TYPE byte) {
    return (compl(byte));
}

template <typename TYPE>
inline void INVERT_eq(TYPE &byte) {
    byte = compl(byte);
}

// ----- Advanced bitwise operations ------------------------------------------

template <typename TYPE>
inline TYPE clearBit(TYPE byte, TYPE bit) {
    return (AND<TYPE>(byte, INVERT<TYPE>(LEFT<TYPE>(1, bit))));
}

template <typename TYPE>
inline void clearBit_eq(TYPE &byte, TYPE bit) {
    AND_eq<TYPE>(byte, INVERT<TYPE>(LEFT<TYPE>(1, bit)));
}

template <typename TYPE>
inline TYPE setBit(TYPE byte, TYPE bit) {
    return (OR<TYPE>(byte, (LEFT<TYPE>(1, bit))));
}

template <typename TYPE>
inline void setBit_eq(TYPE &byte, TYPE bit) {
    OR_eq<TYPE>(byte, (LEFT<TYPE>(1, bit)));
}

template <typename TYPE>
inline bool readBit(TYPE byte, TYPE bit) {
    return (AND<TYPE>(byte, (LEFT<TYPE>(1, bit))));
}

#endif /* BITOPERATIONS_HPP_ */
