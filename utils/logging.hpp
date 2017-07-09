#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>



#if defined(NDEBUG)
#define HEXDUMP(...)
#define LOG(...)
#else
#define HEXDUMP(...) xXx::hexdump(__VA_ARGS__)
#define LOG(...) xXx::log(__VA_ARGS__)

#endif

namespace xXx {

void hexdump(const void *bytes, size_t numBytes);
void log(const char *format, ...);

} /* namespace xXx */

#endif /* LOGGING_HPP_ */
