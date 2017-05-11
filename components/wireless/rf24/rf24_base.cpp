#include <assert.h>
#include <string.h>

#include <xXx/components/wireless/rf24/rf24_base.hpp>
#include <xXx/components/wireless/rf24/rf24_types.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>

#define DUMMY 0xFF

#define __CAST(x) static_cast<uint8_t>(x)

namespace xXx {

RF24_BASE::RF24_BASE(ISpi &spi) : _spi(spi) {}

RF24_BASE::~RF24_BASE() {}

uint8_t RF24_BASE::transmit(uint8_t command, void *inBytes, void *outBytes, size_t numBytes) {
    uint8_t status;
    uint8_t buffer[numBytes + 1];

    buffer[0] = command;

    if (inBytes != NULL) {
        memcpy(&buffer[1], inBytes, numBytes);
    } else {
        memset(&buffer[1], DUMMY, numBytes);
    }

    _spi.transmit_receive(buffer, buffer, numBytes + 1);

    status = buffer[0];

    if (outBytes != NULL) {
        memcpy(outBytes, &buffer[1], numBytes);
    }

    return (status);
}

uint8_t RF24_BASE::R_REGISTER(RF24_Register address, void *bytes, size_t numBytes) {
    uint8_t command = OR<uint8_t>(__CAST(RF24_Command::R_REGISTER), __CAST(address));
    uint8_t status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t RF24_BASE::W_REGISTER(RF24_Register address, void *bytes, size_t numBytes) {
    uint8_t command = OR<uint8_t>(__CAST(RF24_Command::W_REGISTER), __CAST(address));
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::R_RX_PAYLOAD(void *bytes, size_t numBytes) {
    uint8_t command = __CAST(RF24_Command::R_RX_PAYLOAD);
    uint8_t status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t RF24_BASE::W_TX_PAYLOAD(void *bytes, size_t numBytes) {
    uint8_t command = __CAST(RF24_Command::W_TX_PAYLOAD);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::FLUSH_TX() {
    uint8_t command = __CAST(RF24_Command::FLUSH_TX);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24_BASE::FLUSH_RX() {
    uint8_t command = __CAST(RF24_Command::FLUSH_RX);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24_BASE::REUSE_TX_PL() {
    uint8_t command = __CAST(RF24_Command::REUSE_TX_PL);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24_BASE::R_RX_PL_WID(uint8_t &payloadLength) {
    uint8_t command = __CAST(RF24_Command::R_RX_PL_WID);
    uint8_t status  = transmit(command, NULL, &payloadLength, 1);

    return (status);
}

uint8_t RF24_BASE::W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes) {
    uint8_t command = OR<uint8_t>(__CAST(RF24_Command::W_ACK_PAYLOAD), pipe);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes) {
    uint8_t command = __CAST(RF24_Command::W_TX_PAYLOAD_NOACK);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::NOP() {
    uint8_t command = __CAST(RF24_Command::NOP);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

} /* namespace xXx */
