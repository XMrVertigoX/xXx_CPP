#ifndef SIMPLETASK_HPP_
#define SIMPLETASK_HPP_

#include <FreeRTOS.h>
#include <task.h>

enum Task_Priority_t {
    Task_Priority_IDLE = tskIDLE_PRIORITY,
    Task_Priority_LOW,
    Task_Priority_MID,
    Task_Priority_HIGH
};

namespace xXx {

class SimpleTask {
   private:
    TaskHandle_t _handle = NULL;
    uint8_t _priority    = Task_Priority_MID;
    uint16_t _stackSize  = configMINIMAL_STACK_SIZE;

    virtual void setup() = 0;
    virtual void loop()  = 0;

   protected:
    virtual ~SimpleTask();

    void delay(TickType_t ticksToDelay);
    void notifyTake(BaseType_t clearCounter = pdTRUE, TickType_t ticksToWait = portMAX_DELAY);

   public:
    void create(uint16_t stackSize = configMINIMAL_STACK_SIZE,
                uint8_t priority   = Task_Priority_MID);
    void destroy();
    void notify();
    void notifyFromISR();
    void resume();
    void resumeFromISR();
    void suspend();

    UBaseType_t getStackHighWaterMark();
};

} /* namespace xXx */

#endif /* SIMPLETASK_HPP_ */
