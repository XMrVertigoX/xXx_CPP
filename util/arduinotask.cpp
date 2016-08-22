#include <cstdlib>

#include <FreeRTOS.h>
#include <task.h>

#include "arduinotask.hpp"

#define TASK_FUNCTION                                                 \
    [](void *pvParameters) {                                          \
        ArduinoTask *task = static_cast<ArduinoTask *>(pvParameters); \
        task->setup();                                                \
        for (;;) {                                                    \
            task->loop();                                             \
        }                                                             \
    }

BaseType_t ArduinoTask::attachToScheduler(uint16_t stackDepth,
                                          UBaseType_t priority) {
    BaseType_t status =
        xTaskCreate(TASK_FUNCTION, _name, stackDepth, this, priority, _handle);

    return (status);
}
