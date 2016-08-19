#ifndef SPIDRV_HPP_
#define SPIDRV_HPP_

#include <parts.h>

#if SAME70
#include "same70/spidrv.hpp"
#else
#error Unsupported chip type
#endif

#endif /* SPIDRV_HPP_ */
