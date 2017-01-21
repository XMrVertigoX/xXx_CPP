#include <assert.h>
#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_api.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define LAMBDA []

#define min(a, b) (a < b ? a : b)

namespace xXx {

static const uint8_t rxSettling = 130;
static const uint8_t txSettling = 130;

static inline uint8_t getPipeIndex(uint8_t status) {
    bitwiseAND_r(status, VALUE_8(STATUS_t::RX_P_NO_MASK));
    shiftRight_r(status, VALUE_8(STATUS_t::RX_P_NO));

    return (status);
}

nRF24L01P_API::nRF24L01P_API(ISpi &spi, IGpio &ce, IGpio &irq)
    : nRF24L01P_BASE(spi), ArduinoTask(256, 2), _ce(ce), _irq(irq),
      _txQueue(NULL), _rxQueue{NULL, NULL, NULL, NULL, NULL, NULL},
      _operatingMode(OperatingMode_t::Shutdown) {}

nRF24L01P_API::~nRF24L01P_API() {}

void nRF24L01P_API::setup() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_API *self = static_cast<nRF24L01P_API *>(user);

        BaseType_t taskSwitchRequested = pdFALSE;
        vTaskNotifyGiveFromISR(self->_handle, &taskSwitchRequested);
        portYIELD_FROM_ISR(taskSwitchRequested);
    };

    clearInterrupts();

    // Enable Enhanced ShockBurst™
    uint8_t feature = readShortRegister(Register_t::FEATURE);
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DYN_ACK));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_ACK_PAY));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DPL));
    writeShortRegister(Register_t::FEATURE, feature);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_API::loop() {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    uint8_t status = cmd_NOP();

    if (readBit(status, VALUE_8(STATUS_t::MAX_RT))) {
        handle_MAX_RT(status);
        setSingleBit(Register_t::STATUS, VALUE_8(STATUS_t::MAX_RT));
    }

    if (readBit(status, VALUE_8(STATUS_t::RX_DR))) {
        handle_RX_DR(status);
        setSingleBit(Register_t::STATUS, VALUE_8(STATUS_t::RX_DR));
    }

    if (readBit(status, VALUE_8(STATUS_t::TX_DS))) {
        handle_TX_DS(status);
        setSingleBit(Register_t::STATUS, VALUE_8(STATUS_t::TX_DS));
    }

    if (!(readBit(status, VALUE_8(STATUS_t::TX_FULL)))) {
        foo();
    }
}

void nRF24L01P_API::foo() {
    if (_txQueue == NULL) {
        return;
    }

    UBaseType_t usedSlots = _txQueue->usedSlots();

    if (!usedSlots) {
        return;
    }

    uint8_t status = cmd_NOP();

    while (!(readBit(status, VALUE_8(STATUS_t::TX_FULL)))) {
        uint8_t numBytes = min(32, usedSlots);
        uint8_t bytes[numBytes];

        for (int i = 0; i < numBytes; ++i) {
            _txQueue->dequeue(bytes[i]);
        }

        status = cmd_W_TX_PAYLOAD(bytes, numBytes);
    }

    switchOperatingMode(OperatingMode_t::Tx);
}

void nRF24L01P_API::handle_MAX_RT(uint8_t status) {
    switchOperatingMode(OperatingMode_t::Standby);
}

void nRF24L01P_API::handle_RX_DR(uint8_t status) {
    portENTER_CRITICAL();

    uint8_t fifo_status;

    do {
        uint8_t status     = cmd_NOP();
        uint8_t rxNumBytes = getPayloadLength();
        uint8_t rxBytes[min(32, rxNumBytes)];

        assert(!(rxNumBytes > 32));

        if (rxNumBytes > 32) {
            cmd_FLUSH_RX();
        } else {
            cmd_R_RX_PAYLOAD(rxBytes, rxNumBytes);
        }

        uint8_t pipeIndex = getPipeIndex(status);

        if ((pipeIndex <= 5) && (_rxQueue[pipeIndex] != NULL)) {
            for (int i = 0; i < rxNumBytes; ++i) {
                BaseType_t success =
                    _rxQueue[pipeIndex]->enqueue(rxBytes[i], true);

                // assert(success == pdTRUE);
            }
        }

        fifo_status = readShortRegister(Register_t::FIFO_STATUS);
    } while (!(readBit(fifo_status, VALUE_8(FIFO_STATUS_t::RX_EMPTY))));

    portEXIT_CRITICAL();
}

