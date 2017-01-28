#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <FreeRTOS.h>
#include <queue.h>

namespace xXx {

template <typename TYPE> class Queue {
  public:
    Queue(UBaseType_t size);
    ~Queue();
    BaseType_t dequeue(TYPE &element, TickType_t ticksToWait = portMAX_DELAY,
                       bool isr = false);
    BaseType_t enqueue(TYPE &element, TickType_t ticksToWait = portMAX_DELAY,
                       bool isr = false);

    UBaseType_t freeSlots();
    UBaseType_t usedSlots(bool isr = false);

  private:
    QueueHandle_t _queue;
};

template <typename TYPE> Queue<TYPE>::Queue(UBaseType_t size) : _queue(NULL) {
    _queue = xQueueCreate(size, sizeof(TYPE));
}

template <typename TYPE> Queue<TYPE>::~Queue() {
    vQueueDelete(_queue);
}

template <typename TYPE>
BaseType_t Queue<TYPE>::enqueue(TYPE &element, TickType_t ticksToWait,
                                bool isr) {
    if (isr) {
        return (xQueueSendToBackFromISR(_queue, &element, pdFALSE));
    } else {
        return (xQueueSendToBack(_queue, &element, ticksToWait));
    }
}

template <typename TYPE>
BaseType_t Queue<TYPE>::dequeue(TYPE &element, TickType_t ticksToWait,
                                bool isr) {
    if (isr) {
        return (xQueueReceiveFromISR(_queue, &element, pdFALSE));
    } else {
        return (xQueueReceive(_queue, &element, ticksToWait));
    }
}

template <typename TYPE> UBaseType_t Queue<TYPE>::usedSlots(bool isr) {
    if (isr) {
        return (uxQueueMessagesWaitingFromISR(_queue));
    } else {
        return (uxQueueMessagesWaiting(_queue));
    }
}

template <typename TYPE> UBaseType_t Queue<TYPE>::freeSlots() {
    return (uxQueueSpacesAvailable(_queue));
}

template <typename TYPE> using Queue_Handle_t = Queue<TYPE> *;

} /* namespace xXx */

#endif /* QUEUE_HPP_ */
