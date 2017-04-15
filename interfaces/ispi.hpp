#ifndef ISPI_HPP_
#define ISPI_HPP_

#include <stddef.h>
#include <stdint.h>

#include <xXx/templates/queue.hpp>

namespace xXx {

typedef void (*ISpi_Callback_t)(void *user);

class ISpi {
   public:
    virtual ~ISpi() = default;

    virtual uint8_t transmit_receive(uint8_t txBytes[], uint8_t rxBytes[], size_t numBytes) = 0;
};

} /* namespace xXx */

#endif /* ISPI_HPP_ */
