#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/utils/logging.hpp>

namespace xXx {

template <typename TYPE>
class CircularBuffer {
   private:
    TYPE *elements;
    size_t numberOfElements;
    size_t head;
    size_t tail;

    // Copy assignment operator
    CircularBuffer &operator=(const CircularBuffer &other) = default;

    // Move assignment operator
    CircularBuffer &operator=(CircularBuffer &&other) = default;

   public:
    ~CircularBuffer() = default;

    // Default constructor
    CircularBuffer(TYPE *buffer, size_t capacity);

    // Copy constructor
    CircularBuffer(const CircularBuffer &other);

    // Move constructor
    CircularBuffer(CircularBuffer &&other);

    bool push(TYPE &item);
    bool pop(TYPE &item);

    size_t itemsAvailable();
    size_t slotsAvailable();
};

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(TYPE *elements, size_t numberOfElements)
    : elements(elements), numberOfElements(numberOfElements), head(0), tail(0) {
    assert(elements != NULL);
}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(const CircularBuffer &other)
    : elements(other.elements),
      numberOfElements(other.numberOfElements),
      head(other.head),
      tail(other.tail) {
    assert(elements != NULL);
    assert(head < numberOfElements);
    assert(tail < numberOfElements);
}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(CircularBuffer &&other)
    : elements(other.elements),
      numberOfElements(other.numberOfElements),
      head(other.head),
      tail(other.tail) {
    assert(elements != NULL);
    assert(head < numberOfElements);
    assert(tail < numberOfElements);
}

template <typename TYPE>
bool CircularBuffer<TYPE>::push(TYPE &item) {
    if (slotsAvailable() == 0) return (false);

    memcpy(&elements[head++], &item, sizeof(TYPE));
    head %= numberOfElements;

    return (true);
}

template <typename TYPE>
bool CircularBuffer<TYPE>::pop(TYPE &item) {
    if (itemsAvailable() == 0) return (false);

    memcpy(&item, &elements[tail++], sizeof(TYPE));
    tail %= numberOfElements;

    return (true);
}

template <typename TYPE>
size_t CircularBuffer<TYPE>::itemsAvailable() {
    size_t itemsAvailable;

    if (tail > head) {
        itemsAvailable = (numberOfElements + head - tail);
    } else {
        itemsAvailable = (head - tail);
    }

    return (itemsAvailable);
}

template <typename TYPE>
size_t CircularBuffer<TYPE>::slotsAvailable() {
    return (numberOfElements - itemsAvailable());
}

} /* namespace xXx */

#endif /* CIRCULAR_BUFFER_HPP */
