#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "logging.hpp"

static const size_t bytesPerLine = 16;

static inline uint32_t ticks2ms(TickType_t ticks) {
    return (ticks * portTICK_PERIOD_MS);
}

static inline uint32_t getSeconds(TickType_t ticks) {
    return (ticks2ms(ticks) / 1000);
}

static inline uint32_t getMilliseconds(TickType_t ticks) {
    return (ticks2ms(ticks) % 1000);
}

static inline void printTime() {
    TickType_t ticks      = xTaskGetTickCount();
    uint32_t seconds      = getSeconds(ticks);
    uint32_t milliseconds = getMilliseconds(ticks);

    printf("[%5lu.%03lu] ", seconds, milliseconds);
}

namespace xXx {

void hexdump(const void *bytes, size_t numBytes) {
    for (size_t i = 0; i < numBytes; i += bytesPerLine) {
        printf("0x%08x:", i);

        for (size_t j = i; j < (i + bytesPerLine); j++) {
            char c;

            if (j < numBytes) {
                c = static_cast<const char *>(bytes)[j];
                printf(" %02x", c);
            } else {
                printf("   ");
            }
        }

        putchar(' ');

        for (size_t j = i; j < (i + bytesPerLine); j++) {
            char c;

            if (j < numBytes) {
                c = static_cast<const char *>(bytes)[j];

                if (not isprint(c)) {
                    c = '.';
                }
            } else {
                c = ' ';
            }

            putchar(c);
        }

        putchar('\n');
    }
}

void log(const char *format, ...) {
    printTime();

    va_list arguments;
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);
}

} /* namespace xXx */
