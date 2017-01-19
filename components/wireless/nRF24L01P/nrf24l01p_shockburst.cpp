#include <assert.h>
#include <stdint.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nrf24l01p_shockburst.hpp>
#include <xXx/components/wireless/nRF24L01P/nrf24l01p_definitions.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

namespace xXx {

static const uint8_t rxSettling = 130;
static const uint8_t txSettling = 130;

nRF24L01P_ShockBurst::nRF24L01P_ShockBurst(ISpi &spi, IGpio &ce, IGpio &irq)
    : nRF24L01P(spi), _ce(ce), _irq(irq), _txQueue(NULL),
      _rxQueue{NULL, NULL, NULL, NULL, NULL, NULL},
      _operatingMode(OperatingMode_t::Shutdown) {}

nRF24L01P_ShockBurst::~nRF24L01P_ShockBurst() {}

static inline uint8_t getPipeIndex(uint8_t status) {
    bitwiseAND_r(status, VALUE_8(STATUS_t::RX_P_NO_MASK));
    shiftRight_r(status, VALUE_8(STATUS_t::RX_P_NO));

    return (status);
}

void nRF24L01P_ShockBurst::enterRxMode() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_ShockBurst *self = static_cast<nRF24L01P_ShockBurst *>(user);

        uint8_t fifo_status;
        uint8_t pipeIndex;
        uint8_t rxNumBytes;
        uint8_t status;

        do {
            status = self->cmd_NOP();

            // TODO: Do something if rxNumBytes > 32
            rxNumBytes = self->getPayloadLength();

            // TODO: Do something if pipeIndex > 5
            pipeIndex = getPipeIndex(status);

            uint8_t rxBytes[rxNumBytes];
            self->cmd_R_RX_PAYLOAD(rxBytes, rxNumBytes);

            for (int i = 0; i < rxNumBytes; ++i) {
                if (!(self->_rxQueue[pipeIndex])) {
                    break;
                }

                // TODO: Check for enqueue return
                self->_rxQueue[pipeIndex]->enqueue(rxBytes[i], true);
            }

            self->clearInterruptFlags();
            fifo_status = self->readShortRegister(Register_t::FIFO_STATUS);
        } while (!(readBit(fifo_status, VALUE_8(FIFO_STATUS_t::RX_EMPTY))));
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

void nRF24L01P_ShockBurst::enterShutdownMode() {
    if (_operatingMode == OperatingMode_t::Rx |
        _operatingMode == OperatingMode_t::Tx) {
        enterStandbyMode();
    }

    clearSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}

void nRF24L01P_ShockBurst::enterStandbyMode() {
    _irq.disableInterrupt();
    _ce.clear();

    setSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}

void nRF24L01P_ShockBurst::enterTxMode() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_ShockBurst *self = static_cast<nRF24L01P_ShockBurst *>(user);

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

void nRF24L01P_ShockBurst::switchOperatingMode(OperatingMode_t operatingMode) {
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

void nRF24L01P_ShockBurst::init() {
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

void nRF24L01P_ShockBurst::update() {
    if (_txQueue) {
        UBaseType_t usedSlots = _txQueue->usedSlots();

        if (usedSlots) {
            uint8_t status = cmd_NOP();

            if (readBit(status, VALUE_8(STATUS_t::TX_FULL))) {
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

void nRF24L01P_ShockBurst::enableDataPipe(uint8_t pipe, bool enable) {
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

void nRF24L01P_ShockBurst::configureRxPipe(uint8_t pipe, Queue<uint8_t> &queue,
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

void nRF24L01P_ShockBurst::configureTxPipe(Queue<uint8_t> &queue, uint64_t address) {
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
