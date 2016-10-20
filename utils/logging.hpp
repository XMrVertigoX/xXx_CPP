#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <stdint.h>

#if defined(NDEBUG)
#define BUFFER(...)
#define LOG(...)
#else
#define BUFFER(...) printBuffer(__VA_ARGS__)
#define LOG(...) printFormat(__VA_ARGS__)
#endif

void printBuffer(const char *message, uint8_t bytes[], size_t numBytes);
void printFormat(const char *format, ...);

#endif /* LOGGING_HPP_ */
