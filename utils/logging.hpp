#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(NDEBUG)
#define BUFFER(...)
#define LOG(...)
#else
#define BUFFER(...) printBuffer(__VA_ARGS__)
#define LOG(...) printFormat(__VA_ARGS__)
#endif

void printBuffer(const char *message, void *bytes, size_t numBytes);
void printFormat(const char *format, ...);

#endif /* LOGGING_HPP_ */
