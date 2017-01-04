/*
 * Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#ifndef RF24_CONFIG_H_
#define RF24_CONFIG_H_

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

/*
 * Compatibility functions
 */
static inline void __delayMs(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

static inline void __delayUs(uint32_t us) {
    // Todo: Find better solution
    __delayMs(1);
}

static inline uint32_t __millis() {
    return (xTaskGetTickCount() * portTICK_PERIOD_MS);
}

#define PSTR(x) (x)
#define PROGMEM

#define PRIPSTR "%s"

// typedef char const char;
// typedef uint16_t prog_uint16_t;

#endif // RF24_CONFIG_H_
