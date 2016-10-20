#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "logging.hpp"

static const size_t maxLength = 256;

static void print(const char *message) {
    portENTER_CRITICAL();

    uint32_t millis = xTaskGetTickCount() * portTICK_PERIOD_MS;
    printf("%d.%03d %s\n", millis / 1000, millis % 1000, message);

    portEXIT_CRITICAL();
}

void printFormat(const char *format, ...) {
    char string[maxLength];
    size_t length;

    va_list args;
    va_start(args, format);
    length = vsnprintf(string, maxLength, format, args);
    va_end(args);

    for (size_t i = 0; i < length; i++) {
        if (!isprint(string[i])) {
            string[i] = '_';
        }
    }

    print(string);
}

void printBuffer(const char *message, uint8_t bytes[], size_t numBytes) {
    size_t messageLength    = strlen(message);
    size_t stringLength     = messageLength + (numBytes * 3) + 1;
    size_t byteStringLength = 3;

    char string[stringLength];

    // Copy terminating zero in case that numBytes equals zero (+1)
    memcpy(string, message, messageLength + 1);

    for (int i = 0; i < numBytes; ++i) {
        snprintf(&string[(i * byteStringLength) + messageLength],
                 byteStringLength + 1, " %02x", bytes[i]);
    }

    print(string);
}
