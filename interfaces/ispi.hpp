#ifndef ISPI_HPP_
#define ISPI_HPP_

#include <stddef.h>
#include <stdint.h>

namespace xXx {

typedef void (*ISpi_Callback_t)(uint8_t misoBytes[], size_t misoNumBytes,
                                void *user);

class ISpi {
  public:
    virtual ~ISpi() = default;

    virtual uint8_t transmit(uint8_t mosiBytes[], uint8_t misoBytes[],
                             size_t numBytes) = 0;
};

} /* namespace xXx */

#endif /* ISPI_HPP_ */
