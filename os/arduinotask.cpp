#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "arduinotask.hpp"

namespace xXx {

ArduinoTask::ArduinoTask(uint16_t stackSize, uint8_t priority, char *friendlyName) : _handle(NULL) {
    taskCreate(stackSize, priority, friendlyName);
}

ArduinoTask::~ArduinoTask() {
    taskDelete();
}

void ArduinoTask::taskFunction(void *self) {
    static_cast<ArduinoTask *>(self)->setup();
    for (;;) static_cast<ArduinoTask *>(self)->loop();
}

void ArduinoTask::taskDelay(TickType_t ticksToDelay) {
    vTaskDelay(ticksToDelay);
}

void ArduinoTask::taskNotifyTake(bool clearCounter, TickType_t ticksToWait) {
    ulTaskNotifyTake(clearCounter, ticksToWait);
}

void ArduinoTask::taskCreate(uint16_t stackSize, uint8_t priority, char *friendlyName) {
    xTaskCreate(taskFunction, friendlyName, stackSize, this, priority, &_handle);
}

void ArduinoTask::taskDelete() {
    vTaskDelete(_handle);
}

void ArduinoTask::taskNotify(uint32_t value, eNotifyAction action) {
    xTaskNotify(_handle, value, action);
}

void ArduinoTask::taskNotifyFromISR(uint32_t value, eNotifyAction action) {
    BaseType_t higherPriorityTaskWoken;

    xTaskNotifyFromISR(_handle, value, action, &higherPriorityTaskWoken);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void ArduinoTask::taskResume() {
    vTaskResume(_handle);
}

void ArduinoTask::taskResumeFromISR() {
    BaseType_t higherPriorityTaskWoken;

    higherPriorityTaskWoken = xTaskResumeFromISR(_handle);
    portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void ArduinoTask::taskSuspend() {
    vTaskSuspend(_handle);
}

char *ArduinoTask::getTaskName() {
    return (pcTaskGetTaskName(_handle));
}

UBaseType_t ArduinoTask::getStackHighWaterMark() {
    return (uxTaskGetStackHighWaterMark(_handle));
}

} /* namespace xXx */
