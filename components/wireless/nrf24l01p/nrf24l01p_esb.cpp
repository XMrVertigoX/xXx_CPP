#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_esb.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

// TODO: Implement better indicators
#define __MAKE_SURE(expression) assert(expression)

#define __MAX(value, limit) (value > limit ? limit : value)
#define __MIN(value, limit) (value > limit ? limit : value)

namespace xXx {

union addressUnion_t {
    uint64_t m64;
    uint8_t m8[sizeof(int64_t)];
};

static inline uint8_t extractPipe(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

nRF24L01P_ESB::nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq)
    : nRF24L01P_BASE(spi), _ce(ce), _irq(irq) {}

nRF24L01P_ESB::~nRF24L01P_ESB() {
    switchOperatingMode(OperatingMode_Shutdown);
}

void nRF24L01P_ESB::setup() {
    LOG("%p: %s", this, __PRETTY_FUNCTION__);

    auto interruptFunction = [](void *user) {
        nRF24L01P_ESB *self = static_cast<nRF24L01P_ESB *>(user);
        self->notifyFromISR();
    };

    // Enable dynamic payload length only
    uint8_t feature = 0;
    clearBit_eq<uint8_t>(feature, FEATURE_EN_DYN_ACK);
    clearBit_eq<uint8_t>(feature, FEATURE_EN_ACK_PAY);
    setBit_eq<uint8_t>(feature, FEATURE_EN_DPL);
    cmd_W_REGISTER(Register_FEATURE, &feature, sizeof(feature));

    // Clear interrupts
    uint8_t status = 0;
    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_ESB::loop() {
    notifyTake();

    uint8_t status = cmd_NOP();

    if (readBit<uint8_t>(status, STATUS_RX_DR)) {
        handle_RX_DR(status);
    } else if (readBit<uint8_t>(status, STATUS_MAX_RT)) {
        handle_MAX_RT(status);
    } else if (readBit<uint8_t>(status, STATUS_TX_DS)) {
        handle_TX_DS(status);
    } else {
        writeTxFifo(status);
    }
}

void nRF24L01P_ESB::readRxFifo(uint8_t status) {
    uint8_t bytes[rxFifoSize];
    uint8_t numBytes;

    uint8_t pipe = extractPipe(status);

    if (pipe > 5) return;

    cmd_R_RX_PL_WID(numBytes);

    if (numBytes > rxFifoSize) {
        cmd_FLUSH_RX();
        return;
    }

    cmd_R_RX_PAYLOAD(bytes, numBytes);

    if (_rxCallback[pipe]) {
        _rxCallback[pipe](bytes, numBytes, _rxUser[pipe]);
    }
}

void nRF24L01P_ESB::writeTxFifo(uint8_t status) {
    if (readBit<uint8_t>(status, STATUS_TX_FULL)) return;

    uint8_t numBytes = __MAX(_txBufferEnd - _txBufferStart, txFifoSize);

    cmd_W_TX_PAYLOAD(&_txBuffer[_txBufferStart], numBytes);

    _txBufferStart += numBytes;
}

void nRF24L01P_ESB::handle_MAX_RT(uint8_t status) {
    cmd_FLUSH_TX();

    // TODO
    txCallback();

    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void nRF24L01P_ESB::handle_RX_DR(uint8_t status) {
    readRxFifo(status);

    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void nRF24L01P_ESB::handle_TX_DS(uint8_t status) {
    if (_txBufferEnd > _txBufferStart) {
        writeTxFifo(status);
    } else {
        txCallback();
    }

    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void nRF24L01P_ESB::txCallback() {
    assert(_txBufferStart == _txBufferEnd);

    if (_txCallback) {
        _txCallback(getRetransmissionCounter(), _txUser);
    }

    _txCallback = NULL;
    _txUser     = NULL;
}

void nRF24L01P_ESB::enterRxMode() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    setBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    _ce.set();

    delayUs(rxSettling);
}

void nRF24L01P_ESB::enterShutdownMode() {
    uint8_t config;

    _ce.clear();

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    clearBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));
}

void nRF24L01P_ESB::enterStandbyMode() {
    uint8_t config;

    _ce.clear();

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));
}

void nRF24L01P_ESB::enterTxMode() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    clearBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    _ce.set();

    delayUs(txSettling);
}

void nRF24L01P_ESB::configureTxPipe(uint64_t txAddress) {
    setTxAddress(txAddress);
    configureRxPipe(0, txAddress);
    enableDataPipe(0);
}

void nRF24L01P_ESB::configureRxPipe(uint8_t pipe, uint64_t rxAddress) {
    setRxAddress(pipe, rxAddress);
    enableDynamicPayloadLength(pipe);
}

