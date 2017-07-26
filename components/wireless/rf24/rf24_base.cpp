#include <cstdint>
#include <cstring>
#include <type_traits>

#include <xXx/components/wireless/rf24/rf24_base.hpp>
#include <xXx/components/wireless/rf24/rf24_types.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>

static const uint8_t dummyByte = 0xFF;

template <typename TYPE>
constexpr typename std::underlying_type<TYPE>::type asUnderlyingType(TYPE enumValue) {
    return (static_cast<typename std::underlying_type<TYPE>::type>(enumValue));
}

namespace xXx {

RF24_BASE::RF24_BASE(ISpi &spi)
    : _spi(spi) {}

RF24_BASE::~RF24_BASE() {}

uint8_t RF24_BASE::transmit(uint8_t command, const uint8_t *txBytes, uint8_t *rxBytes, size_t numBytes) {
    uint8_t status;
    uint8_t buffer[numBytes + 1];

    buffer[0] = command;

    if (txBytes != NULL) {
        std::memcpy(&buffer[1], txBytes, numBytes);
    } else {
        std::memset(&buffer[1], dummyByte, numBytes);
    }

    _spi.transmit_receive(buffer, buffer, numBytes + 1);

    status = buffer[0];

    if (rxBytes != NULL) {
        std::memcpy(rxBytes, &buffer[1], numBytes);
    }

    return (status);
}

uint8_t RF24_BASE::R_REGISTER(RF24_Register address, uint8_t *bytes, size_t numBytes) {
    uint8_t command = OR<uint8_t>(asUnderlyingType(RF24_Command::R_REGISTER), asUnderlyingType(address));
    uint8_t status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t RF24_BASE::W_REGISTER(RF24_Register address, const uint8_t *bytes, size_t numBytes) {
    uint8_t command = OR<uint8_t>(asUnderlyingType(RF24_Command::W_REGISTER), asUnderlyingType(address));
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::R_RX_PAYLOAD(uint8_t *bytes, size_t numBytes) {
    uint8_t command = asUnderlyingType(RF24_Command::R_RX_PAYLOAD);
    uint8_t status  = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t RF24_BASE::W_TX_PAYLOAD(const uint8_t *bytes, size_t numBytes) {
    uint8_t command = asUnderlyingType(RF24_Command::W_TX_PAYLOAD);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::FLUSH_TX() {
    uint8_t command = asUnderlyingType(RF24_Command::FLUSH_TX);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24_BASE::FLUSH_RX() {
    uint8_t command = asUnderlyingType(RF24_Command::FLUSH_RX);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24_BASE::REUSE_TX_PL() {
    uint8_t command = asUnderlyingType(RF24_Command::REUSE_TX_PL);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24_BASE::R_RX_PL_WID(uint8_t &payloadLength) {
    uint8_t command = asUnderlyingType(RF24_Command::R_RX_PL_WID);
    uint8_t status  = transmit(command, NULL, &payloadLength, 1);

    return (status);
}

uint8_t RF24_BASE::W_ACK_PAYLOAD(uint8_t pipe, const uint8_t *bytes, size_t numBytes) {
    uint8_t command = OR<uint8_t>(asUnderlyingType(RF24_Command::W_ACK_PAYLOAD), pipe);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::W_TX_PAYLOAD_NOACK(const uint8_t *bytes, size_t numBytes) {
    uint8_t command = asUnderlyingType(RF24_Command::W_TX_PAYLOAD_NOACK);
    uint8_t status  = transmit(command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24_BASE::NOP() {
    uint8_t command = asUnderlyingType(RF24_Command::NOP);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

} /* namespace xXx */
