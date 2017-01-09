#include <string.h>

#include <xXx/components/wireless/nRF24L01/nRF24L01.hpp>
#include <xXx/components/wireless/nRF24L01/nRF24L01_definitions.hpp>

uint8_t nRF24L01::transmit(uint8_t command, uint8_t txBytes[],
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

uint8_t nRF24L01::readShortRegister(Register_t address) {
    uint8_t result;

    cmd_R_REGISTER(address, &result, 1);

    return (result);
}

uint8_t nRF24L01::writeShortRegister(Register_t address, uint8_t value) {
    uint8_t status = cmd_W_REGISTER(address, &value, 1);

    return (status);
}