void nRF24L01P_ESB::switchOperatingMode(OperatingMode_t operatingMode) {
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
}

uint8_t nRF24L01P_ESB::queueTransmission(uint8_t bytes[], size_t numBytes, txCallback_t callback,
                                         void *user) {
    if (_txBufferStart < _txBufferEnd) return (1);

    _txBuffer      = bytes;
    _txBufferStart = 0;
    _txBufferEnd   = numBytes;
    _txCallback    = callback;
    _txUser        = user;

    return (0);
}

uint8_t nRF24L01P_ESB::startListening(uint8_t pipe, rxCallback_t callback, void *user) {
    __MAKE_SURE(pipe < 6);

    _rxCallback[pipe] = callback;
    _rxUser[pipe]     = user;

    enableDataPipe(pipe);

    return (0);
}

uint8_t nRF24L01P_ESB::stopListening(uint8_t pipe) {
    __MAKE_SURE(pipe < 6);

    disableDataPipe(pipe);

    _rxCallback[pipe] = NULL;
    _rxUser[pipe]     = NULL;

    return (0);
}

// ----- getters and setters --------------------------------------------------

void nRF24L01P_ESB::enableDataPipe(uint8_t pipe) {
    __MAKE_SURE(pipe < 6);

    uint8_t en_rxaddr;

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    setBit_eq<uint8_t>(en_rxaddr, pipe);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    // TODO: __MAKE_SURE();
}

void nRF24L01P_ESB::disableDataPipe(uint8_t pipe) {
    __MAKE_SURE(pipe < 6);

    uint8_t en_rxaddr;

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    clearBit_eq<uint8_t>(en_rxaddr, pipe);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    // TODO: __MAKE_SURE();
}

void nRF24L01P_ESB::enableDynamicPayloadLength(uint8_t pipe) {
    __MAKE_SURE(pipe < 6);

    uint8_t dynpd;

    cmd_R_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));
    setBit_eq<uint8_t>(dynpd, pipe);
    cmd_W_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));

    // TODO: __MAKE_SURE();
}

void nRF24L01P_ESB::disableDynamicPayloadLength(uint8_t pipe) {
    __MAKE_SURE(pipe < 6);

    uint8_t dynpd;

    cmd_R_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));
    clearBit_eq<uint8_t>(dynpd, pipe);
    cmd_W_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));

    // TODO: __MAKE_SURE();
}

CRCConfig_t nRF24L01P_ESB::getCrcConfig() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));

    if (readBit<uint8_t>(config, CONFIG_EN_CRC) == false) {
        return (CRCConfig_DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (CrcConfig_2Bytes);
    } else {
        return (CRCConfig_1Byte);
    }
}

uint8_t nRF24L01P_ESB::setCrcConfig(CRCConfig_t crcConfig) {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));

    switch (crcConfig) {
        case CRCConfig_DISABLED: {
            clearBit_eq<uint8_t>(config, CONFIG_EN_CRC);
        } break;
        case CRCConfig_1Byte: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            clearBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
        case CrcConfig_2Bytes: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            setBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
    }

    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    __MAKE_SURE(crcConfig == getCrcConfig());

    return (0);
}

uint8_t nRF24L01P_ESB::getChannel() {
    uint8_t channel;

    cmd_R_REGISTER(Register_RF_CH, &channel, sizeof(channel));

    return (channel);
}

uint8_t nRF24L01P_ESB::setChannel(uint8_t channel) {
    __MAKE_SURE(channel < 0x80);

    cmd_W_REGISTER(Register_RF_CH, &channel, sizeof(channel));

    __MAKE_SURE(channel == getChannel());

    return (0);
}

DataRate_t nRF24L01P_ESB::getDataRate() {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW)) {
        return (DataRate_250KBPS);
    }

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH)) {
        return (DataRate_2MBPS);
    }

    return (DataRate_1MBPS);
}

