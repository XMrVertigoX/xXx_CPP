#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "arduinotask.hpp"

namespace xXx {

ArduinoTask::ArduinoTask(uint16_t stackSize, uint8_t priority, char *friendlyName) : _handle(NULL) {
    xTaskCreate(taskFunction, friendlyName, stackSize, this, priority, &_handle);
}

ArduinoTask::~ArduinoTask() {
    vTaskDelete(_handle);
}

void ArduinoTask::taskFunction(void *self) {
    static_cast<ArduinoTask *>(self)->setup();
    for (;;) static_cast<ArduinoTask *>(self)->loop();
}

void ArduinoTask::notify(uint32_t value, eNotifyAction action) {
    xTaskNotify(_handle, value, action);
}

void ArduinoTask::notifyFromISR(uint32_t value, eNotifyAction action) {
    BaseType_t higherPriorityTaskWoken;

    xTaskNotifyFromISR(_handle, value, action, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void ArduinoTask::notifyTake(bool clearCounter, TickType_t ticksToWait) {
    ulTaskNotifyTake(clearCounter, ticksToWait);
}

void ArduinoTask::resume() {
    vTaskResume(_handle);
}

void ArduinoTask::resumeFromISR() {
    BaseType_t higherPriorityTaskWoken;

    higherPriorityTaskWoken = xTaskResumeFromISR(_handle);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void ArduinoTask::suspend() {
    vTaskSuspend(_handle);
}

char *ArduinoTask::getTaskName() {
    return (pcTaskGetTaskName(_handle));
}

UBaseType_t ArduinoTask::getStackHighWaterMark() {
    return (uxTaskGetStackHighWaterMark(_handle));
}

} /* namespace xXx */
