#ifndef NRF24L01P_HPP_
#define NRF24L01P_HPP_

#include <stdint.h>

#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>

using namespace xXx;

class nRF24L01P {
  private:
    ISpi &_spi;
    IGpio &_ce;
    IGpio &_irq;

    uint8_t read(uint8_t addr, uint8_t dataBytes[], size_t dataNumBytes);
    uint8_t write(uint8_t addr, uint8_t dataBytes[], size_t dataNumBytes);

  public:
    nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P();

    void config_powerUp();
    void config_powerDown();
};

#endif /* NRF24L01P_HPP_ */
