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

void ArduinoTask::suspend() {
    vTaskSuspend(_handle);
}

void ArduinoTask::resume() {
    vTaskResume(_handle);
}

// ----- private static -------------------------------------------------------

void ArduinoTask::taskFunction(void *pvParameters) {
    static_cast<ArduinoTask *>(pvParameters)->setup();
    for (;;) static_cast<ArduinoTask *>(pvParameters)->loop();
}

} /* namespace xXx */
