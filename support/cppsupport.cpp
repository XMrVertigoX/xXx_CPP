#include <cstdlib>

#include <lib/util/logging.hpp>

extern "C" void __cxa_pure_virtual() {
    EMERGENCY("__cxa_pure_virtual");
    abort();
}
