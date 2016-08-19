#ifndef NRF24L01P_HPP_
#define NRF24L01P_HPP_

#include <cstdint>

#include <lib/services/spidrv.hpp>

class nRF24L01P {
   private:
    SpiDrv &_spi;
    SpiDrv_Device_t &_device;

    uint8_t read(uint8_t addr, uint8_t bytes[], uint32_t numBytes);
    uint8_t write(uint8_t addr, uint8_t bytes[], uint32_t numBytes);

   public:
    nRF24L01P(SpiDrv &spi, SpiDrv_Device_t &device);
    ~nRF24L01P();

    void config_powerUp();
    void config_powerDown();
};

#endif /* NRF24L01P_HPP_ */
