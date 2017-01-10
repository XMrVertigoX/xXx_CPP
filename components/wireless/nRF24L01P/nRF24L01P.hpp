#ifndef NRF24L01P_HPP_
#define NRF24L01P_HPP_

#include <stdint.h>

#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>

using namespace xXx;

enum class Register_t : uint8_t;

enum DataRate_t : uint8_t { DataRate_1MBPS, DataRate_2MBPS, DataRate_250KBPS };

enum CRC_t : uint8_t { CRC_DISABLED, CRC_1BYTE, CRC_2BYTES };

enum PowerLevel_t : uint8_t {
    PowerLevel_18dBm,
    PowerLevel_12dBm,
    PowerLevel_6dBm,
    PowerLevel_0dBm
};

class nRF24L01P {
  private:
    uint8_t payload_size;
    bool ack_payload_available;
    bool dynamic_payloads_enabled;
    uint8_t ack_payload_length;
    uint64_t pipe0_reading_address;

    IGpio &_ce;
    IGpio &_irq;
    ISpi &_spi;

    /*
     * nRF24L01_cmd.cpp
     */
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

    /*
     * nRF24L01_util.cpp
     */
    uint8_t transmit(uint8_t command, uint8_t txBytes[], uint8_t rxBytes[],
                     size_t numBytes);
    uint8_t readShortRegister(Register_t reg);
    uint8_t writeShortRegister(Register_t reg, uint8_t value);
    void clearIRQs();
    void clearSingleBit(Register_t address, uint8_t bit);
    void setSingleBit(Register_t address, uint8_t bit);

  public:
    nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P();

    void init();

    void enterRxMode();
    void leaveRxMode();
    void enterTxMode();
    void leaveTxMode();

    // XXX
    void startWrite(uint8_t *bytes, size_t numBytes);

    /*
     * Getter/Setter
     */
    // uint64_t getTxAddress();
    void setTxAddress(uint64_t address);
    // uint64_t getRxAddress(uint8_t number);
    void setRxAddress(uint8_t number, uint64_t address);
    PowerLevel_t getPowerLevel(void);
    void setPowerLevel(PowerLevel_t level);
    DataRate_t getDataRate();
    void setDataRate(DataRate_t dataRate);
    CRC_t getCRCConfig();
    void setCRCConfig(CRC_t crc);
    // bool setPowerState();
    void setPowerState(bool enable);
    // TODO: Split into two functions and implement getter
    void setRetries(uint8_t delay, uint8_t count);
    // uint8_t getChannel();
    void setChannel(uint8_t channel);
};

#endif // NRF24L01P_HPP_
