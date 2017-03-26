#ifndef SIMPLETASK_HPP_
#define SIMPLETASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

const uint16_t defaultStackSize = configMINIMAL_STACK_SIZE;
const uint8_t defaultPriority   = tskIDLE_PRIORITY + 1;

namespace xXx {

class SimpleTask {
   private:
    TaskHandle_t _handle = NULL;
    uint8_t _priority    = defaultPriority;
    uint16_t _stackSize  = defaultStackSize;

    virtual void setup() = 0;
    virtual void loop()  = 0;

   protected:
    virtual ~SimpleTask();

    void taskDelay(TickType_t ticksToDelay);
    void taskNotifyTake(BaseType_t clearCounter = pdTRUE, TickType_t ticksToWait = portMAX_DELAY);

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

#endif /* SIMPLETASK_HPP_ */
