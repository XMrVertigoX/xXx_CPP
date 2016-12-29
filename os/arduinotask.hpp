#ifndef ARDUINOTASK_HPP_
#define ARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

const uint16_t defaultStackSize   = configMINIMAL_STACK_SIZE;
const uint8_t defaultTaskPriority = tskIDLE_PRIORITY + 1;

namespace xXx {

class ArduinoTask {
  public:
    ArduinoTask(uint16_t stackSize   = defaultStackSize,
                uint8_t taskPriority = defaultTaskPriority);
    virtual ~ArduinoTask();

    virtual void setup() = 0;
    virtual void loop()  = 0;

    void suspend();
    void resume();

  private:
    TaskHandle_t _handle = NULL;

  private:
    static void taskFunction(void *task);
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
