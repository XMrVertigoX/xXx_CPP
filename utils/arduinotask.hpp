#ifndef ARDUINOTASK_HPP_
#define ARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

namespace xXx {

class ArduinoTask{
   public:
    virtual ~ArduinoTask() = default;

    virtual void setup() = 0;
    virtual void loop() = 0;

    BaseType_t attach(uint16_t stackDepth, UBaseType_t priority);

   protected:
    TaskHandle_t _handle = NULL;
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
