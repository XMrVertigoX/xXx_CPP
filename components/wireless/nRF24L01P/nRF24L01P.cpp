#include <assert.h>
#include <stdint.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nRF24L01P_register_map.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

static const uint8_t rxSettling = 130;
static const uint8_t txSettling = 130;

nRF24L01P::nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq), _txQueue(NULL),
      _rxQueue{NULL, NULL, NULL, NULL, NULL, NULL}, _MAX_RT(false),
      _TX_DS(false), _RX_DR(false) {}

nRF24L01P::~nRF24L01P() {}

static inline uint8_t getPipeIndex(uint8_t status) {
    bitwiseAND_r(status, VALUE_8(STATUS_t::RX_P_NO_MASK));
    shiftRight_r(status, VALUE_8(STATUS_t::RX_P_NO));

    return (status);
}

void nRF24L01P::enterRxMode() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P *self = static_cast<nRF24L01P *>(user);
        uint8_t status  = self->cmd_NOP();

        if (readBit(status, VALUE_8(STATUS_t::RX_DR))) {
            self->_RX_DR = true;
        }
    };

    uint8_t config = readShortRegister(Register_t::CONFIG);

    // Enter PRX mode
    setBit_r(config, VALUE_8(CONFIG_t::PRIM_RX));
    // Enable RX interrupt
    clearBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));

    writeShortRegister(Register_t::CONFIG, config);

    _irq.enableInterrupt(interruptFunction, this);
    _ce.set();

    delayUs(rxSettling);
}

void nRF24L01P::leaveRxMode() {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    // Disable RX interrupt
    setBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));

    writeShortRegister(Register_t::CONFIG, config);

    _irq.disableInterrupt();
    _ce.clear();
}

void nRF24L01P::enterTxMode() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P *self = static_cast<nRF24L01P *>(user);
        uint8_t status  = self->cmd_NOP();

        if (readBit(status, VALUE_8(STATUS_t::MAX_RT))) {
            self->_MAX_RT = true;
        }

        if (readBit(status, VALUE_8(STATUS_t::TX_DS))) {
            self->_TX_DS = true;
        }
    };

    uint8_t config = readShortRegister(Register_t::CONFIG);

    // Enter PTX mode
    clearBit_r(config, VALUE_8(CONFIG_t::PRIM_RX));
    // Enable TX retransmit interrupt
    clearBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    // Enable TX interrupt
    clearBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));

    writeShortRegister(Register_t::CONFIG, config);

    _irq.enableInterrupt(interruptFunction, this);
    _ce.set();

    delayUs(txSettling);
}

void nRF24L01P::leaveTxMode() {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    // Disable TX retransmit interrupt
    setBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    // Disable TX interrupt
    setBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));

    writeShortRegister(Register_t::CONFIG, config);

    _irq.disableInterrupt();
    _ce.clear();
}

void nRF24L01P::init() {
    // Clear IRQs
    uint8_t status = readShortRegister(Register_t::STATUS);
    setBit_r(status, VALUE_8(STATUS_t::MAX_RT));
    setBit_r(status, VALUE_8(STATUS_t::TX_DS));
    setBit_r(status, VALUE_8(STATUS_t::RX_DR));
    writeShortRegister(Register_t::STATUS, status);

    // Disable IRQs
    uint8_t config = readShortRegister(Register_t::CONFIG);
    setBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    setBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));
    setBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));
    writeShortRegister(Register_t::CONFIG, config);

    uint8_t dynpd = readShortRegister(Register_t::DYNPD);
    setBit_r(dynpd, VALUE_8(DYNPD_t::DPL_P0));
    setBit_r(dynpd, VALUE_8(DYNPD_t::DPL_P1));
    setBit_r(dynpd, VALUE_8(DYNPD_t::DPL_P2));
    setBit_r(dynpd, VALUE_8(DYNPD_t::DPL_P3));
    setBit_r(dynpd, VALUE_8(DYNPD_t::DPL_P4));
    setBit_r(dynpd, VALUE_8(DYNPD_t::DPL_P5));
    writeShortRegister(Register_t::DYNPD, dynpd);

    uint8_t feature = readShortRegister(Register_t::FEATURE);
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DYN_ACK));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_ACK_PAY));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DPL));
    writeShortRegister(Register_t::FEATURE, feature);

    setRetries(0xF, 0xF);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.disableInterrupt();
    _ce.clear();
}

void nRF24L01P::update() {
    if (_MAX_RT) {
        LOG("_MAX_RT");
    }

    if (_TX_DS) {
    }

    if (_RX_DR) {
        uint8_t fifo_status;
        uint8_t status;
        uint8_t pipe;
        uint8_t rxNumBytes;

        do {
            status = cmd_NOP();

            rxNumBytes = getPayloadLength();
            uint8_t rxBytes[rxNumBytes];
            cmd_R_RX_PAYLOAD(rxBytes, rxNumBytes);

            pipe = getPipeIndex(status);

            if (pipe < 0b110) {
                for (int i = 0; i < rxNumBytes; ++i) {
                    if (_rxQueue[pipe]) {
                        _rxQueue[pipe]->enqueue(rxBytes[i], true);
                    }
                }
            }

            cmd_W_REGISTER(Register_t::STATUS, &status, sizeof(status));

            fifo_status = readShortRegister(Register_t::FIFO_STATUS);
        } while (!(readBit(fifo_status, VALUE_8(FIFO_STATUS_t::RX_EMPTY))));

        _RX_DR = false;
    }

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

void nRF24L01P::configureRxPipe(uint8_t pipe, Queue<uint8_t> &queue,
                                uint64_t address) {
    assert(pipe < 6);

    if (address > 0) {
        setRxAddress(pipe, address);
    }

    switch (pipe) {
        case 0: {
            setSingleBit(Register_t::EN_RXADDR, VALUE_8(EN_RXADDR_t::ERX_P0));
        } break;
        case 1: {
            setSingleBit(Register_t::EN_RXADDR, VALUE_8(EN_RXADDR_t::ERX_P1));
        } break;
        case 2: {
            setSingleBit(Register_t::EN_RXADDR, VALUE_8(EN_RXADDR_t::ERX_P2));
        } break;
        case 3: {
            setSingleBit(Register_t::EN_RXADDR, VALUE_8(EN_RXADDR_t::ERX_P3));
        } break;
        case 4: {
            setSingleBit(Register_t::EN_RXADDR, VALUE_8(EN_RXADDR_t::ERX_P4));
        } break;
        case 5: {
            setSingleBit(Register_t::EN_RXADDR, VALUE_8(EN_RXADDR_t::ERX_P5));
        } break;
    }

    _rxQueue[pipe] = &queue;
}

void nRF24L01P::configureTxPipe(Queue<uint8_t> &queue, uint64_t address) {
    if (address > 0) {
        setTxAddress(address);
    } else {
        address = getTxAddress();
    }

    setRxAddress(0, address);

    _txQueue = &queue;
}

void nRF24L01P::powerUp() {
    setSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}

void nRF24L01P::powerDown() {
    clearSingleBit(Register_t::CONFIG, VALUE_8(CONFIG_t::PWR_UP));
}
