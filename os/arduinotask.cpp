#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "arduinotask.hpp"

#define ARDUINO                                                                \
    [](void *pvParameters) {                                                   \
        static_cast<ArduinoTask *>(pvParameters)->setup();                     \
        for (;;) {                                                             \
            static_cast<ArduinoTask *>(pvParameters)->loop();                  \
        }                                                                      \
    }

namespace xXx {

ArduinoTask::ArduinoTask(uint16_t stack, UBaseType_t priority) : _handle(NULL) {
    xTaskCreate(ARDUINO, NULL, stack, this, priority, &_handle);
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

} /* namespace xXx */
