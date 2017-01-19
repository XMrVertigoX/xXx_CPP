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

namespace xXx {

static const uint8_t rxSettling = 130;
static const uint8_t txSettling = 130;

nRF24L01P_API::nRF24L01P_API(ISpi &spi, IGpio &ce, IGpio &irq)
    : nRF24L01P_BASE(spi), _ce(ce), _irq(irq), _txQueue(NULL),
      _rxQueue{NULL, NULL, NULL, NULL, NULL, NULL},
      _operatingMode(OperatingMode_t::Shutdown) {}

nRF24L01P_API::~nRF24L01P_API() {}

static inline uint8_t getPipeIndex(uint8_t status) {
    bitwiseAND_r(status, VALUE_8(STATUS_t::RX_P_NO_MASK));
    shiftRight_r(status, VALUE_8(STATUS_t::RX_P_NO));

    return (status);
}

void nRF24L01P_API::enterRxMode() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_API *self = static_cast<nRF24L01P_API *>(user);

        uint8_t status = self->cmd_NOP();

        if (!(readBit(status, VALUE_8(STATUS_t::RX_DR)))) {
            self->clearInterruptFlags();
            return;
        }

        uint8_t fifo_status = self->readShortRegister(Register_t::FIFO_STATUS);

        while (!(readBit(fifo_status, VALUE_8(FIFO_STATUS_t::RX_EMPTY)))) {
            uint8_t rxNumBytes = self->getPayloadLength();
            uint8_t rxBytes[rxNumBytes];

            if (rxNumBytes > 32) {
                self->cmd_FLUSH_RX();
            } else {
                self->cmd_R_RX_PAYLOAD(rxBytes, rxNumBytes);
            }

            uint8_t pipeIndex = getPipeIndex(status);

            if ((pipeIndex <= 5) && (self->_rxQueue[pipeIndex] != NULL)) {
                // uint8_t rxBytes[rxNumBytes];

                for (int i = 0; i < rxNumBytes; ++i) {
                    self->_rxQueue[pipeIndex]->enqueue(rxBytes[i], true);
                }
            }

            self->clearInterruptFlags();
            fifo_status = self->readShortRegister(Register_t::FIFO_STATUS);
        }
    };

    if (_operatingMode != OperatingMode_t::Standby) {
        enterStandbyMode();
    }

    // Enter PRX mode
    setSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PRIM_RX));

    _irq.enableInterrupt(interruptFunction, this);
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
    _irq.disableInterrupt();
    _ce.clear();

    setSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}

void nRF24L01P_API::enterTxMode() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_API *self = static_cast<nRF24L01P_API *>(user);

        uint8_t status = self->cmd_NOP();

        if (readBit(status, VALUE_8(STATUS_t::MAX_RT))) {
            self->clearInterruptFlags();
        }

        if (readBit(status, VALUE_8(STATUS_t::TX_DS))) {
            self->clearInterruptFlags();
        }
    };

    if (_operatingMode != OperatingMode_t::Standby) {
        enterStandbyMode();
    }

    // Enter PTX mode
    clearSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PRIM_RX));

    _irq.enableInterrupt(interruptFunction, this);
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

void nRF24L01P_API::init() {
    clearInterruptFlags();

    // Enable Enhanced ShockBurst™
    uint8_t feature = readShortRegister(Register_t::FEATURE);
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DYN_ACK));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_ACK_PAY));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DPL));
    writeShortRegister(Register_t::FEATURE, feature);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();
}

void nRF24L01P_API::update() {
    if (_txQueue) {
        UBaseType_t usedSlots = _txQueue->usedSlots();

        if (usedSlots) {
            uint8_t fifo_status = readShortRegister(Register_t::FIFO_STATUS);

            if (readBit(fifo_status, VALUE_8(FIFO_STATUS_t::TX_FULL))) {
                return;
            }

            uint8_t buffer[usedSlots];

            for (int i = 0; i < usedSlots; ++i) {
                _txQueue->dequeue(buffer[i]);
            }

            cmd_W_TX_PAYLOAD(buffer, usedSlots);
        }
    }
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
                                    uint64_t address) {
    assert(!(pipe > 5));

    if (pipe > 5) {
        return;
    }

    if (address > 0) {
        setRxAddress(pipe, address);
    }

    enableDataPipe(pipe, true);

    _rxQueue[pipe] = &queue;
}

void nRF24L01P_API::configureTxPipe(Queue<uint8_t> &queue, uint64_t address) {
    if (address > 0) {
        setTxAddress(address);
    } else {
        address = getTxAddress();
    }

    setRxAddress(0, address);

    enableDataPipe(0, true);

    _txQueue = &queue;
}

} /* namespace xXx */
