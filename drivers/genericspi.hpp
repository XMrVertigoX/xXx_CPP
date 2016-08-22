#ifndef GENERICSPI_HPP_
#define GENERICSPI_HPP_

class GenericSpi {
   public:
    virtual ~GenericSpi() {}

    virtual void transmit(uint8_t misoBytes[], uint8_t mosiBytes[],
                          size_t numBytes) = 0;
};

#endif /* GENERICSPI_HPP_ */
