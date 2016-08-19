#ifndef IARDUINOTASK_HPP_
#define IARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

#include "staticsingleton.hpp"

class ArduinoTask {
   public:
    virtual void setup() = 0;
    virtual void loop() = 0;

    BaseType_t attachToScheduler(const char *const name,
                                 const uint16_t stackDepth,
                                 UBaseType_t priority,
                                 TaskHandle_t *const handle = NULL);

   protected:
    virtual ~ArduinoTask();
};

#endif /* IARDUINOTASK_HPP_ */
