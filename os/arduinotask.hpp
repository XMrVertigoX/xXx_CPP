#ifndef ARDUINOTASK_HPP_
#define ARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

// clang-format off
#define DEFAULT_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#define DEFAULT_TASK_PRIORITY (tskIDLE_PRIORITY + 1)
// clang-format on

namespace xXx {

class ArduinoTask {
  public:
    ArduinoTask(uint16_t stackSize   = DEFAULT_STACK_SIZE,
                uint8_t taskPriority = DEFAULT_TASK_PRIORITY);
    virtual ~ArduinoTask();

    virtual void setup() = 0;
    virtual void loop()  = 0;

    void suspend();
    void resume();

  private:
    static void taskFunction(void *pvParameters);

    TaskHandle_t _handle = NULL;
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
