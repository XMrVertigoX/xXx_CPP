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

    printf("%lu.%03lu ", seconds(ticks), milliseconds(ticks));

    va_list arguments;
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);

    putchar('\r');
    putchar('\n');
}

void printBuffer(const char *message, void *bytes, size_t numBytes) {
    TickType_t ticks = xTaskGetTickCount();

    printf("%lu.%03lu %s", seconds(ticks), milliseconds(ticks), message);

    for (size_t i = 0; i < numBytes; ++i) {
        printf(" %02x", static_cast<uint8_t *>(bytes)[i]);
    }

    putchar('\r');
    putchar('\n');
}
