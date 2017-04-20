#if not defined(NRF24L01P_ESB_HPP_)
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>

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
typedef void (*txCallback_t)(uint8_t bytes[], size_t numBytes, void *user);

class nRF24L01P_ESB : public nRF24L01P_BASE {
   private:
    IGpio &_ce;
    IGpio &_irq;

    uint8_t _notificationCounter = 0;

    rxCallback_t _rxCallback[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    void *_rxUser[6]            = {NULL, NULL, NULL, NULL, NULL, NULL};

    uint8_t *_txBytes        = NULL;
    size_t _txBytesStart     = 0;
    size_t _txBytesEnd       = 0;
    txCallback_t _txCallback = NULL;
    void *_txUser            = NULL;

    int8_t notifyGive();
    int8_t notifyTake();

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

    void readRxFifo(int8_t status);
    void writeTxFifo(int8_t status);

    void enableDataPipe(uint8_t pipeIndex);
    void disableDataPipe(uint8_t pipeIndex);
    void enableDynamicPayloadLength(uint8_t pipeIndex);
    void disableDynamicPayloadLength(uint8_t pipeIndex);

   public:
    nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ESB();

    void setup();
    void loop();

    void configureTxPipe(uint64_t address);
    void configureRxPipe(uint8_t pipe, uint64_t address);
    void switchOperatingMode(OperatingMode_t mode);

    int8_t send(uint8_t bytes[], size_t numBytes, txCallback_t callback, void *user);
    int8_t startListening(uint8_t pipe, rxCallback_t callback, void *user);
    int8_t stopListening(uint8_t pipe);

    int8_t getChannel();
    void setChannel(int8_t channel);
    CRCConfig_t getCrcConfig();
    void setCrcConfig(CRCConfig_t crc);
    void setDataRate(DataRate_t dataRate);
    DataRate_t getDataRate();
    OutputPower_t getOutputPower();
    void setOutputPower(OutputPower_t level);
    uint8_t getRetryCount();
    void setRetryCount(uint8_t count);
    uint8_t getRetryDelay();
    void setRetryDelay(uint8_t delay);
    int64_t getRxAddress(uint8_t pipe);
    void setRxAddress(uint8_t pipe, int64_t address);
    int64_t getTxAddress();
    void setTxAddress(int64_t address);
    int8_t getPackageLossCounter();
    int8_t getRetransmitCounter();
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
