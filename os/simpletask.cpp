#include <assert.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "simpletask.hpp"

#define __INFINITE_LOOP for (;;)

namespace xXx {

SimpleTask::~SimpleTask() {
    destroy();
}

void SimpleTask::delay(TickType_t ticksToDelay) {
    vTaskDelay(ticksToDelay);
}

void SimpleTask::notifyTake(BaseType_t clearCounter, TickType_t ticksToWait) {
    ulTaskNotifyTake(clearCounter, ticksToWait);
}

void SimpleTask::create(uint16_t stackSize, uint8_t priority) {
    auto taskFunction = [](void *simpleTask) {
        static_cast<SimpleTask *>(simpleTask)->setup();
        __INFINITE_LOOP static_cast<SimpleTask *>(simpleTask)->loop();
    };

    BaseType_t error = xTaskCreate(taskFunction, NULL, stackSize, this, priority, &_handle);
    assert(error == pdPASS);
}

void SimpleTask::destroy() {
    vTaskDelete(_handle);
}

void SimpleTask::notify() {
    xTaskNotifyGive(_handle);
}

void SimpleTask::notifyFromISR() {
    BaseType_t higherPriorityTaskWoken;

    vTaskNotifyGiveFromISR(_handle, &higherPriorityTaskWoken);

    if (higherPriorityTaskWoken) {
        taskYIELD();
    }
}

void SimpleTask::resume() {
    vTaskResume(_handle);
}

void SimpleTask::resumeFromISR() {
    BaseType_t higherPriorityTaskWoken;

    higherPriorityTaskWoken = xTaskResumeFromISR(_handle);

    if (higherPriorityTaskWoken) {
        taskYIELD();
    }
}

void SimpleTask::suspend() {
    vTaskSuspend(_handle);
}

UBaseType_t SimpleTask::getStackHighWaterMark() {
    return (uxTaskGetStackHighWaterMark(_handle));
}

} /* namespace xXx */
