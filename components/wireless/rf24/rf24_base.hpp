#ifndef RF24_BASE_HPP
#define RF24_BASE_HPP

#include <xXx/components/wireless/rf24/rf24_types.hpp>
#include <xXx/interfaces/ispi.hpp>

// clang-format off

#define R_REGISTER(...)         readRegister(__VA_ARGS__)
#define W_REGISTER(...)         writeRegister(__VA_ARGS__)
#define R_RX_PAYLOAD(...)       readRxPayload(__VA_ARGS__);
#define W_TX_PAYLOAD(...)       writeTxPayload(__VA_ARGS__);
#define FLUSH_TX(...)           flushTxFifo(__VA_ARGS__);
#define FLUSH_RX(...)           flushRxFifo(__VA_ARGS__);
#define REUSE_TX_PL(...)        reuseTxPayload(__VA_ARGS__);
#define R_RX_PL_WID(...)        readRxPayloadLength(__VA_ARGS__);
#define W_ACK_PAYLOAD(...)      cmd_W_ACK_PAYLOAD(__VA_ARGS__);
#define W_TX_PAYLOAD_NOACK(...) cmd_W_TX_PAYLOAD_NOACK(__VA_ARGS__);
#define NOP(...)                getStatus(__VA_ARGS__);

// clang-format on

namespace xXx {

class RF24_BASE {
   private:
    ISpi &_spi;

    uint8_t transmit(uint8_t command, void *txBytes, void *rxBytes, size_t numBytes);

   protected:
    RF24_BASE(ISpi &spi);
    virtual ~RF24_BASE();

    uint8_t readRegister(RF24_Register reg, void *bytes, size_t numBytes);
    uint8_t writeRegister(RF24_Register reg, void *bytes, size_t numBytes);
    uint8_t readRxPayload(void *bytes, size_t numBytes);
    uint8_t writeTxPayload(void *bytes, size_t numBytes);
    uint8_t flushTxFifo();
    uint8_t flushRxFifo();
    uint8_t reuseTxPayload();
    uint8_t readRxPayloadLength(uint8_t &payloadLength);
    uint8_t cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes);
    uint8_t cmd_W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes);
    uint8_t getStatus();
};

} /* namespace xXx */

#endif /* RF24_BASE_HPP */
