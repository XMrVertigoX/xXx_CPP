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

#define constrain(value, limit) (value > limit ? limit : value)
#define isPipeIndexValid(pipeIndex) (pipeIndex > 5 ? false : true)

namespace xXx {

union addressUnion_t {
    int64_t m64;
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
    writeShortRegister(Register_FEATURE, feature);

    // Clear interrupts
    uint8_t status = 0;
    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    writeShortRegister(Register_STATUS, status);

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();

    _irq.enableInterrupt(interruptFunction, this);
}

void nRF24L01P_ESB::loop() {
    notifyTake();

    int8_t status = cmd_NOP();

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

void nRF24L01P_ESB::readRxFifo(int8_t status) {
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

void nRF24L01P_ESB::writeTxFifo(int8_t status) {
    if (readBit<int8_t>(status, STATUS_TX_FULL)) return;

    uint8_t numBytes = constrain(_txBufferEnd - _txBufferStart, txFifoSize);

    cmd_W_TX_PAYLOAD(&_txBuffer[_txBufferStart], numBytes);

    _txBufferStart += numBytes;
}

void nRF24L01P_ESB::handle_MAX_RT(int8_t status) {
    cmd_FLUSH_TX();
    txCallback();
    writeShortRegister(Register_STATUS, STATUS_MAX_RT_MASK);
}

void nRF24L01P_ESB::handle_RX_DR(int8_t status) {
    readRxFifo(status);
    writeShortRegister(Register_STATUS, STATUS_RX_DR_MASK);
}

void nRF24L01P_ESB::handle_TX_DS(int8_t status) {
    if (_txBufferEnd > _txBufferStart) {
        writeTxFifo(status);
    } else {
        txCallback();
    }

    writeShortRegister(Register_STATUS, STATUS_TX_DS_MASK);
}

void nRF24L01P_ESB::txCallback() {
    assert(_txBufferStart == _txBufferEnd);

    if (_txCallback) {
        _txCallback(getRetransmitCounter(), _txUser);
    }

    _txCallback = NULL;
    _txUser     = NULL;
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

int8_t nRF24L01P_ESB::queueTransmission(uint8_t bytes[], size_t numBytes, txCallback_t callback,
                                        void *user) {
    //if (_txCallback != NULL) return (-1);

    _txBuffer      = bytes;
    _txBufferStart = 0;
    _txBufferEnd   = numBytes;
    _txCallback    = callback;
    _txUser        = user;

    return (0);
}

int8_t nRF24L01P_ESB::startListening(uint8_t pipe, rxCallback_t callback, void *user) {
    assert(isPipeIndexValid(pipe));

    if (_rxCallback[pipe] != NULL) return (-1);

    _rxCallback[pipe] = callback;
    _rxUser[pipe]     = user;

    enableDataPipe(pipe);

    return (0);
}

int8_t nRF24L01P_ESB::stopListening(uint8_t pipe) {
    assert(isPipeIndexValid(pipe));

    disableDataPipe(pipe);

    _rxCallback[pipe] = NULL;
    _rxUser[pipe]     = NULL;

    return (0);
}

// ----- helper functions -----------------------------------------------------

uint8_t nRF24L01P_ESB::readShortRegister(Register_t reg) {
    uint8_t result;
    cmd_R_REGISTER(reg, &result, 1);
    return (result);
}

void nRF24L01P_ESB::writeShortRegister(Register_t reg, uint8_t val) {
    cmd_W_REGISTER(reg, &val, 1);
}

void nRF24L01P_ESB::clearSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    clearBit_eq(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

void nRF24L01P_ESB::setSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    setBit_eq(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

// ----- getters and setters --------------------------------------------------

void nRF24L01P_ESB::enableDataPipe(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));
    setSingleBit(Register_EN_RXADDR, pipeIndex);
}

void nRF24L01P_ESB::disableDataPipe(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));
    clearSingleBit(Register_EN_RXADDR, pipeIndex);
}

void nRF24L01P_ESB::enableDynamicPayloadLength(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));
    setSingleBit(Register_DYNPD, pipeIndex);
}

void nRF24L01P_ESB::disableDynamicPayloadLength(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));
    clearSingleBit(Register_DYNPD, pipeIndex);
}

