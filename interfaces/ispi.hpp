#ifndef ISPI_HPP_
#define ISPI_HPP_

namespace xXx {

typedef void (*ISpi_Callback_t)(uint8_t misoBytes[], size_t misoNumBytes,
                                void *user);

class ISpi {
  public:
    virtual ~ISpi() = default;

    virtual uint8_t transmit(uint8_t mosiBytes[], size_t mosiNumBytes,
                             ISpi_Callback_t callback, void *user) = 0;
};

} /* namespace xXx */

#endif /* ISPI_HPP_ */
