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

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    void handle_MAX_RT(uint8_t status);
    void handle_RX_DR(uint8_t status);
    void handle_TX_DS(uint8_t status);

    void txCallback();

    void readRxFifo(uint8_t status);
    void writeTxFifo(uint8_t status);

    uint8_t getRetransmissionCounter();

    void enableDataPipe(uint8_t pipeIndex);
    void disableDataPipe(uint8_t pipeIndex);
    void enableDynamicPayloadLength(uint8_t pipeIndex);
    void disableDynamicPayloadLength(uint8_t pipeIndex);

    uint32_t getRxBaseAddress_0();
    uint32_t getRxBaseAddress_1();
    uint8_t getRxAddressPrefix(uint8_t pipe);
    uint32_t getTxBaseAddress();
    uint8_t getTxAddressPrefix();

    uint8_t getChannel();
    CRCConfig_t getCrcConfig();
    DataRate_t getDataRate();
    OutputPower_t getOutputPower();
    uint8_t getRetryCount();
    uint8_t getRetryDelay();

   public:
    nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ESB();

    void setup();
    void loop();

    void switchOperatingMode(OperatingMode_t mode);

    uint8_t queueTransmission(uint8_t bytes[], size_t numBytes, txCallback_t callback, void *user);
    void startListening(uint8_t pipe, rxCallback_t callback, void *user);
    void stopListening(uint8_t pipe);

    uint8_t getPackageLossCounter();

    void setRxBaseAddress_0(uint32_t baseAddress);
    void setRxBaseAddress_1(uint32_t baseAddress);
    void setRxAddressPrefix(uint8_t pipe, uint8_t addressPrefix);
    void setTxBaseAddress(uint32_t baseAddress);
    void setTxAddressPrefix(uint8_t addressPrefix);

    void setChannel(uint8_t channel);
    void setCrcConfig(CRCConfig_t crc);
    void setDataRate(DataRate_t dataRate);
    void setOutputPower(OutputPower_t level);
    void setRetryCount(uint8_t count);
    void setRetryDelay(uint8_t delay);
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
