#ifndef ARDUINOTASK_HPP_
#define ARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

const uint16_t defaultStackSize = configMINIMAL_STACK_SIZE;
const uint8_t defaultPriority   = tskIDLE_PRIORITY + 1;

namespace xXx {

class ArduinoTask {
  private:
    static void taskFunction(void *self);

  protected:
    TaskHandle_t _handle;

    ArduinoTask(uint16_t stackSize = defaultStackSize, uint8_t priority = defaultPriority);
    virtual ~ArduinoTask();

    virtual void setup() = 0;
    virtual void loop()  = 0;

    void taskDelay(TickType_t ticksToDelay);
    void taskNotifyTake(bool clearCounter = false, TickType_t ticksToWait = portMAX_DELAY);

  public:
    void taskCreate(uint16_t stackSize = defaultStackSize, uint8_t priority = defaultPriority);
    void taskDelete();
    void taskNotify(uint32_t value = 0, eNotifyAction action = eIncrement);
    void taskNotifyFromISR(uint32_t value = 0, eNotifyAction action = eIncrement);
    void taskResume();
    void taskResumeFromISR();
    void taskSuspend();

    UBaseType_t getStackHighWaterMark();
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
