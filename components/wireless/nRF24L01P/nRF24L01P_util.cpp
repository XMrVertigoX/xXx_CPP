#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nRF24L01P_definitions.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

uint8_t nRF24L01P::readShortRegister(Register_t address) {
    uint8_t result;

    cmd_R_REGISTER(address, &result, 1);

    return (result);
}

uint8_t nRF24L01P::writeShortRegister(Register_t address, uint8_t reg) {
    uint8_t status;

    status = cmd_W_REGISTER(address, &reg, 1);

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

uint8_t nRF24L01P::getRxNumBytes() {
    uint8_t rxNumBytes;

    cmd_R_RX_PL_WID(rxNumBytes);

    return (rxNumBytes);
}
