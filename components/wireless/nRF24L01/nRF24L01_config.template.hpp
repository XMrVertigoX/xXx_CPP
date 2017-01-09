#ifndef NRF24L01_CONFIG_TEMPLATE_HPP_
#define NRF24L01_CONFIG_TEMPLATE_HPP_

#include <stdint.h>

static inline void delayMs(uint32_t ms) {}

static inline void delayUs(uint32_t us) {}

static inline uint32_t getMillis() {
    return (0);
}

#endif // NRF24L01_CONFIG_TEMPLATE_HPP_
