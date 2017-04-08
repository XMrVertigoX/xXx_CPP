#include <assert.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/templates/queue.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

namespace xXx {

nRF24L01P_BASE::nRF24L01P_BASE() {}

nRF24L01P_BASE::~nRF24L01P_BASE() {}

static uint8_t dummyByte = 0xFF;

int8_t nRF24L01P_BASE::transmit(uint8_t command, uint8_t txBytes[], uint8_t rxBytes[],
                                size_t numBytes) {
    uint8_t status;
    uint8_t bytes[numBytes + 1];

    bytes[0] = command;

    if (txBytes) {
        memcpy(&bytes[1], txBytes, numBytes);
    } else {
        memset(&bytes[1], dummyByte, numBytes);
    }

    transmit_receive(bytes, numBytes + 1);

    status = bytes[0];

    if (rxBytes) {
        memcpy(rxBytes, &bytes[1], numBytes);
    }

    return (status);
}

int8_t nRF24L01P_BASE::cmd_R_REGISTER(Register_t address, uint8_t bytes[], size_t numBytes) {
    uint8_t command = OR<uint8_t>(Command_R_REGISTER, static_cast<uint8_t>(address));
    uint8_t status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_REGISTER(Register_t address, uint8_t bytes[], size_t numBytes) {
    uint8_t command = OR<uint8_t>(Command_W_REGISTER, static_cast<uint8_t>(address));
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_TX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command = Command_W_TX_PAYLOAD;
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_R_RX_PAYLOAD(uint8_t bytes[], size_t numBytes) {
    uint8_t command = Command_R_RX_PAYLOAD;
    uint8_t status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_FLUSH_TX() {
    uint8_t command = Command_FLUSH_TX;
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_FLUSH_RX() {
    uint8_t command = Command_FLUSH_RX;
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_REUSE_TX_PL() {
    uint8_t command = Command_REUSE_TX_PL;
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_R_RX_PL_WID(uint8_t &payloadLength) {
    uint8_t command = Command_R_RX_PL_WID;
    uint8_t status  = transmit(command, NULL, &payloadLength, 1);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes) {
    uint8_t command = OR<uint8_t>(Command_W_ACK_PAYLOAD, pipe);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes) {
    uint8_t command = Command_W_TX_PAYLOAD_NOACK;
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

int8_t nRF24L01P_BASE::cmd_NOP() {
    uint8_t command = Command_NOP;
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

} /* namespace xXx */
