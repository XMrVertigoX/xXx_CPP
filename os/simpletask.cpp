#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include <xXx/os/simpletask.hpp>

namespace xXx {

SimpleTask::~SimpleTask() {
    taskDelete();
}

void SimpleTask::taskDelay(TickType_t ticksToDelay) {
    vTaskDelay(ticksToDelay);
}

void SimpleTask::taskNotifyTake(BaseType_t clearCounter, TickType_t ticksToWait) {
    ulTaskNotifyTake(clearCounter, ticksToWait);
}

void SimpleTask::taskCreate(uint16_t stackSize, uint8_t priority) {
    auto taskFunction = [](void *simpleTask) {
        static_cast<SimpleTask *>(simpleTask)->setup();
        for (;;) static_cast<SimpleTask *>(simpleTask)->loop();
    };

    xTaskCreate(taskFunction, NULL, stackSize, this, priority, &_handle);
}

void SimpleTask::taskDelete() {
    vTaskDelete(_handle);
}

void SimpleTask::taskNotify(uint32_t value, eNotifyAction action) {
    xTaskNotify(_handle, value, action);
}

void SimpleTask::taskNotifyFromISR(uint32_t value, eNotifyAction action) {
    BaseType_t higherPriorityTaskWoken;

    xTaskNotifyFromISR(_handle, value, action, &higherPriorityTaskWoken);

    if (higherPriorityTaskWoken) {
        taskYIELD();
    }
}

void SimpleTask::taskResume() {
    vTaskResume(_handle);
}

void SimpleTask::taskResumeFromISR() {
    BaseType_t higherPriorityTaskWoken;

    higherPriorityTaskWoken = xTaskResumeFromISR(_handle);

    if (higherPriorityTaskWoken) {
        taskYIELD();
    }
}

void SimpleTask::taskSuspend() {
    vTaskSuspend(_handle);
}

UBaseType_t SimpleTask::getStackHighWaterMark() {
    return (uxTaskGetStackHighWaterMark(_handle));
}

} /* namespace xXx */
