#ifndef ARDUINOTASK_HPP_
#define ARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

namespace xXx {

class ArduinoTask {
  public:
    ArduinoTask(uint16_t stack, UBaseType_t priority);
    virtual ~ArduinoTask();

    virtual void setup() = 0;
    virtual void loop() = 0;

    void suspend();
    void resume();

  private:
    TaskHandle_t _handle;
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
