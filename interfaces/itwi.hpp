#ifndef ITWI_HPP_
#define ITWI_HPP_

namespace xXx {

class ITwi {
  public:
    virtual ~ITwi() = default;

    virtual uint8_t startTransmission(void) = 0;
    virtual uint8_t stopTransmission(void) = 0;
    virtual uint8_t readBytes(uint8_t address, uint8_t bytes[],
                              size_t numBytes) = 0;
    virtual uint8_t writeBytes(uint8_t address, uint8_t bytes[],
                               size_t numBytes) = 0;
};

} /* namespace xXx */

#endif /* ITWI_HPP_ */
