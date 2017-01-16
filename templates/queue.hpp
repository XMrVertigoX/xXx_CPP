#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <FreeRTOS.h>
#include <queue.h>

namespace xXx {

template <typename TYPE> class Queue {
  public:
    Queue(UBaseType_t size);
    ~Queue();
    BaseType_t dequeue(TYPE &element, bool isr = false);
    BaseType_t enqueue(TYPE &element, bool isr = false);

    UBaseType_t freeSlots();
    UBaseType_t size();
    UBaseType_t usedSlots(bool isr = false);

  private:
    QueueHandle_t _queue;
    UBaseType_t _size;
};

template <typename TYPE>
Queue<TYPE>::Queue(UBaseType_t size) : _queue(NULL), _size(size) {
    _queue = xQueueCreate(size, sizeof(TYPE));
}

template <typename TYPE> Queue<TYPE>::~Queue() {
    vQueueDelete(_queue);
}

template <typename TYPE>
BaseType_t Queue<TYPE>::enqueue(TYPE &element, bool isr) {
    if (isr) {
        return (xQueueSendToBackFromISR(_queue, &element, pdFALSE));
    } else {
        return (xQueueSendToBack(_queue, &element, 0));
    }
}

template <typename TYPE>
BaseType_t Queue<TYPE>::dequeue(TYPE &element, bool isr) {
    if (isr) {
        return (xQueueReceiveFromISR(_queue, &element, pdFALSE));
    } else {
        return (xQueueReceive(_queue, &element, 0));
    }
}

template <typename TYPE> UBaseType_t Queue<TYPE>::usedSlots(bool isr) {
    if (isr) {
        return (uxQueueMessagesWaitingFromISR(_queue));
    } else {
        return (uxQueueMessagesWaiting(_queue));
    }
}

template <typename TYPE> UBaseType_t Queue<TYPE>::size() {
    return (_size);
}

template <typename TYPE> UBaseType_t Queue<TYPE>::freeSlots() {
    return (uxQueueSpacesAvailable(_queue));
}

template <typename TYPE> using Queue_Handle_t = Queue<TYPE> *;

} /* namespace xXx */

#endif /* QUEUE_HPP_ */