uint8_t nRF24L01P_ESB::setDataRate(DataRate_t dataRate) {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    switch (dataRate) {
        case (DataRate_1MBPS): {
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_2MBPS: {
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            setBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_250KBPS: {
            setBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
    }

    cmd_W_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    __MAKE_SURE(dataRate == getDataRate());

    return (0);
}

OutputPower_t nRF24L01P_ESB::getOutputPower() {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    // TODO: Make sure that the following cast is save;
    assert(rf_setup < 4);

    return (static_cast<OutputPower_t>(rf_setup));
}

uint8_t nRF24L01P_ESB::setOutputPower(OutputPower_t outputPower) {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    AND_eq(rf_setup, INVERT<uint8_t>(RF_SETUP_RF_PWR_MASK));
    OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(outputPower, RF_SETUP_RF_PWR));

    cmd_W_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    __MAKE_SURE(outputPower == getOutputPower());

    return (0);
}

uint8_t nRF24L01P_ESB::getRetryCount() {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

uint8_t nRF24L01P_ESB::setRetryCount(uint8_t count) {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(__MAX(count, 0xF), SETUP_RETR_ARC));

    cmd_W_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    __MAKE_SURE(count == getRetryCount());

    return (0);
}

uint8_t nRF24L01P_ESB::getRetryDelay() {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

uint8_t nRF24L01P_ESB::setRetryDelay(uint8_t delay) {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(__MAX(delay, 0xF), SETUP_RETR_ARD));
    cmd_W_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    __MAKE_SURE(delay == getRetryDelay());

    return (0);
}

uint64_t nRF24L01P_ESB::getRxAddress(uint8_t pipe) {
    __MAKE_SURE(pipe < 6);

    addressUnion_t rxAddressUnion;
    rxAddressUnion.m64 = 0;

    switch (pipe) {
        case 0: {
            cmd_R_REGISTER(Register_RX_ADDR_P0, rxAddressUnion.m8, RX_ADDR_P0_LENGTH);
        } break;
        case 1: {
            cmd_R_REGISTER(Register_RX_ADDR_P1, rxAddressUnion.m8, RX_ADDR_P1_LENGTH);
        } break;
        case 2: {
            cmd_R_REGISTER(Register_RX_ADDR_P2, rxAddressUnion.m8, RX_ADDR_P2_LENGTH);
        } break;
        case 3: {
            cmd_R_REGISTER(Register_RX_ADDR_P3, rxAddressUnion.m8, RX_ADDR_P3_LENGTH);
        } break;
        case 4: {
            cmd_R_REGISTER(Register_RX_ADDR_P5, rxAddressUnion.m8, RX_ADDR_P4_LENGTH);
        } break;
        case 5: {
            cmd_R_REGISTER(Register_RX_ADDR_P5, rxAddressUnion.m8, RX_ADDR_P5_LENGTH);
        } break;
    }

    return (rxAddressUnion.m64);
}

uint8_t nRF24L01P_ESB::setRxAddress(uint8_t pipe, uint64_t rxAddress) {
    __MAKE_SURE(pipe < 6);

    addressUnion_t rxAddressUnion;
    rxAddressUnion.m64 = rxAddress;

    switch (pipe) {
        case 0: {
            cmd_W_REGISTER(Register_RX_ADDR_P0, rxAddressUnion.m8, RX_ADDR_P0_LENGTH);
        } break;
        case 1: {
            cmd_W_REGISTER(Register_RX_ADDR_P1, rxAddressUnion.m8, RX_ADDR_P1_LENGTH);
        } break;
        case 2: {
            cmd_W_REGISTER(Register_RX_ADDR_P2, rxAddressUnion.m8, RX_ADDR_P2_LENGTH);
        } break;
        case 3: {
            cmd_W_REGISTER(Register_RX_ADDR_P3, rxAddressUnion.m8, RX_ADDR_P3_LENGTH);
        } break;
        case 4: {
            cmd_W_REGISTER(Register_RX_ADDR_P4, rxAddressUnion.m8, RX_ADDR_P4_LENGTH);
        } break;
        case 5: {
            cmd_W_REGISTER(Register_RX_ADDR_P5, rxAddressUnion.m8, RX_ADDR_P5_LENGTH);
        } break;
    }

    __MAKE_SURE(rxAddress == getRxAddress(pipe));

    return (0);
}

uint64_t nRF24L01P_ESB::getTxAddress() {
    addressUnion_t txAddressUnion;
    txAddressUnion.m64 = 0;

    cmd_R_REGISTER(Register_TX_ADDR, txAddressUnion.m8, TX_ADDR_LENGTH);

    return (txAddressUnion.m64);
}

uint8_t nRF24L01P_ESB::setTxAddress(uint64_t txAddress) {
    addressUnion_t txAddressUnion;
    txAddressUnion.m64 = txAddress;

    cmd_W_REGISTER(Register_TX_ADDR, txAddressUnion.m8, TX_ADDR_LENGTH);

    __MAKE_SURE(txAddress == getTxAddress());

    return (0);
}

uint8_t nRF24L01P_ESB::getPackageLossCounter() {
    uint8_t observe_tx;

    cmd_R_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT);
    cmd_W_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));

    return (observe_tx);
}

uint8_t nRF24L01P_ESB::getRetransmissionCounter() {
    uint8_t observe_tx;

    cmd_R_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT);
    cmd_W_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));

    return (observe_tx);
}

} /* namespace xXx */
