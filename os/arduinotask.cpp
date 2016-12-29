#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "arduinotask.hpp"

namespace xXx {

ArduinoTask::ArduinoTask(uint16_t stackSize, uint8_t taskPriority) {
    xTaskCreate(taskFunction, NULL, stackSize, this, taskPriority, &_handle);
}

ArduinoTask::~ArduinoTask() {
    vTaskDelete(_handle);
}

void ArduinoTask::resume() {
    vTaskResume(_handle);
}

void ArduinoTask::suspend() {
    vTaskSuspend(_handle);
}

void ArduinoTask::taskFunction(void *task) {
    static_cast<ArduinoTask *>(task)->setup();
    for (;;) static_cast<ArduinoTask *>(task)->loop();
}

} /* namespace xXx */
