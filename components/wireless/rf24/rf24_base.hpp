#ifndef RF24_BASE_HPP
#define RF24_BASE_HPP

#include <xXx/components/wireless/rf24/rf24_types.hpp>
#include <xXx/interfaces/ispi.hpp>

namespace xXx {

class RF24_BASE {
   private:
    ISpi &_spi;

    uint8_t transmit(uint8_t command, void *txBytes, void *rxBytes, size_t numBytes);

   protected:
    RF24_BASE(ISpi &spi);
    virtual ~RF24_BASE();

    uint8_t R_REGISTER(RF24_Register reg, void *bytes, size_t numBytes);
    uint8_t W_REGISTER(RF24_Register reg, void *bytes, size_t numBytes);
    uint8_t R_RX_PAYLOAD(void *bytes, size_t numBytes);
    uint8_t W_TX_PAYLOAD(void *bytes, size_t numBytes);
    uint8_t FLUSH_TX();
    uint8_t FLUSH_RX();
    uint8_t REUSE_TX_PL();
    uint8_t R_RX_PL_WID(uint8_t &payloadLength);
    uint8_t W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes);
    uint8_t W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes);
    uint8_t NOP();
};

} /* namespace xXx */

#endif /* RF24_BASE_HPP */