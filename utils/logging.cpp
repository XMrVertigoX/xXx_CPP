#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

#include "logging.hpp"

#define BYTE_REPRESEANTATION_FORMAT " %02x"
#define DUMMY_CHARACTER '_'
#define TIME_FORMAT "%d.%03d"
#define MESSAGE_FORMAT "%.*s\r\n"
#define OUTPUT_FORMAT TIME_FORMAT " " MESSAGE_FORMAT

static const size_t maxLength                = 256;
static const size_t byteRepresentationLength = 3;

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

    va_list arguments;
    char outputBytes[maxLength];
    size_t outputNumBytes;

    va_start(arguments, format);
    outputNumBytes = vsnprintf(outputBytes, maxLength, format, arguments);
    va_end(arguments);

    for (size_t i = 0; i < outputNumBytes; ++i) {
        if (!isprint(outputBytes[i])) {
            outputBytes[i] = DUMMY_CHARACTER;
        }
    }

    printf(OUTPUT_FORMAT, seconds(ticks), milliseconds(ticks), outputNumBytes,
           outputBytes);
}

void printBuffer(const char *message, uint8_t bytes[], size_t numBytes) {
    TickType_t ticks = xTaskGetTickCount();

    size_t messageLength = strlen(message);
    size_t outputNumBytes =
        messageLength + (numBytes * byteRepresentationLength) + 1;
    char outputBytes[outputNumBytes];

    memcpy(outputBytes, message, messageLength);

    for (int i = 0; i < numBytes; ++i) {
        int position = (byteRepresentationLength * i) + messageLength;
        char *string = &outputBytes[position];

        snprintf(string, byteRepresentationLength + 1,
                 BYTE_REPRESEANTATION_FORMAT, bytes[i]);
    }

    printf(OUTPUT_FORMAT, seconds(ticks), milliseconds(ticks), outputNumBytes,
           outputBytes);
}
