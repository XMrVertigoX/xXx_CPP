#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "logging.hpp"

static inline uint32_t ticks2ms(TickType_t ticks) {
    return (ticks * portTICK_PERIOD_MS);
}

static inline uint32_t seconds(TickType_t ticks) {
    return (ticks2ms(ticks) / 1000);
}

static inline uint32_t milliseconds(TickType_t ticks) {
    return (ticks2ms(ticks) % 1000);
}

void printFormat(const char *format, ...) {
    TickType_t ticks = xTaskGetTickCount();

    printf("%d.%03d ", seconds(ticks), milliseconds(ticks));

    va_list arguments;
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);

    putchar('\n');
}

void printBuffer(const char *message, uint8_t bytes[], size_t numBytes) {
    TickType_t ticks = xTaskGetTickCount();

    printf("%d.%03d %s", seconds(ticks), milliseconds(ticks), message);

    for (int i = 0; i < numBytes; ++i) {
        printf(" %02x", bytes[i]);
    }

    putchar('\n');
}
