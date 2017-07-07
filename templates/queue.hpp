#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

namespace xXx {

template <typename TYPE>
class Queue {
   public:
    Queue(UBaseType_t size);
    ~Queue();
    BaseType_t dequeue(TYPE &element, TickType_t ticksToWait = portMAX_DELAY);
    BaseType_t dequeueFromISR(TYPE &element);
    BaseType_t enqueue(TYPE &element, TickType_t ticksToWait = portMAX_DELAY);
    BaseType_t enqueueFromISR(TYPE &element);
    UBaseType_t queueSpacesAvailable();
    UBaseType_t queueSpacesAvailableFromISR();
    BaseType_t queuePeek(TYPE &element, TickType_t ticksToWait = portMAX_DELAY);
    BaseType_t queuePeekFromISR(TYPE &element);
    UBaseType_t queueMessagesWaiting();
    UBaseType_t queueMessagesWaitingFromISR();

   private:
    QueueHandle_t _queue;
};

template <typename TYPE>
Queue<TYPE>::Queue(UBaseType_t size)
    : _queue(NULL) {
    _queue = xQueueCreate(size, sizeof(TYPE));
}

template <typename TYPE>
Queue<TYPE>::~Queue() {
    vQueueDelete(_queue);
}

template <typename TYPE>
BaseType_t Queue<TYPE>::enqueue(TYPE &element, TickType_t ticksToWait) {
    return (xQueueSendToBack(_queue, &element, ticksToWait));
}

template <typename TYPE>
BaseType_t Queue<TYPE>::enqueueFromISR(TYPE &element) {
    BaseType_t higherPriorityTaskWoken, success;

    success = xQueueSendToBackFromISR(_queue, &element, &higherPriorityTaskWoken);

    if (success) {
        taskYIELD();
    }

    return (success);
}

template <typename TYPE>
BaseType_t Queue<TYPE>::dequeue(TYPE &element, TickType_t ticksToWait) {
    return (xQueueReceive(_queue, &element, ticksToWait));
}

template <typename TYPE>
BaseType_t Queue<TYPE>::dequeueFromISR(TYPE &element) {
    BaseType_t higherPriorityTaskWoken, success;

    success = xQueueReceiveFromISR(_queue, &element, &higherPriorityTaskWoken);

    if (success) {
        taskYIELD();
    }

    return (success);
}

template <typename TYPE>
BaseType_t Queue<TYPE>::queuePeek(TYPE &element, TickType_t ticksToWait) {
    return (xQueuePeek(_queue, &element, ticksToWait));
}

template <typename TYPE>
BaseType_t Queue<TYPE>::queuePeekFromISR(TYPE &element) {
    return (xQueuePeekFromISR(_queue, &element));
}

template <typename TYPE>
UBaseType_t Queue<TYPE>::queueMessagesWaiting() {
    return (uxQueueMessagesWaiting(_queue));
}

template <typename TYPE>
UBaseType_t Queue<TYPE>::queueMessagesWaitingFromISR() {
    return (uxQueueMessagesWaitingFromISR(_queue));
}

template <typename TYPE>
UBaseType_t Queue<TYPE>::queueSpacesAvailable() {
    return (uxQueueSpacesAvailable(_queue));
}

template <typename TYPE>
UBaseType_t Queue<TYPE>::queueSpacesAvailableFromISR() {
    return (uxQueueSpacesAvailable(_queue));
}

} /* namespace xXx */

#endif /* QUEUE_HPP_ */
