#include "si1145.hpp"

#include <stdint.h>

#include "twi.h"

namespace xXx {

SI1145::SI1145(ITwi &twi, uint8_t address) : _twi(twi), _address(address) {}

SI1145::~SI1145() {}

void SI1145::init() {}

} /* namespace xXx */
