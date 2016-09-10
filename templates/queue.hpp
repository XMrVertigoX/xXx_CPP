#ifndef QUEUE_HPP_
#define QUEUE_HPP_

#include <FreeRTOS.h>
#include <queue.h>

namespace xXx {

template <typename TYPE>
class Queue {
   public:
    Queue(UBaseType_t numElements);
    ~Queue();
    BaseType_t enqueue(TYPE &element, bool isr = false);
    BaseType_t dequeue(TYPE &element, bool isr = false);

   private:
    QueueHandle_t _queue;
};

template <typename TYPE>
Queue<TYPE>::Queue(UBaseType_t numElements) {
    _queue = xQueueCreate(numElements, sizeof(TYPE));
}

template <typename TYPE>
Queue<TYPE>::~Queue() {
    vQueueDelete(_queue);
}

template <typename TYPE>
BaseType_t Queue<TYPE>::enqueue(TYPE &element, bool isr) {
    if (isr) {
        xQueueSendFromISR(_queue, &element, pdFALSE);
    } else {
        xQueueSend(_queue, &element, 0);
    }
}

template <typename TYPE>
BaseType_t Queue<TYPE>::dequeue(TYPE &element, bool isr) {
    if (isr) {
        xQueueReceiveFromISR(_queue, &element, pdFALSE);
    } else {
        xQueueReceive(_queue, &element, 0);
    }
}

} /* namespace xXx */

#endif /* QUEUE_HPP_ */
