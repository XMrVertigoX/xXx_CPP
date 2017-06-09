#ifndef ISPI_HPP
#define ISPI_HPP

#include <stddef.h>
#include <stdint.h>

namespace xXx {

class ISpi {
   public:
    virtual uint8_t transmit_receive(uint8_t *txBytes, uint8_t *rxBytes, size_t numBytes) = 0;
};

} /* namespace xXx */

#endif /* ISPI_HPP */
