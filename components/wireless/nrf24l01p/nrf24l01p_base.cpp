#include <assert.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

namespace xXx {

static inline void prepareBuffer(uint8_t bytes[], size_t numBytes) {
    memset(bytes, 0xff, numBytes);
}

nRF24L01P_BASE::nRF24L01P_BASE(ISpi &spi) : _spi(spi) {}

nRF24L01P_BASE::~nRF24L01P_BASE() {}

int8_t nRF24L01P_BASE::transmit(uint8_t command, uint8_t bytes[], size_t numBytes) {
    if (bytes == NULL && numBytes > 0) return (-1);

    int8_t status;

    uint8_t newBytes[numBytes + 1];

    newBytes[0] = command;

    if (bytes != NULL) {
        memcpy(&newBytes[1], bytes, numBytes);
    }

    _spi.transmit_receive(newBytes, numBytes + 1);

    status = newBytes[0];

    if (bytes != NULL) {
        memcpy(bytes, &newBytes[1], numBytes);
    }

    return (status);
}

int8_t nRF24L01P_BASE::cmd_R_REGISTER(Register_t address, uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    int8_t status;

    prepareBuffer(bytes, numBytes);

    command = OR<uint8_t>(Command_R_REGISTER, address);
    status  = transmit(command, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_REGISTER(Register_t address, uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    int8_t status;

    command = OR<uint8_t>(Command_W_REGISTER, address);
    status  = transmit(command, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_TX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    int8_t status;

    command = Command_W_TX_PAYLOAD;
    status  = transmit(command, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_R_RX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    int8_t status;

    prepareBuffer(bytes, numBytes);

    command = Command_R_RX_PAYLOAD;
    status  = transmit(command, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_FLUSH_TX() {
    uint8_t command;
    int8_t status;

    command = Command_FLUSH_TX;
    status  = transmit(command);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_FLUSH_RX() {
    uint8_t command;
    int8_t status;

    command = Command_FLUSH_RX;
    status  = transmit(command);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_REUSE_TX_PL() {
    uint8_t command;
    int8_t status;

    command = Command_REUSE_TX_PL;
    status  = transmit(command);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_R_RX_PL_WID(uint8_t &payloadLength) {
    uint8_t command;
    int8_t status;

    prepareBuffer(&payloadLength, 1);

    command = Command_R_RX_PL_WID;
    status  = transmit(command, &payloadLength, 1);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    int8_t status;

    command = OR<uint8_t>(Command_W_ACK_PAYLOAD, pipe);
    status  = transmit(command, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes) {
    uint8_t command;
    int8_t status;

    command = Command_W_TX_PAYLOAD_NOACK;
    status  = transmit(command, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_NOP() {
    uint8_t command;
    int8_t status;

    command = Command_NOP;
    status  = transmit(command);

    return (status);
}

} /* namespace xXx */
