#ifndef SPIDRV_SAME70_HPP_
#define SPIDRV_SAME70_HPP_

#include <asf.h>

enum SpiDrv_Mode_t {
    SpiDrv_Mode_0,  // CPOL = 0, CPHA = 0
    SpiDrv_Mode_1,  // CPOL = 0, CPHA = 1
    SpiDrv_Mode_2,  // CPOL = 1, CPHA = 0
    SpiDrv_Mode_3,  // CPOL = 1, CPHA = 1
};

enum SpiDrv_Peripheral_t {
    SpiDrv_Peripheral_0,
    SpiDrv_Peripheral_1,
    SpiDrv_Peripheral_2,
    SpiDrv_Peripheral_3,
};

struct SpiDrv_Device_t {
    SpiDrv_Peripheral_t peripheral;
};

struct SpiDrv_Buffer_t {
    uint8_t *misoBytes;
    uint8_t *mosiBytes;
    size_t numBytes;
};

class SpiDrv {
   private:
    void configurePeripheralChipSelectPin(SpiDrv_Peripheral_t &peripheral);
    void configurePins();

    Spi *_spi;

   public:
    SpiDrv(Spi *spi);
    ~SpiDrv();
    uint8_t enableMasterMode(uint32_t delayBetweenChipSelect = 0);
    uint8_t setupDevice(SpiDrv_Device_t &device, SpiDrv_Peripheral_t peripheral,
                        SpiDrv_Mode_t mode, uint32_t baudRate);
    uint8_t transceive(SpiDrv_Device_t &device, uint8_t misoBytes[],
                       uint8_t mosiBytes[], size_t numBytes);
};

#endif /* SPIDRV_SAME70_HPP_ */
