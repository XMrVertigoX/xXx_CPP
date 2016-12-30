#include <stdint.h>
#include <string.h>

#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/logging.hpp>

#include "definitions.h"
#include "nrf24l01p.hpp"

#define lambda []

const uint8_t spiPlaceholder = 0xFF;
const uint8_t commandLength  = 1;

nRF24L01P::nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq) {}

nRF24L01P::~nRF24L01P() {}

uint8_t nRF24L01P::read(uint8_t command, uint8_t dataBytes[],
                        size_t dataNumBytes) {
    uint8_t mosiNumBytes = dataNumBytes + commandLength;

    uint8_t mosiBytes[mosiNumBytes];

    mosiBytes[0] = command;

    uint8_t *startOfCommand = &mosiBytes[0];
    uint8_t *startOfData    = &mosiBytes[commandLength];
    memset(startOfData, spiPlaceholder, dataNumBytes);

    _spi.transmit(mosiBytes, mosiNumBytes,
                  lambda(uint8_t misoBytes[], size_t misoNumBytes, void *user) {
                      BUFFER("<<<", misoBytes, misoNumBytes);
                  },
                  this);

    return (0);
}

uint8_t nRF24L01P::write(uint8_t command, uint8_t dataBytes[],
                         size_t dataNumBytes) {
    uint8_t mosiNumBytes = dataNumBytes + commandLength;

    uint8_t mosiBytes[mosiNumBytes];

    mosiBytes[0] = command;

    uint8_t *startOfCommand = &mosiBytes[0];
    uint8_t *startOfData    = &mosiBytes[commandLength];
    memcpy(startOfData, dataBytes, dataNumBytes);

    _spi.transmit(mosiBytes, mosiNumBytes,
                  lambda(uint8_t misoBytes[], size_t misoNumBytes, void *user) {
                      BUFFER("<<<", misoBytes, misoNumBytes);
                  },
                  this);

    return (0);
}

static inline void clearBit(uint8_t &byte, uint8_t bit) {
    byte &= ~(1 << bit);
}

static inline void setBit(uint8_t &byte, uint8_t bit) {
    byte |= (1 << bit);
}

void nRF24L01P::config_powerDown() {
    LOG("powerDown");

    uint8_t data[1];
    read(R_REGISTER | CONFIG, data, sizeof(data));
    clearBit(data[0], CONFIG_PWR_UP);
    write(W_REGISTER | CONFIG, data, sizeof(data));
}

void nRF24L01P::config_powerUp() {
    LOG("powerUp");

    uint8_t data[1];
    read(R_REGISTER | CONFIG, data, sizeof(data));
    setBit(data[0], CONFIG_PWR_UP);
    write(W_REGISTER | CONFIG, data, sizeof(data));
}
