#ifndef NRF24L01_HPP_
#define NRF24L01_HPP_

#include <stdint.h>

#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>

using namespace xXx;

enum nRF24L01_Command_t : uint8_t;

enum nRF24L01_Register_t : uint8_t;

enum nRF24L01_DataRate_t : uint8_t { SPEED_1MBPS, SPEED_2MBPS, SPEED_250KBPS };

enum RF24_CRC_t : uint8_t { RF24_CRC_DISABLED, RF24_CRC_8, RF24_CRC_16 };

enum RF24_PowerLevel_t : uint8_t {
    RF24_PA_18dBm,
    RF24_PA_12dBm,
    RF24_PA_6dBm,
    RF24_PA_0dBm
};

class nRF24L01 {
  private:
    bool p_variant;
    uint8_t payload_size;
    bool ack_payload_available;
    bool dynamic_payloads_enabled;
    uint8_t ack_payload_length;
    uint64_t pipe0_reading_address;

    IGpio &_ce;
    IGpio &_irq;
    ISpi &_spi;

    uint8_t cmd_R_REGISTER(nRF24L01_Register_t reg, uint8_t *bytes,
                           uint8_t numBytes);
    uint8_t cmd_W_REGISTER(nRF24L01_Register_t reg, uint8_t *bytes,
                           uint8_t numBytes);
    uint8_t cmd_R_RX_PAYLOAD(uint8_t *bytes, uint8_t numBytes);
    uint8_t cmd_W_TX_PAYLOAD(uint8_t *bytes, uint8_t numBytes);
    uint8_t cmd_FLUSH_TX(void);
    uint8_t cmd_FLUSH_RX(void);
    uint8_t cmd_REUSE_TX_PL(void);
    uint8_t cmd_R_RX_PL_WID(void);
    uint8_t cmd_W_ACK_PAYLOAD(void);
    uint8_t cmd_W_TX_PAYLOAD_NOACK(void);
    uint8_t cmd_NOP(void);

    uint8_t transmit(uint8_t command, uint8_t txBytes[], uint8_t rxBytes[],
                     size_t numBytes);
    uint8_t readShortRegister(nRF24L01_Register_t reg);
    uint8_t writeShortRegister(nRF24L01_Register_t reg, uint8_t value);
    void clearIRQs();
    void clearSingleBit(nRF24L01_Register_t address, uint8_t bit);
    void setSingleBit(nRF24L01_Register_t address, uint8_t bit);

  public:
    nRF24L01(ISpi &spi, IGpio &ce, IGpio &irq);
    void print_address_register(char *name, uint8_t reg, uint8_t qty = 1);
    void toggle_features(void);
    void init(void);
    void startListening(void);
    void stopListening(void);
    bool write(uint8_t *bytes, uint8_t numBytes);
    bool available(void);
    bool read(uint8_t *bytes, uint8_t numBytes);
    void setTxAddress(uint64_t address);
    void setRxAddress(uint8_t number, uint64_t address);
    void setRetries(uint8_t delay, uint8_t count);
    void setChannel(uint8_t channel);
    void setPayloadSize(uint8_t size);
    uint8_t getPayloadSize(void);
    uint8_t getDynamicPayloadSize(void);
    void enableAckPayload(void);
    void enableDynamicPayloads(void);
    void setAutoAck(bool enable);
    void setAutoAck(uint8_t pipe, bool enable);
    void setPowerLevel(RF24_PowerLevel_t level);
    RF24_PowerLevel_t getPowerLevel(void);
    void setDataRate(nRF24L01_DataRate_t speed);
    nRF24L01_DataRate_t getDataRate(void);
    void setCRCConfig(RF24_CRC_t numBytesgth);
    RF24_CRC_t getCRCConfig(void);
    void setPowerState(bool enable);
    bool available(uint8_t *pipe_num);
    void startWrite(uint8_t *bytes, uint8_t numBytes);
    void writeAckPayload(uint8_t pipe, uint8_t *bytes, uint8_t numBytes);
    bool isAckPayloadAvailable(void);
    void whatHappened(bool &tx_ok, bool &tx_fail, bool &rx_ready);
    bool testRPD(void);
};

#endif // NRF24L01_HPP_
