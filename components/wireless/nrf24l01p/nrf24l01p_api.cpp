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
    : ArduinoTask(256, 2), _ce(ce), _irq(irq), _spi(spi), _txQueue(NULL),
      _rxQueue{NULL, NULL, NULL, NULL, NULL, NULL},
      _operatingMode(OperatingMode_t::Shutdown) {}

nRF24L01P_API::~nRF24L01P_API() {}

void nRF24L01P_API::transmit_receive(Queue<uint8_t> mosiQueue,
                                     Queue<uint8_t> misoQueue) {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_API *self = static_cast<nRF24L01P_API *>(user);

        self->notifyFromISR();
    };

    _spi.transmit_receive(mosiQueue, misoQueue, interruptFunction, this);

    notifyTake(true);
}

void nRF24L01P_API::setup() {
    auto interruptFunction = LAMBDA(void *user) {
        nRF24L01P_API *self = static_cast<nRF24L01P_API *>(user);

        self->notifyFromISR();
    };

    // Mask all interrupts
    uint8_t config = readShortRegister(Register_t::CONFIG);
    setBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    setBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));
    setBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));
    writeShortRegister(Register_t::CONFIG, config);

    // Enable Enhanced ShockBurst™
    uint8_t feature = readShortRegister(Register_t::FEATURE);
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DYN_ACK));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_ACK_PAY));
    setBit_r(feature, VALUE_8(FEATURE_t::EN_DPL));
    writeShortRegister(Register_t::FEATURE, feature);

    clearInterrupts();

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_API::loop() {
    notifyTake(true);

    uint8_t status = cmd_NOP();

    if (readBit(status, VALUE_8(STATUS_t::RX_DR))) {
        handle_RX_DR(status);
        setSingleBit(Register_t::STATUS, VALUE_8(STATUS_t::RX_DR));
    }

    if (readBit(status, VALUE_8(STATUS_t::MAX_RT))) {
        handle_MAX_RT(status);
        setSingleBit(Register_t::STATUS, VALUE_8(STATUS_t::MAX_RT));
    }

    if (readBit(status, VALUE_8(STATUS_t::TX_DS))) {
        handle_TX_DS(status);
        setSingleBit(Register_t::STATUS, VALUE_8(STATUS_t::TX_DS));
    }

    if (transmitData(status)) {
        switchOperatingMode(OperatingMode_t::Tx);
    }
}

bool nRF24L01P_API::receiveData(UBaseType_t freeSlots) {
    return (false);
}

bool nRF24L01P_API::transmitData(uint8_t status) {
    if (!_txQueue) {
        return (false);
    }

    UBaseType_t usedSlots = _txQueue->usedSlots();

    if (!usedSlots) {
        return (false);
    }

    while (!(readBit(status, VALUE_8(STATUS_t::TX_FULL)))) {
        uint8_t numBytes = min(32, usedSlots);
        uint8_t bytes[numBytes];

        for (int i = 0; i < numBytes; ++i) {
            _txQueue->dequeue(bytes[i]);
        }

        cmd_W_TX_PAYLOAD(bytes, numBytes);

        status = cmd_NOP();
    }

    return (true);
}

void nRF24L01P_API::handle_MAX_RT(uint8_t status) {
    cmd_FLUSH_TX();
    switchOperatingMode(OperatingMode_t::Standby);
}

void nRF24L01P_API::handle_RX_DR(uint8_t status) {
    uint8_t fifo_status;

    do {
        uint8_t rxNumBytes = getPayloadLength();
        uint8_t rxBytes[min(32, rxNumBytes)];

        assert(rxNumBytes <= 32);

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
            }
        }

        fifo_status = readShortRegister(Register_t::FIFO_STATUS);
        status      = cmd_NOP();
    } while (!(readBit(fifo_status, VALUE_8(FIFO_STATUS_t::RX_EMPTY))));
}

void nRF24L01P_API::handle_TX_DS(uint8_t status) {
    transmitData(status);

    uint8_t fifo_status = readShortRegister(Register_t::FIFO_STATUS);

    if (readBit(fifo_status, VALUE_8(FIFO_STATUS_t::TX_EMPTY))) {
        switchOperatingMode(OperatingMode_t::Standby);
    } else {
        switchOperatingMode(OperatingMode_t::Tx);
    }
}

void nRF24L01P_API::enterRxMode() {
    switchOperatingMode(OperatingMode_t::Standby);

    // Enter PRX mode, unmask interrupt
    uint8_t config = readShortRegister(Register_t::CONFIG);
    clearBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));
    setBit_r(config, VALUE_8(CONFIG_t::PRIM_RX));
    writeShortRegister(Register_t::CONFIG, config);

    _ce.set();

    delayUs(rxSettling);
}

void nRF24L01P_API::enterShutdownMode() {
    _ce.clear();

    uint8_t config = readShortRegister(Register_t::CONFIG);
    //    setBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    //    setBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));
    //    setBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));
    clearBit_r(config, VALUE_8(CONFIG_t::PWR_UP));
    writeShortRegister(Register_t::CONFIG, config);
}

void nRF24L01P_API::enterStandbyMode() {
    _ce.clear();

    uint8_t config = readShortRegister(Register_t::CONFIG);
    //    setBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    //    setBit_r(config, VALUE_8(CONFIG_t::MASK_RX_DR));
    //    setBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));
    setBit_r(config, VALUE_8(CONFIG_t::PWR_UP));
    writeShortRegister(Register_t::CONFIG, config);
}

void nRF24L01P_API::enterTxMode() {
    switchOperatingMode(OperatingMode_t::Standby);

    // Enter PTX mode, unmask interrupts
    uint8_t config = readShortRegister(Register_t::CONFIG);
    clearBit_r(config, VALUE_8(CONFIG_t::MASK_MAX_RT));
    clearBit_r(config, VALUE_8(CONFIG_t::MASK_TX_DS));
    clearBit_r(config, VALUE_8(CONFIG_t::PRIM_RX));
    writeShortRegister(Register_t::CONFIG, config);

    _ce.set();

    delayUs(txSettling);
}

void nRF24L01P_API::switchOperatingMode(OperatingMode_t operatingMode) {
    if (_operatingMode == operatingMode) {
        return;
    }

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

} /* namespace xXx */
