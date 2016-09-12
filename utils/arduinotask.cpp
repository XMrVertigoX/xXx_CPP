#include <cstdlib>

#include <FreeRTOS.h>
#include <task.h>

#include <xXx/utils/arduinotask.hpp>

#define TASK_FUNCTION                                                 \
    [](void *pvParameters) {                                          \
        ArduinoTask *task = static_cast<ArduinoTask *>(pvParameters); \
        task->setup();                                                \
        for (;;) {                                                    \
            task->loop();                                             \
        }                                                             \
    }

namespace xXx {

BaseType_t ArduinoTask::attach(uint16_t stack, UBaseType_t priority) {
    return (xTaskCreate(TASK_FUNCTION, NULL, stack, this, priority, &_handle));
}

} /* namespace xXx */
