#ifndef NRF24L01_CONFIG_TEMPLATE_H_
#define NRF24L01_CONFIG_TEMPLATE_H_

#define _BV(x) (1 << (x))
#define pgm_read_byte(b) (*(b))
#define pgm_read_word(p) (*(p))

static inline void delayMs(uint32_t ms) {}

static inline void delayUs(uint32_t us) {}

static inline uint32_t getMillis() {
    return (0);
}

#endif // NRF24L01_CONFIG_TEMPLATE_H_
