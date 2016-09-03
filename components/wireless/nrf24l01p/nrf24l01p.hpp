#ifndef NRF24L01P_HPP_
#define NRF24L01P_HPP_

#include <cstdint>

#include <xXx/services/ispi.hpp>

class nRF24L01P {
   private:
    ISpi &_spi;

    uint8_t read(uint8_t addr, uint8_t bytes[], uint32_t numBytes);
    uint8_t write(uint8_t addr, uint8_t bytes[], uint32_t numBytes);

   public:
    nRF24L01P(ISpi &spi);
    ~nRF24L01P();

    void config_powerUp();
    void config_powerDown();
};

#endif /* NRF24L01P_HPP_ */
