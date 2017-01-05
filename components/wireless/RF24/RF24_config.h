/*
 * Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#ifndef RF24_CONFIG_H_
#define RF24_CONFIG_H_

#include <FreeRTOS.h>
#include <task.h>

#define _BV(x) (1 << (x))
#define pgm_read_byte(b) (*(b))
#define pgm_read_word(p) (*(p))

static inline void delayMs(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

static inline void delayUs(uint32_t us) {
    // TODO: Find better solution
    delayMs(1);
}

static inline uint32_t getMillis() {
    return (xTaskGetTickCount() * portTICK_PERIOD_MS);
}

#endif // RF24_CONFIG_H_
