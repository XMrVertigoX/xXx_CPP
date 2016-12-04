#ifndef ITWI_HPP_
#define ITWI_HPP_

#include <stddef.h>

namespace xXx {

typedef void (*ITwi_Callback_t)(uint8_t rxBytes[], size_t rxNumBytes);

class ITwi {
  public:
    virtual ~ITwi() = default;

    virtual ITwi_Status_t transmit_receive(uint8_t address, uint8_t txBytes[],
                                           size_t txNumBytes, size_t rxNumBytes,
                                           ITwi_Callback_t callback,
                                           void *user) = 0;
};

} /* namespace xXx */

#endif /* ITWI_HPP_ */
