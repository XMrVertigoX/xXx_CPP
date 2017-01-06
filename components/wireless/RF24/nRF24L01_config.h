#ifndef NRF24L01_CONFIG_H_
#define NRF24L01_CONFIG_H_

#include <asf.h>

#include <FreeRTOS.h>
#include <task.h>

#define _BV(x) (1 << (x))
#define pgm_read_byte(b) (*(b))
#define pgm_read_word(p) (*(p))

static inline void delayMs(uint32_t ms) {
    delay_ms(ms);
}

static inline void delayUs(uint32_t us) {
    delay_us(us);
}

static inline uint32_t getMillis() {
    return (xTaskGetTickCount() * portTICK_PERIOD_MS);
}

#endif // NRF24L01_CONFIG_H_
