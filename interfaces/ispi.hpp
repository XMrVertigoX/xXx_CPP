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

    // virtual uint8_t transmit(uint8_t mosiBytes[], size_t numBytes) = 0;
    // virtual uint8_t receive(uint8_t misoBytes[], size_t numBytes)  = 0;

    virtual uint8_t transmit_receive(Queue<uint8_t> &mosiQueue,
                                     Queue<uint8_t> &misoQueue,
                                     ISpi_Callback_t callback, void *user) = 0;
};

} /* namespace xXx */

#endif /* ISPI_HPP_ */
