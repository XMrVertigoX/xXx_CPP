#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <stdint.h>
#include <string.h>

#include <xXx/utils/logging.hpp>

namespace xXx {

template <typename TYPE>
class CircularBuffer {
   private:
    TYPE *data;

    size_t head = 0;
    size_t tail = 0;

    size_t capacity;

   public:
    CircularBuffer(size_t size);
    ~CircularBuffer();

    bool pushLeft(TYPE &item);
    bool popRight(TYPE &item);

    size_t itemsAvailable();
    size_t slotsAvailable();
};

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(size_t size) : capacity(size) {
    data = new TYPE[capacity];
}

template <typename TYPE>
CircularBuffer<TYPE>::~CircularBuffer() {
    delete data;
}

template <typename TYPE>
bool CircularBuffer<TYPE>::pushLeft(TYPE &item) {
    if (slotsAvailable() == 0) return (false);

    memcpy(&data[head++], &item, sizeof(TYPE));
    head %= capacity;

    return (true);
}

template <typename TYPE>
bool CircularBuffer<TYPE>::popRight(TYPE &item) {
    if (itemsAvailable() == 0) return (false);

    memcpy(&item, &data[tail++], sizeof(TYPE));
    tail %= capacity;

    return (true);
}

template <typename TYPE>
size_t CircularBuffer<TYPE>::itemsAvailable() {
    if (tail > head) {
        return (capacity + head - tail);
    } else {
        return (head - tail);
    }
}

template <typename TYPE>
size_t CircularBuffer<TYPE>::slotsAvailable() {
    if (tail > head) {
        return (tail - head);
    } else {
        return (capacity + tail - head);
    }
}

} /* namespace xXx */

#endif /* CIRCULAR_BUFFER_HPP */
