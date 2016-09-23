#include <cstdlib>

#include <FreeRTOS.h>
#include <task.h>

#include "arduinotask.hpp"

#define ARDUINO                                                    \
    [](void *pvParameters) {                                       \
        static_cast<ArduinoTask *>(pvParameters)->setup();         \
        for (;;) static_cast<ArduinoTask *>(pvParameters)->loop(); \
    }

namespace xXx {

BaseType_t ArduinoTask::attach(uint16_t stack, UBaseType_t priority) {
    return (xTaskCreate(ARDUINO, NULL, stack, this, priority, &_handle));
}

} /* namespace xXx */
