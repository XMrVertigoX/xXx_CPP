#ifndef ITWI_HPP_
#define ITWI_HPP_

#include <stddef.h>

namespace xXx {

enum ITwi_Status_t { ITwi_SUCCESS, ITwi_FAILURE };

typedef void (*ITwi_Callback_t)(ITwi_Status_t status, uint8_t rxBytes[],
                                size_t rxNumBytes);

class ITwi {
  public:
    virtual ~ITwi() = default;

    virtual ITwi_Status_t transmit_receive(uint8_t address, uint8_t txBytes[],
                                           size_t txNumBytes,
                                           ITwi_Callback_t callback,
                                           void *user) = 0;
};

} /* namespace xXx */

#endif /* ITWI_HPP_ */
