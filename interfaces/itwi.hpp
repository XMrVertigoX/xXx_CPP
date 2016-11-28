#ifndef ITWI_HPP_
#define ITWI_HPP_

#include <stddef.h>

namespace xXx {

struct ITwi_Message_t {
    uint8_t address;
    uint8_t *txBytes;
    size_t txNumBytes;
    uint8_t *rxBytes;
    size_t rxNumBytes;
};

enum ITwi_Status_t { ITwi_SUCCESS, ITwi_FAILURE };

class ITwi {
  public:
    virtual ~ITwi() = default;

    virtual ITwi_Status_t transmit_receive(ITwi_Message_t message) = 0;
};

} /* namespace xXx */

#endif /* ITWI_HPP_ */
