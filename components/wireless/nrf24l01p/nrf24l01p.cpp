#include <cstdint>
#include <cstring>

#include <lib/services/spidrv.hpp>
#include <lib/util/logging.hpp>

#include "definitions.h"
#include "nrf24l01p.hpp"

#define PLACEHOLDER 0xFF

nRF24L01P::nRF24L01P(SpiDrv &spi, SpiDrv_Device_t &device)
    : _spi(spi), _device(device) {}

nRF24L01P::~nRF24L01P() {}

uint8_t nRF24L01P::read(uint8_t command, uint8_t bytes[], uint32_t numBytes) {
    uint8_t miso[numBytes + 1];
    uint8_t mosi[numBytes + 1];

    mosi[0] = command;
    memset(&mosi[1], PLACEHOLDER, numBytes);

    _spi.transceive(_device, miso, mosi, numBytes + 1);

    memcpy(&miso[1], bytes, numBytes);

    return (0);
}

uint8_t nRF24L01P::write(uint8_t command, uint8_t bytes[], uint32_t numBytes) {
    uint8_t miso[numBytes + 1];
    uint8_t mosi[numBytes + 1];

    mosi[0] = command;
    memcpy(&mosi[1], bytes, numBytes);

    _spi.transceive(_device, miso, mosi, numBytes + 1);

    return (0);
}

static inline void clearBit(uint8_t &byte, uint8_t bit) {
    byte &= ~(1 << bit);
}

static inline void setBit(uint8_t &byte, uint8_t bit) {
    byte |= (1 << bit);
}

void nRF24L01P::config_powerDown() {
    DEBUG("powerDown");

    uint8_t data[1];
    read(R_REGISTER | CONFIG, data, sizeof(data));
    clearBit(data[0], CONFIG_PWR_UP);
    write(W_REGISTER | CONFIG, data, sizeof(data));
}

void nRF24L01P::config_powerUp() {
    DEBUG("powerUp");

    uint8_t data[1];
    read(R_REGISTER | CONFIG, data, sizeof(data));
    setBit(data[0], CONFIG_PWR_UP);
    write(W_REGISTER | CONFIG, data, sizeof(data));
}
