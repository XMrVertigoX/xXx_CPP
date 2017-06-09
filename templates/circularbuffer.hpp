#ifndef CIRCULAR_BUFFER_HPP
#define CIRCULAR_BUFFER_HPP

#include <stdint.h>
#include <string.h>

namespace xXx {

template <typename TYPE>
class CircularBuffer {
   private:
    TYPE *elements;

    size_t maxElements;
    size_t numElements;
    size_t head;
    size_t tail;

   public:
    ~CircularBuffer();

    // Default constructor
    CircularBuffer(size_t maxElements);

    // Copy constructor
    CircularBuffer(const CircularBuffer &other);

    // Copy assignment operator
    CircularBuffer &operator=(const CircularBuffer &other);

    // Move constructor
    CircularBuffer(CircularBuffer &&other);

    // Move assignment operator
    CircularBuffer &operator=(CircularBuffer &&other);

    bool push(const TYPE &element);
    bool pop(TYPE &element);

    size_t itemsAvailable();
    size_t slotsAvailable();
};

template <typename TYPE>
CircularBuffer<TYPE>::~CircularBuffer() {
    delete[] elements;
}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(size_t maxElements)
    : elements(new TYPE[maxElements]), maxElements(maxElements), numElements(0), head(0), tail(0) {}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(const CircularBuffer<TYPE> &other)
    : elements(new TYPE[other.maxElements]),
      maxElements(other.maxElements),
      numElements(other.numElements),
      head(other.head),
      tail(other.tail) {
    memcpy(elements, other.elements, maxElements * sizeof(TYPE));
}

template <typename TYPE>
CircularBuffer<TYPE> &CircularBuffer<TYPE>::operator=(const CircularBuffer<TYPE> &other) {
    if (&other == this) return (*this);

    delete[] elements;

    elements    = new TYPE[other.maxElements];
    maxElements = other.maxElements;
    numElements = other.numElements;
    head        = other.head;
    tail        = other.tail;

    memcpy(elements, other.elements, numElements * sizeof(TYPE));

    return (*this);
}

template <typename TYPE>
CircularBuffer<TYPE>::CircularBuffer(CircularBuffer<TYPE> &&other)
    : elements(other.elements),
      maxElements(other.maxElements),
      numElements(other.numElements),
      head(other.head),
      tail(other.tail) {
    other.elements    = NULL;
    other.maxElements = 0;
    other.numElements = 0;
    other.head        = 0;
    other.tail        = 0;
}

template <typename TYPE>
CircularBuffer<TYPE> &CircularBuffer<TYPE>::operator=(CircularBuffer &&other) {
    if (&other == this) return (*this);

    delete[] elements;

    elements    = other.elements;
    maxElements = other.maxElements;
    numElements = other.numElements;
    head        = other.head;
    tail        = other.tail;

    other.elements    = NULL;
    other.maxElements = 0;
    other.numElements = 0;
    other.head        = 0;
    other.tail        = 0;

    return (*this);
}

template <typename TYPE>
bool CircularBuffer<TYPE>::push(const TYPE &element) {
    if (slotsAvailable() == 0) return (false);

    memcpy(&elements[head], &element, sizeof(TYPE));

    head = (head + 1) % maxElements;
    numElements++;

    return (true);
}

template <typename TYPE>
bool CircularBuffer<TYPE>::pop(TYPE &element) {
    if (itemsAvailable() == 0) return (false);

    memcpy(&element, &elements[tail], sizeof(TYPE));

    tail = (tail + 1) % maxElements;
    numElements--;

    return (true);
}

template <typename TYPE>
size_t CircularBuffer<TYPE>::itemsAvailable() {
    return (numElements);
}

template <typename TYPE>
size_t CircularBuffer<TYPE>::slotsAvailable() {
    return (maxElements - numElements);
}

} /* namespace xXx */

#endif /* CIRCULAR_BUFFER_HPP */