void nRF24L01P_API::handle_TX_DS(uint8_t status) {
    portENTER_CRITICAL();

    if (_txQueue) {
        UBaseType_t usedSlots = _txQueue->usedSlots();

        if (usedSlots) {
            while (!(readBit(status, VALUE_8(STATUS_t::TX_FULL)))) {
                uint8_t numBytes = min(32, usedSlots);
                uint8_t bytes[numBytes];

                for (int i = 0; i < numBytes; ++i) {
                    _txQueue->dequeue(bytes[i]);
                }

                status = cmd_W_TX_PAYLOAD(bytes, numBytes);
            }
        } else {
            switchOperatingMode(OperatingMode_t::Standby);
        }
    }

    portEXIT_CRITICAL();
}

void nRF24L01P_API::enterRxMode() {
    if (_operatingMode == OperatingMode_t::Rx) {
        return;
    }

    if (_operatingMode != OperatingMode_t::Standby) {
        enterStandbyMode();
    }

    // Enter PRX mode
    setSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PRIM_RX));

    _ce.set();

    delayUs(rxSettling);
}

void nRF24L01P_API::enterShutdownMode() {
    if (_operatingMode == OperatingMode_t::Rx |
        _operatingMode == OperatingMode_t::Tx) {
        enterStandbyMode();
    }

    clearSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}

void nRF24L01P_API::enterStandbyMode() {
    _ce.clear();

    setSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}

void nRF24L01P_API::enterTxMode() {
    if (_operatingMode == OperatingMode_t::Tx) {
        return;
    }

    if (_operatingMode != OperatingMode_t::Standby) {
        enterStandbyMode();
    }

    // Enter PTX mode
    clearSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PRIM_RX));

    _ce.set();

    delayUs(txSettling);
}

void nRF24L01P_API::switchOperatingMode(OperatingMode_t operatingMode) {
    switch (operatingMode) {
        case OperatingMode_t::Rx: {
            enterRxMode();
        } break;
        case OperatingMode_t::Shutdown: {
            enterShutdownMode();
        } break;
        case OperatingMode_t::Standby: {
            enterStandbyMode();
        } break;
        case OperatingMode_t::Tx: {
            enterTxMode();
        } break;
    }

    _operatingMode = operatingMode;
}

void nRF24L01P_API::enableDataPipe(uint8_t pipe, bool enable) {
    if (pipe > 5) {
        return;
    }

    if (enable) {
        setSingleBit(Register_t::EN_RXADDR, pipe);
        setSingleBit(Register_t::DYNPD, pipe);
    } else {
        clearSingleBit(Register_t::EN_RXADDR, pipe);
        clearSingleBit(Register_t::DYNPD, pipe);
    }
}

void nRF24L01P_API::configureRxPipe(uint8_t pipe, Queue<uint8_t> &queue,
                                    uint64_t rxAddress) {
    assert(!(pipe > 5));

    if (rxAddress > 0) {
        setRxAddress(pipe, rxAddress);
    }

    enableDataPipe(pipe, true);

    _rxQueue[pipe] = &queue;
}

void nRF24L01P_API::configureTxPipe(Queue<uint8_t> &queue, uint64_t txAddress) {
    if (txAddress > 0) {
        setTxAddress(txAddress);
    } else {
        txAddress = getTxAddress();
    }

    setRxAddress(0, txAddress);

    enableDataPipe(0, true);

    _txQueue = &queue;
}

void nRF24L01P_API::send(uint8_t bytes[], size_t numBytes) {}

} /* namespace xXx */
