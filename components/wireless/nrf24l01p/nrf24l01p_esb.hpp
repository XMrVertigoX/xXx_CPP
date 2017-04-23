#if not defined(NRF24L01P_ESB_HPP_)
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/os/simpletask.hpp>

enum DataRate_t : uint8_t { DataRate_1MBPS, DataRate_2MBPS, DataRate_250KBPS };

enum CRCConfig_t : uint8_t { CRCConfig_DISABLED, CRCConfig_1Byte, CrcConfig_2Bytes };

enum OutputPower_t : uint8_t {
    OutputPower_m18dBm,
    OutputPower_m12dBm,
    OutputPower_m6dBm,
    OutputPower_0dBm
};

enum OperatingMode_t : uint8_t {
    OperatingMode_Shutdown,
    OperatingMode_Standby,
    OperatingMode_Rx,
    OperatingMode_Tx
};

namespace xXx {

typedef void (*rxCallback_t)(uint8_t bytes[], size_t numBytes, void *user);
typedef void (*txCallback_t)(int8_t numRetries, void *user);

class nRF24L01P_ESB : public nRF24L01P_BASE, public SimpleTask {
   private:
    IGpio &_ce;
    IGpio &_irq;

    rxCallback_t _rxCallback[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    void *_rxUser[6]            = {NULL, NULL, NULL, NULL, NULL, NULL};

    uint8_t *_txBuffer       = NULL;
    size_t _txBufferStart    = 0;
    size_t _txBufferEnd      = 0;
    txCallback_t _txCallback = NULL;
    void *_txUser            = NULL;

    uint8_t readShortRegister(Register_t reg);
    void writeShortRegister(Register_t reg, uint8_t regValue);
    void clearSingleBit(Register_t reg, uint8_t bitIndex);
    void setSingleBit(Register_t reg, uint8_t bitIndex);

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    void handle_MAX_RT(int8_t status);
    void handle_RX_DR(int8_t status);
    void handle_TX_DS(int8_t status);

    void txCallback();

    void readRxFifo(int8_t status);
    void writeTxFifo(int8_t status);

    void enableDataPipe(uint8_t pipeIndex);
    void disableDataPipe(uint8_t pipeIndex);
    void enableDynamicPayloadLength(uint8_t pipeIndex);
    void disableDynamicPayloadLength(uint8_t pipeIndex);

    int8_t getChannel();
    CRCConfig_t getCrcConfig();
    DataRate_t getDataRate();
    OutputPower_t getOutputPower();
    int8_t getRetryCount();
    int8_t getRetryDelay();
    int64_t getRxAddress(uint8_t pipe);
    int64_t getTxAddress();
    int8_t getPackageLossCounter();
    int8_t getRetransmissionCounter();

   public:
    nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ESB();

    void setup();
    void loop();

    void configureTxPipe(uint64_t address);
    void configureRxPipe(uint8_t pipe, uint64_t address);
    void switchOperatingMode(OperatingMode_t mode);

    int8_t queueTransmission(uint8_t bytes[], size_t numBytes, txCallback_t callback, void *user);
    int8_t startListening(uint8_t pipe, rxCallback_t callback, void *user);
    int8_t stopListening(uint8_t pipe);

    int8_t setChannel(int8_t channel);
    int8_t setCrcConfig(CRCConfig_t crc);
    int8_t setDataRate(DataRate_t dataRate);
    int8_t setOutputPower(OutputPower_t level);
    int8_t setRetryCount(uint8_t count);
    int8_t setRetryDelay(uint8_t delay);
    int8_t setRxAddress(uint8_t pipe, int64_t address);
    int8_t setTxAddress(int64_t address);
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
