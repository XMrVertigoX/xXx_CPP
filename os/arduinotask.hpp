#ifndef ARDUINOTASK_HPP_
#define ARDUINOTASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

static const uint16_t defaultStackSize = configMINIMAL_STACK_SIZE;
static const uint8_t defaultPriority   = tskIDLE_PRIORITY + 1;

namespace xXx {

class ArduinoTask {
  private:
    static void taskFunction(void *self);

  protected:
    TaskHandle_t _handle;

    void notifyTake(bool clearCounter = false, TickType_t ticksToWait = portMAX_DELAY);

  public:
    ArduinoTask(uint16_t stackSize = defaultStackSize, uint8_t priority = defaultPriority,
                char *friendlyName = NULL);
    virtual ~ArduinoTask();

    virtual void setup() = 0;
    virtual void loop()  = 0;

    void notify(uint32_t value = 0, eNotifyAction action = eIncrement);
    void notifyFromISR(uint32_t value = 0, eNotifyAction action = eIncrement);
    void suspend();
    void resume();
    void resumeFromISR();

    char *getTaskName();
    UBaseType_t getStackHighWaterMark();
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
