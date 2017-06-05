#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <assert.h>
#include <stdint.h>
#include <string.h>

namespace xXx {

template <typename TYPE>
class CircularBuffer {
   private:
    TYPE *elements;
    size_t numberOfElements;
    size_t head;
    size_t tail;

   public:
    ~CircularBuffer() = default;

    // Default constructor
    CircularBuffer(TYPE *elements, size_t numberOfElements);

    // Copy constructor
    CircularBuffer(const CircularBuffer &other);

    // Move constructor
    CircularBuffer(CircularBuffer &&other);

    // Copy assignment operator
    CircularBuffer &operator=(const CircularBuffer &other);

    // Move assignment operator
    CircularBuffer &operator=(CircularBuffer &&other);

    bool push(const TYPE &element);
    bool pop(TYPE &element);

    size_t itemsAvailable();
    size_t slotsAvailable();
};

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(TYPE *elements, size_t numberOfElements)
    : elements(elements), numberOfElements(numberOfElements), head(0), tail(0) {
    assert(elements != NULL);
}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(const CircularBuffer<TYPE> &other)
    : elements(other.elements),
      numberOfElements(other.numberOfElements),
      head(other.head),
      tail(other.tail) {}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(CircularBuffer<TYPE> &&other)
    : elements(other.elements),
      numberOfElements(other.numberOfElements),
      head(other.head),
      tail(other.tail) {}

template <typename TYPE>
CircularBuffer<TYPE> &CircularBuffer<TYPE>::operator=(const CircularBuffer<TYPE> &other) {
    if (&other != this) {
        elements         = other.elements;
        numberOfElements = other.numberOfElements;
        head             = other.head;
        tail             = other.tail;
    }

    return *this;
}

template <typename TYPE>
CircularBuffer<TYPE> &CircularBuffer<TYPE>::operator=(CircularBuffer &&other) {
    if (&other != this) {
        elements         = other.elements;
        numberOfElements = other.numberOfElements;
        head             = other.head;
        tail             = other.tail;
    }

    return *this;
}

template <typename TYPE>
bool CircularBuffer<TYPE>::push(const TYPE &element) {
    if (slotsAvailable() == 0) return (false);

    memcpy(&elements[head++], &element, sizeof(TYPE));
    head %= numberOfElements;

    return (true);
}

template <typename TYPE>
bool CircularBuffer<TYPE>::pop(TYPE &element) {
    if (itemsAvailable() == 0) return (false);

    memcpy(&element, &elements[tail++], sizeof(TYPE));
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
    size_t slotsAvailable;

    slotsAvailable = (numberOfElements - itemsAvailable());

    return (slotsAvailable);
}

} /* namespace xXx */

#endif /* CIRCULAR_BUFFER_HPP */
