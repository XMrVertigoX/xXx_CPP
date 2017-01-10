#ifndef NRF24L01P_HPP_
#define NRF24L01P_HPP_

#include <stdint.h>

#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>

using namespace xXx;

enum class Register_t : uint8_t;
enum class DataRate_t : uint8_t {
    DataRate_1MBPS,
    DataRate_2MBPS,
    DataRate_250KBPS
};
enum class Crc_t : uint8_t { DISABLED, CRC8, CRC16 };
enum class OutputPower_t : uint8_t {
    PowerLevel_18dBm = 0,
    PowerLevel_12dBm = 2,
    PowerLevel_6dBm  = 4,
    PowerLevel_0dBm  = 6
};

class nRF24L01P {
  private:
    ISpi &_spi;
    IGpio &_ce;
    IGpio &_irq;

    uint8_t cmd_R_REGISTER(Register_t reg, uint8_t *bytes, size_t numBytes);
    uint8_t cmd_W_REGISTER(Register_t reg, uint8_t *bytes, size_t numBytes);
    uint8_t cmd_R_RX_PAYLOAD(uint8_t *bytes, size_t numBytes);
    uint8_t cmd_W_TX_PAYLOAD(uint8_t *bytes, size_t numBytes);
    uint8_t cmd_FLUSH_TX();
    uint8_t cmd_FLUSH_RX();
    uint8_t cmd_REUSE_TX_PL();
    uint8_t cmd_R_RX_PL_WID(uint8_t bytes[], size_t numBytes);
    uint8_t cmd_W_ACK_PAYLOAD(uint8_t pipe, uint8_t bytes[], size_t numBytes);
    uint8_t cmd_W_TX_PAYLOAD_NOACK();
    uint8_t cmd_NOP();

    uint8_t transmit(uint8_t command, uint8_t txBytes[], uint8_t rxBytes[],
                     size_t numBytes);
    uint8_t readShortRegister(Register_t reg);
    uint8_t writeShortRegister(Register_t reg, uint8_t regValue);
    void clearSingleBit(Register_t address, uint8_t bitIndex);
    void setSingleBit(Register_t address, uint8_t bitIndex);

    void enterTxMode();
    void leaveTxMode();
    void enterRxMode();
    void leaveRxMode();

    Crc_t getCrcConfig();
    DataRate_t getDataRate();

  public:
    nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P();

    void init();

    // Basic usage
    void startWrite(uint8_t *bytes, size_t numBytes);
    void configureRxPipe(uint8_t pipe, uint64_t address = 0);
    void configureTxPipe(uint64_t address = 0);
    void startListening();
    void stopListening();
    void powerUp();
    void powerDown();

    uint64_t getRxAddress(uint8_t pipe);
    void setRxAddress(uint8_t pipe, uint64_t address);
    uint64_t getTxAddress();
    void setTxAddress(uint64_t address);

    void setCrcConfig(Crc_t crc);
    void setChannel(uint8_t channel);
    void setDataRate(DataRate_t dataRate);
    void setOutputPower(OutputPower_t level);
    void setRetries(uint8_t delay, uint8_t count);
};

#endif // NRF24L01P_HPP_
