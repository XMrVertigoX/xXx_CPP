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

ArduinoTask::~ArduinoTask() {}

BaseType_t ArduinoTask::attachToScheduler(const char *const name,
                                          const uint16_t stackDepth,
                                          UBaseType_t priority,
                                          TaskHandle_t *const handle) {
    BaseType_t status =
        xTaskCreate(TASK_FUNCTION, name, stackDepth, this, priority, handle);

    return (status);
}
