#ifndef NRF24L01P_ESB_HPP_
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/os/simpletask.hpp>
#include <xXx/templates/queue.hpp>

namespace xXx {

typedef void (*txCallback_t)(int8_t numRetries, void *user);

class RF24_ESB : public nRF24L01P_BASE, public SimpleTask {
   private:
    IGpio &ce;
    IGpio &irq;

    Queue_Handle_t<RF24_Package_t> rxQueue[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    Queue_Handle_t<RF24_Package_t> txQueue    = NULL;

    uint8_t *_txBuffer       = NULL;
    size_t _txBufferStart    = 0;
    size_t _txBufferEnd      = 0;
    txCallback_t _txCallback = NULL;
    void *_txUser            = NULL;

    void setup();
    void loop();

    void handle_MAX_RT(uint8_t status);
    void handle_RX_DR(uint8_t status);
    void handle_TX_DS(uint8_t status);

    void txCallback();

    uint8_t readRxFifo(uint8_t status);
    uint8_t writeTxFifo(uint8_t status);

    uint8_t getRetransmissionCounter();

    RF24_Status_t enableDataPipe(uint8_t pipeIndex);
    RF24_Status_t disableDataPipe(uint8_t pipeIndex);
    RF24_Status_t enableDynamicPayloadLength(uint8_t pipeIndex);
    RF24_Status_t disableDynamicPayloadLength(uint8_t pipeIndex);

    RF24_Address_t getRxAddress(uint8_t pipe);
    RF24_Address_t getTxAddress();

    uint8_t getChannel();
    RF24_CRCConfig getCrcConfig();
    RF24_DataRate getDataRate();
    RF24_OutputPower getOutputPower();
    uint8_t getRetryCount();
    uint8_t getRetryDelay();

   public:
    RF24_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~RF24_ESB();

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    uint8_t queuePackage(uint8_t bytes[], size_t numBytes, txCallback_t callback, void *user);
    uint8_t queuePackage2(RF24_Package_t package);

    RF24_Status_t startListening(uint8_t pipe, Queue<RF24_Package_t> &rxQueue);
    RF24_Status_t stopListening(uint8_t pipe);

    uint8_t getPackageLossCounter();

    RF24_Status_t setRxAddress(uint8_t pipe, RF24_Address_t address);
    RF24_Status_t setTxAddress(RF24_Address_t address);

    RF24_Status_t setChannel(uint8_t channel);
    RF24_Status_t setCrcConfig(RF24_CRCConfig crc);
    RF24_Status_t setDataRate(RF24_DataRate dataRate);
    RF24_Status_t setOutputPower(RF24_OutputPower level);
    RF24_Status_t setRetryCount(uint8_t count);
    RF24_Status_t setRetryDelay(uint8_t delay);
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
