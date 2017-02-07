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

static const uint8_t txSettling = 130;
static const uint8_t rxSettling = 130;
static const uint8_t txFifoSize = 32;
static const uint8_t rxFifoSize = 32;

namespace xXx {

nRF24L01P_ESB::nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq)
    : ArduinoTask(256, 2), _ce(ce), _irq(irq), _spi(spi),
      _rxQueue{NULL, NULL, NULL, NULL, NULL, NULL}, _txQueue(NULL),
      _operatingMode(OperatingMode_Shutdown) {}

nRF24L01P_ESB::~nRF24L01P_ESB() {}

void nRF24L01P_ESB::transmit_receive(Queue<uint8_t> &queue) {
    _spi.transmit_receive(queue);
}

void nRF24L01P_ESB::setup() {
    auto interruptFunction = LAMBDA(void *user) {
        static_cast<nRF24L01P_ESB *>(user)->notifyFromISR();
    };

    // Enable Enhanced ShockBurst™
    uint8_t feature = readShortRegister(Register_FEATURE);
    setBit_eq<uint8_t>(feature, FEATURE_EN_DYN_ACK);
    setBit_eq<uint8_t>(feature, FEATURE_EN_ACK_PAY);
    setBit_eq<uint8_t>(feature, FEATURE_EN_DPL);
    writeShortRegister(Register_FEATURE, feature);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    // Clear interrupts
    uint8_t status = 0;
    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    writeShortRegister(Register_STATUS, status);

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_ESB::loop() {
    notifyTake(true);

    uint8_t status = cmd_NOP();

    if (readBit<uint8_t>(status, STATUS_RX_DR)) {
        handle_RX_DR(status);
    }

    if (readBit<uint8_t>(status, STATUS_MAX_RT)) {
        handle_MAX_RT(status);
    }

    if (readBit<uint8_t>(status, STATUS_TX_DS)) {
        handle_TX_DS(status);
    }
}

static inline uint8_t extractPipeIndex(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

uint8_t nRF24L01P_ESB::readRxFifo(uint8_t status) {
    uint8_t rxNumBytes;
    uint8_t pipeIndex;

    pipeIndex = extractPipeIndex(status);

    if (pipeIndex > 5) {
        return (EXIT_FAILURE);
    }

    if (_rxQueue[pipeIndex] == NULL) {
        return (EXIT_FAILURE);
    }

    cmd_R_RX_PL_WID(rxNumBytes);

    if (rxNumBytes > rxFifoSize) {
        // TODO: Flush rx fifo?
        return (EXIT_FAILURE);
    }

    uint8_t rxBytes[rxNumBytes];

    cmd_R_RX_PAYLOAD(rxBytes, rxNumBytes);

    for (int i = 0; i < rxNumBytes; ++i) {
        _rxQueue[pipeIndex]->enqueue(rxBytes[i], true);
    }

    return (EXIT_SUCCESS);
}

uint8_t nRF24L01P_ESB::writeTxFifo(uint8_t status) {
    if (_txQueue == NULL) {
        return (EXIT_FAILURE);
    }

    UBaseType_t usedSlots = _txQueue->usedSlots();

    if (usedSlots == 0) {
        return (EXIT_FAILURE);
    }

    uint8_t numBytes = min(txFifoSize, usedSlots);
    uint8_t bytes[numBytes];

    for (int i = 0; i < numBytes; ++i) {
        _txQueue->dequeue(bytes[i]);
    }

    cmd_W_TX_PAYLOAD(bytes, numBytes);

    return (EXIT_SUCCESS);
}

void nRF24L01P_ESB::handle_MAX_RT(uint8_t status) {
    writeShortRegister(Register_STATUS, STATUS_MAX_RT_MASK);
}

void nRF24L01P_ESB::handle_RX_DR(uint8_t status) {
    uint8_t fifo_status;

    do {
        uint8_t failure = readRxFifo(status);

        if (failure) {
            break;
        }

        fifo_status = readShortRegister(Register_FIFO_STATUS);
    } while (readBit<uint8_t>(fifo_status, FIFO_STATUS_RX_EMPTY) == false);

    writeShortRegister(Register_STATUS, STATUS_RX_DR_MASK);
}

void nRF24L01P_ESB::handle_TX_DS(uint8_t status) {
    uint8_t fifo_status;

    do {
        uint8_t failure = writeTxFifo(status);

        if (failure) {
            break;
        }

        fifo_status = readShortRegister(Register_FIFO_STATUS);
    } while (readBit<uint8_t>(fifo_status, FIFO_STATUS_TX_FULL) == false);

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
        return;
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
