#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <FreeRTOS.h>
#include <queue.h>

namespace xXx {

template <typename TYPE> class Queue {
  public:
    Queue(UBaseType_t numElements);
    ~Queue();
    BaseType_t dequeue(TYPE &element, bool isr = false);
    BaseType_t enqueue(TYPE &element, bool isr = false);

    UBaseType_t freeSlots(bool isr = false);
    UBaseType_t usedSlots();

  private:
    QueueHandle_t _queue;
};

template <typename TYPE> Queue<TYPE>::Queue(UBaseType_t numElements) {
    _queue = xQueueCreate(numElements, sizeof(TYPE));
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

template <typename TYPE> UBaseType_t Queue<TYPE>::freeSlots(bool isr) {
    if (isr) {
        return (uxQueueMessagesWaitingFromISR(_queue));
    } else {
        return (uxQueueMessagesWaiting(_queue));
    }
}

template <typename TYPE> UBaseType_t Queue<TYPE>::usedSlots() {
    return (uxQueueSpacesAvailable(_queue));
}

} /* namespace xXx */

#endif /* QUEUE_HPP_ */
