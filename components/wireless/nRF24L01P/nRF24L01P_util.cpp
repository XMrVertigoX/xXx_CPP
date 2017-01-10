#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nRF24L01P_definitions.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

uint8_t nRF24L01P::transmit(uint8_t command, uint8_t txBytes[],
                            uint8_t rxBytes[], size_t numBytes) {
    uint8_t status;
    size_t transmissionNumBytes = numBytes + 1;
    uint8_t mosiBytes[transmissionNumBytes];
    uint8_t misoBytes[transmissionNumBytes];

    mosiBytes[0] = command;

    if (txBytes != NULL) {
        memcpy(&mosiBytes[1], txBytes, numBytes);
    } else {
        memset(&mosiBytes[1], dummy, numBytes);
    }

    _spi.transmit(mosiBytes, misoBytes, transmissionNumBytes);

    status = misoBytes[0];

    if (rxBytes != NULL) {
        memcpy(rxBytes, &misoBytes[1], numBytes);
    }

    return (status);
}

uint8_t nRF24L01P::readShortRegister(Register_t address) {
    uint8_t result;

    cmd_R_REGISTER(address, &result, 1);

    return (result);
}

uint8_t nRF24L01P::writeShortRegister(Register_t address, uint8_t regValue) {
    uint8_t status;

    status = cmd_W_REGISTER(address, &regValue, 1);

    return (status);
}

void nRF24L01P::clearSingleBit(Register_t address, uint8_t bitIndex) {
    uint8_t reg = readShortRegister(address);
    clearBit_r(reg, bitIndex);
    writeShortRegister(address, reg);
}

void nRF24L01P::setSingleBit(Register_t address, uint8_t bitIndex) {
    uint8_t reg = readShortRegister(address);
    setBit_r(reg, bitIndex);
    writeShortRegister(address, reg);
}
