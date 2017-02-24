#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_esb.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define LAMBDA []

#define min(a, b) (a < b ? a : b)

namespace xXx {

const uint8_t txSettling = 130;
const uint8_t rxSettling = 130;
const uint8_t txFifoSize = 32;
const uint8_t rxFifoSize = 32;

nRF24L01P_ESB::nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq, uint8_t priority)
    : ArduinoTask(256, priority), _ce(ce), _irq(irq), _spi(spi) {}

nRF24L01P_ESB::~nRF24L01P_ESB() {}

void nRF24L01P_ESB::transmit_receive(Queue<uint8_t> &queue) {
    _spi.transmit_receive(queue);
}

void nRF24L01P_ESB::setup() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_ESB *self = static_cast<nRF24L01P_ESB *>(user);

        self->taskNotifyFromISR();
    };

    // Enable Enhanced ShockBurst™
    uint8_t feature = 0;
    OR_eq<uint8_t>(feature, FEATURE_EN_DYN_ACK_MASK);
    OR_eq<uint8_t>(feature, FEATURE_EN_ACK_PAY_MASK);
    OR_eq<uint8_t>(feature, FEATURE_EN_DPL_MASK);
    writeShortRegister(Register_FEATURE, feature);

    // Clear interrupts
    uint8_t status = 0;
    OR_eq<uint8_t>(status, STATUS_MAX_RT_MASK);
    OR_eq<uint8_t>(status, STATUS_RX_DR_MASK);
    OR_eq<uint8_t>(status, STATUS_TX_DS_MASK);
    writeShortRegister(Register_STATUS, status);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_ESB::loop() {
    taskNotifyTake(pdFALSE);

    int8_t status = cmd_NOP();

    if (readBit<uint8_t>(status, STATUS_RX_DR)) {
        handle_RX_DR();
    }

    if (readBit<uint8_t>(status, STATUS_MAX_RT)) {
        handle_MAX_RT();
    }

    if (readBit<uint8_t>(status, STATUS_TX_DS)) {
        handle_TX_DS();
    }

    // if (not readBit<uint8_t>(status, STATUS_TX_FULL)) {
    writeTxFifo();
        // }
}

static inline uint8_t extractPipeIndex(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

int8_t nRF24L01P_ESB::readRxFifo() {
    int8_t status     = cmd_NOP();
    uint8_t pipeIndex = extractPipeIndex(status);

    if (pipeIndex > 5) {
        return (-1);
    }

    if (_rxQueue[pipeIndex] == NULL) {
        return (-1);
    }

    uint8_t numBytes;

    cmd_R_RX_PL_WID(numBytes);

    if (numBytes > rxFifoSize) {
        return (-1);
    }

    uint8_t bytes[numBytes];

    cmd_R_RX_PAYLOAD(bytes, numBytes);

    for (int i = 0; i < numBytes; ++i) {
        _rxQueue[pipeIndex]->enqueue(bytes[i]);
    }

    return (0);
}

int8_t nRF24L01P_ESB::writeTxFifo() {
    if (_txQueue == NULL) {
        return (-1);
    }

    UBaseType_t usedSlots = _txQueue->queueMessagesWaiting();

    if (usedSlots == 0) {
        return (-1);
    }

    uint8_t numBytes = min(txFifoSize, usedSlots);
    uint8_t bytes[numBytes];

    for (int i = 0; i < numBytes; ++i) {
        _txQueue->dequeue(bytes[i]);
    }

    cmd_W_TX_PAYLOAD(bytes, numBytes);

    return (0);
}

void nRF24L01P_ESB::handle_MAX_RT() {
    cmd_FLUSH_TX();
    writeShortRegister(Register_STATUS, STATUS_MAX_RT_MASK);
}

void nRF24L01P_ESB::handle_RX_DR() {
    // uint8_t fifo_status;

    // do {
    int8_t rxStatus = readRxFifo();

    // if (rxStatus < 0) {
    // cmd_FLUSH_RX();
    // }

    // fifo_status = readShortRegister(Register_FIFO_STATUS);
    // } while (not readBit<uint8_t>(fifo_status, FIFO_STATUS_RX_EMPTY));

    writeShortRegister(Register_STATUS, STATUS_RX_DR_MASK);
}

void nRF24L01P_ESB::handle_TX_DS() {
    // uint8_t fifo_status;

    // do {
    // int8_t txStatus = writeTxFifo();

    // if (txStatus < 0) {
    //     break;
    // }

    // fifo_status = readShortRegister(Register_FIFO_STATUS);
    // } while (readBit<uint8_t>(fifo_status, FIFO_STATUS_TX_FULL) == false);

    writeShortRegister(Register_STATUS, STATUS_TX_DS_MASK);
}

void nRF24L01P_ESB::enterRxMode() {
    switchOperatingMode(OperatingMode_Standby);

    // Enter PRX mode
    setSingleBit(Register_CONFIG, CONFIG_PRIM_RX);

    _ce.set();

    delayUs(rxSettling);
}

void nRF24L01P_ESB::enterShutdownMode() {
    _ce.clear();

    clearSingleBit(Register_CONFIG, CONFIG_PWR_UP);
}

void nRF24L01P_ESB::enterStandbyMode() {
    _ce.clear();

    setSingleBit(Register_CONFIG, CONFIG_PWR_UP);
}

void nRF24L01P_ESB::enterTxMode() {
    switchOperatingMode(OperatingMode_Standby);

    // Enter PTX mode
    clearSingleBit(Register_CONFIG, CONFIG_PRIM_RX);

    _ce.set();

    delayUs(txSettling);
}

void nRF24L01P_ESB::switchOperatingMode(OperatingMode_t operatingMode) {
    if (_operatingMode == operatingMode) {
        // return;
    }

    switch (operatingMode) {
        case OperatingMode_Rx: {
            enterRxMode();
        } break;
        case OperatingMode_Shutdown: {
            enterShutdownMode();
        } break;
        case OperatingMode_Standby: {
            enterStandbyMode();
        } break;
        case OperatingMode_Tx: {
            enterTxMode();
        } break;
    }

    _operatingMode = operatingMode;
}

void nRF24L01P_ESB::configureTxPipe(Queue<uint8_t> &txQueue, uint64_t txAddress) {
    setTxAddress(txAddress);
    setRxAddress(0, txAddress);

    enableDataPipe(0);

    _txQueue = &txQueue;
}

void nRF24L01P_ESB::configureRxPipe(uint8_t pipe, Queue<uint8_t> &rxQueue, uint64_t rxAddress) {
    assert(pipe <= 5);

    setRxAddress(pipe, rxAddress);

    enableDataPipe(pipe);

    _rxQueue[pipe] = &rxQueue;
}

} /* namespace xXx */
