#ifndef ISPI_HPP_
#define ISPI_HPP_

class ISpi {
   public:
    virtual ~ISpi() = default;

    virtual uint8_t transmit(uint8_t misoBytes[], uint8_t mosiBytes[],
                             size_t numBytes) = 0;
};

#endif /* ISPI_HPP_ */
