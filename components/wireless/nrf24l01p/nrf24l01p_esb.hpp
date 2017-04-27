#if not defined(NRF24L01P_ESB_HPP_)
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/os/simpletask.hpp>
#include <xXx/templates/queue.hpp>

struct RF24_Package_t {
    uint8_t bytes[32];
    uint8_t numBytes;
};

enum RF24_DataRate_t : uint8_t {
    RF24_DataRate_1MBPS,
    RF24_DataRate_2MBPS,
    RF24_DataRate_250KBPS,
};

enum RF24_CRCConfig_t : uint8_t {
    RF24_CRCConfig_DISABLED,
    RF24_CRCConfig_1Byte,
    RF24_CrcConfig_2Bytes,
};

enum RF24_OutputPower_t : uint8_t {
    RF24_OutputPower_m18dBm,
    RF24_OutputPower_m12dBm,
    RF24_OutputPower_m6dBm,
    RF24_OutputPower_0dBm,
};

enum RF24_OperatingMode_t : uint8_t {
    RF24_OperatingMode_Shutdown,
    RF24_OperatingMode_Standby,
    RF24_OperatingMode_Rx,
    RF24_OperatingMode_Tx,
};

namespace xXx {

typedef void (*txCallback_t)(int8_t numRetries, void *user);

class nRF24L01P_ESB : public nRF24L01P_BASE, public SimpleTask {
   private:
    IGpio &_ce;
    IGpio &_irq;

    Queue_Handle_t<RF24_Package_t> _txQueue    = NULL;
    Queue_Handle_t<RF24_Package_t> _rxQueue[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

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
    uint8_t getRxAddress(uint8_t pipe);
    uint32_t getTxBaseAddress();
    uint8_t getTxAddress();

    uint8_t getChannel();
    RF24_CRCConfig_t getCrcConfig();
    RF24_DataRate_t getDataRate();
    RF24_OutputPower_t getOutputPower();
    uint8_t getRetryCount();
    uint8_t getRetryDelay();

   public:
    nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ESB();

    void setup();
    void loop();

    void switchOperatingMode(RF24_OperatingMode_t mode);

    uint8_t queueTransmission(uint8_t bytes[], size_t numBytes, txCallback_t callback, void *user);
    void startListening(uint8_t pipe, Queue_Handle_t<RF24_Package_t> rxQueue);
    void stopListening(uint8_t pipe);

    uint8_t getPackageLossCounter();

    void setRxBaseAddress_0(uint32_t baseAddress);
    void setRxBaseAddress_1(uint32_t baseAddress);
    void setRxAddress(uint8_t pipe, uint8_t address);
    void setTxBaseAddress(uint32_t baseAddress);
    void setTxAddress(uint8_t address);

    void setChannel(uint8_t channel);
    void setCrcConfig(RF24_CRCConfig_t crc);
    void setDataRate(RF24_DataRate_t dataRate);
    void setOutputPower(RF24_OutputPower_t level);
    void setRetryCount(uint8_t count);
    void setRetryDelay(uint8_t delay);
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
