#include "si1145.hpp"

#include <stdint.h>

#include "twi.h"

namespace xXx {

SI1145::SI1145(ITwi &twi) : _twi(twi) {}

SI1145::~SI1145() {}

} /* namespace xXx */