CRCConfig_t nRF24L01P_ESB::getCrcConfig() {
    uint8_t config = readShortRegister(Register_CONFIG);

    if (not readBit<uint8_t>(config, CONFIG_EN_CRC)) {
        return (CRCConfig_DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (CrcConfig_2Bytes);
    } else {
        return (CRCConfig_1Byte);
    }
}

void nRF24L01P_ESB::setCrcConfig(CRCConfig_t crcConfig) {
    uint8_t config = readShortRegister(Register_CONFIG);

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

    writeShortRegister(Register_CONFIG, config);

    assert(crcConfig == getCrcConfig());
}

int8_t nRF24L01P_ESB::getChannel() {
    int8_t channel = readShortRegister(Register_RF_CH);

    return (channel);
}

void nRF24L01P_ESB::setChannel(int8_t channel) {
    if (channel < 0) {
        writeShortRegister(Register_RF_CH, channel);
    }

    assert(channel == getChannel());
}

DataRate_t nRF24L01P_ESB::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_RF_SETUP);

    if (readBit<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW)) {
        return (DataRate_250KBPS);
    }

    if (readBit<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH)) {
        return (DataRate_2MBPS);
    }

    return (DataRate_1MBPS);
}

void nRF24L01P_ESB::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(Register_RF_SETUP);

    switch (dataRate) {
        case (DataRate_1MBPS): {
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_2MBPS: {
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            setBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_250KBPS: {
            setBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
    }

    writeShortRegister(Register_RF_SETUP, rfSetup);

    assert(dataRate == getDataRate());
}

OutputPower_t nRF24L01P_ESB::getOutputPower() {
    uint8_t rf_setup = readShortRegister(Register_RF_SETUP);

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    // Make sure that the following cast is save;
    assert(rf_setup < 4);

    return (static_cast<OutputPower_t>(rf_setup));
}

void nRF24L01P_ESB::setOutputPower(OutputPower_t outputPower) {
    uint8_t rf_setup = readShortRegister(Register_RF_SETUP);

    AND_eq(rf_setup, INVERT<uint8_t>(RF_SETUP_RF_PWR_MASK));
    OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(outputPower, RF_SETUP_RF_PWR));

    writeShortRegister(Register_RF_SETUP, rf_setup);

    assert(outputPower == getOutputPower());
}

int8_t nRF24L01P_ESB::getRetryCount() {
    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

void nRF24L01P_ESB::setRetryCount(uint8_t count) {
    assert(count <= 0xF);

    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(count, SETUP_RETR_ARC));
    writeShortRegister(Register_SETUP_RETR, setup_retr);

    assert(count == getRetryCount());
}

int8_t nRF24L01P_ESB::getRetryDelay() {
    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

void nRF24L01P_ESB::setRetryDelay(uint8_t delay) {
    assert(delay <= 0xF);

    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(delay, SETUP_RETR_ARD));
    writeShortRegister(Register_SETUP_RETR, setup_retr);

    assert(delay == getRetryDelay());
}

int64_t nRF24L01P_ESB::getRxAddress(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));

    addressUnion_t rxAddressUnion;
    rxAddressUnion.m64 = 0;

    switch (pipeIndex) {
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

void nRF24L01P_ESB::setRxAddress(uint8_t pipeIndex, int64_t rxAddress) {
    assert(isPipeIndexValid(pipeIndex));

    addressUnion_t rxAddressUnion;
    rxAddressUnion.m64 = rxAddress;

    switch (pipeIndex) {
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

    assert(rxAddress == getRxAddress(pipeIndex));
}

int64_t nRF24L01P_ESB::getTxAddress() {
    addressUnion_t txAddressUnion;
    txAddressUnion.m64 = 0;

    cmd_R_REGISTER(Register_TX_ADDR, txAddressUnion.m8, TX_ADDR_LENGTH);

    return (txAddressUnion.m64);
}

void nRF24L01P_ESB::setTxAddress(int64_t txAddress) {
    addressUnion_t txAddressUnion;
    txAddressUnion.m64 = txAddress;

    cmd_W_REGISTER(Register_TX_ADDR, txAddressUnion.m8, TX_ADDR_LENGTH);

    assert(txAddress == getTxAddress());
}

int8_t nRF24L01P_ESB::getPackageLossCounter() {
    uint8_t observe_tx = readShortRegister(Register_OBSERVE_TX);

    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT);

    return (observe_tx);
}

int8_t nRF24L01P_ESB::getRetransmitCounter() {
    uint8_t observe_tx = readShortRegister(Register_OBSERVE_TX);

    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT);

    return (observe_tx);
}

} /* namespace xXx */
