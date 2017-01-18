#include <assert.h>
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
        memset(&mosiBytes[1], dummyByte, numBytes);
    }

    _spi.transmit(mosiBytes, misoBytes, transmissionNumBytes);

    status = misoBytes[0];

    if (rxBytes != NULL) {
        memcpy(rxBytes, &misoBytes[1], numBytes);
    }

    return (status);
}

uint8_t nRF24L01P::cmd_R_REGISTER(Register_t address, uint8_t bytes[],
                                  size_t numBytes) {
    uint8_t command, status;

    command = bitwiseOR(VALUE(Command_t::R_REGISTER), VALUE(address));
    status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t nRF24L01P::cmd_W_REGISTER(Register_t address, uint8_t bytes[],
                                  size_t numBytes) {
    uint8_t command, status;

    command = bitwiseOR(VALUE(Command_t::W_REGISTER), VALUE(address));
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t nRF24L01P::cmd_W_TX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command, status;

    command = VALUE(Command_t::W_TX_PAYLOAD);
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t nRF24L01P::cmd_R_RX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command, status;

    command = VALUE(Command_t::R_RX_PAYLOAD);
    status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t nRF24L01P::cmd_FLUSH_TX() {
    uint8_t command, status;

    command = VALUE(Command_t::FLUSH_TX);
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01P::cmd_FLUSH_RX() {
    uint8_t command, status;

    command = VALUE(Command_t::FLUSH_RX);
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01P::cmd_REUSE_TX_PL() {
    uint8_t command, status;

    command = VALUE(Command_t::REUSE_TX_PL);
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01P::cmd_R_RX_PL_WID(uint8_t &payloadLength) {
    uint8_t command, status;

    command = VALUE(Command_t::R_RX_PL_WID);
    status  = transmit(command, NULL, &payloadLength, 1);

    return (status);
}

// TODO
uint8_t nRF24L01P::cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[],
                                     size_t numBytes) {
    uint8_t command, status;

    bitwiseAND_r(pipe, 0b111);
    command = bitwiseOR(VALUE(Command_t::W_ACK_PAYLOAD), pipe);
    status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

// TODO
uint8_t nRF24L01P::cmd_W_TX_PAYLOAD_NOACK() {
    uint8_t command, status;

    command = VALUE(Command_t::W_TX_PAYLOAD_NOACK);

    return (status);
}

uint8_t nRF24L01P::cmd_NOP() {
    uint8_t command, status;

    command = VALUE(Command_t::NOP);
    status  = transmit(command, NULL, NULL, 0);

    return (status);
}
