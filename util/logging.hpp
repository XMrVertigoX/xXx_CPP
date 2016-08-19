#ifndef LOGGING_HPP_
#define LOGGING_HPP_

#include <cstdint>

/*
 * Follows RFC5424 (https://tools.ietf.org/html/rfc5424)
 */

#define SEVERITY_EMERGENCY 0
#define SEVERITY_ALERT 1
#define SEVERITY_CRITICAL 2
#define SEVERITY_ERROR 3
#define SEVERITY_WARNING 4
#define SEVERITY_NOTICE 5
#define SEVERITY_INFO 6
#define SEVERITY_DEBUG 7

#if !defined(LOG_LEVEL)
#define LOG_LEVEL SEVERITY_WARNING
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_EMERGENCY
#define EMERGENCY(...)
#else
#define EMERGENCY(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_ALERT
#define ALERT(...)
#else
#define ALERT(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_CRITICAL
#define CRITICAL(...)
#else
#define CRITICAL(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_ERROR
#define ERROR(...)
#else
#define ERROR(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_WARNING
#define WARNING(...)
#else
#define WARNING(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_NOTICE
#define NOTICE(...)
#else
#define NOTICE(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_INFO
#define INFO(...)
#else
#define INFO(...) printFormat(__VA_ARGS__)
#endif

#if defined(NDEBUG) || LOG_LEVEL < SEVERITY_DEBUG
#define BUFFER(...)
#define DEBUG(...)
#else
#define BUFFER(...) printBuffer(__VA_ARGS__)
#define DEBUG(...) printFormat(__VA_ARGS__)
#endif

void printBuffer(const char *message, uint8_t bytes[], size_t numBytes);
void printFormat(const char *format, ...);

#endif /* LOGGING_HPP_ */
