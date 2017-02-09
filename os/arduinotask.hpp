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

    void taskDelay(TickType_t ticksToDelay);
    void taskNotifyTake(bool clearCounter = false, TickType_t ticksToWait = portMAX_DELAY);

  public:
    ArduinoTask(uint16_t stackSize = defaultStackSize, uint8_t priority = defaultPriority,
                char *friendlyName = NULL);
    virtual ~ArduinoTask();

    virtual void setup() = 0;
    virtual void loop()  = 0;

    void taskCreate(uint16_t stackSize, uint8_t priority, char *friendlyName);
    void taskDelete();
    void taskNotify(uint32_t value = 0, eNotifyAction action = eIncrement);
    void taskNotifyFromISR(uint32_t value = 0, eNotifyAction action = eIncrement);
    void taskResume();
    void taskResumeFromISR();
    void taskSuspend();

    char *getTaskName();
    UBaseType_t getStackHighWaterMark();
};

} /* namespace xXx */

#endif /* ARDUINOTASK_HPP_ */
