#if not defined(NRF24L01P_BASE_HPP_)
#define NRF24L01P_BASE_HPP_

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/interfaces/ispi.hpp>

namespace xXx {

class nRF24L01P_BASE {
   private:
    ISpi &_spi;

    uint8_t transmit(uint8_t command, uint8_t txBytes[], uint8_t rxBytes[], size_t numBytes);

   protected:
    nRF24L01P_BASE(ISpi &spi);
    virtual ~nRF24L01P_BASE();

    uint8_t cmd_R_REGISTER(Register_t reg, uint8_t bytes[], size_t numBytes);
    uint8_t cmd_W_REGISTER(Register_t reg, uint8_t bytes[], size_t numBytes);
    uint8_t cmd_R_RX_PAYLOAD(uint8_t bytes[], size_t numBytes);
    uint8_t cmd_W_TX_PAYLOAD(uint8_t bytes[], size_t numBytes);
    uint8_t cmd_FLUSH_TX();
    uint8_t cmd_FLUSH_RX();
    uint8_t cmd_REUSE_TX_PL();
    uint8_t cmd_R_RX_PL_WID(uint8_t &payloadLength);
    uint8_t cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes);
    uint8_t cmd_W_TX_PAYLOAD_NOACK(uint8_t bytes[], size_t numBytes);
    uint8_t cmd_NOP();
};

} /* namespace xXx */

#endif /* NRF24L01P_BASE_HPP_ */
