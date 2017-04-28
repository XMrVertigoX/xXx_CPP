#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_esb.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define __MAX(value, limit) (value > limit ? limit : value)
#define __MIN(value, limit) (value > limit ? limit : value)

#define __BOUNCE(expression, statement) \
    if (!(expression)) return (statement)

namespace xXx {

static inline uint8_t extractPipe(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

RF24_ESB::RF24_ESB(ISpi &spi, IGpio &ce, IGpio &irq) : nRF24L01P_BASE(spi), ce(ce), irq(irq) {}

RF24_ESB::~RF24_ESB() {
    enterShutdownMode();
}

void RF24_ESB::setup() {
    LOG("%p: %s", this, __PRETTY_FUNCTION__);

    auto interruptFunction = [](void *user) {
        RF24_ESB *self = static_cast<RF24_ESB *>(user);
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

    irq.enableInterrupt(interruptFunction, this);
}

void RF24_ESB::loop() {
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

uint8_t RF24_ESB::readRxFifo(uint8_t status) {
    RF24_Package_t package;

    uint8_t pipe = extractPipe(status);

    if (pipe > 5) {
        return (RF24_Failure);
    }

    cmd_R_RX_PL_WID(package.numBytes);

    if (package.numBytes > rxFifoSize) {
        cmd_FLUSH_RX();
        return (RF24_Failure);
    }

    cmd_R_RX_PAYLOAD(package.bytes, package.numBytes);

    if (this->rxQueue[pipe] != NULL) {
        this->rxQueue[pipe]->enqueue(package);  // TODO: Timeout?
    }

    return (RF24_Success);
}

uint8_t RF24_ESB::writeTxFifo(uint8_t status) {
    if (readBit<uint8_t>(status, STATUS_TX_FULL)) {
        return (RF24_Failure);
    }

    uint8_t numBytes = __MAX(_txBufferEnd - _txBufferStart, txFifoSize);

    cmd_W_TX_PAYLOAD(&_txBuffer[_txBufferStart], numBytes);

    _txBufferStart += numBytes;

    return (RF24_Success);
}

void RF24_ESB::handle_MAX_RT(uint8_t status) {
    cmd_FLUSH_TX();

    // TODO
    txCallback();

    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void RF24_ESB::handle_RX_DR(uint8_t status) {
    readRxFifo(status);

    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void RF24_ESB::handle_TX_DS(uint8_t status) {
    if (_txBufferEnd > _txBufferStart) {
        writeTxFifo(status);
    } else {
        txCallback();
    }

    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void RF24_ESB::txCallback() {
    assert(_txBufferStart == _txBufferEnd);

    if (_txCallback) {
        _txCallback(getRetransmissionCounter(), _txUser);
    }

    _txCallback = NULL;
    _txUser     = NULL;
}

void RF24_ESB::enterRxMode() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    setBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    ce.set();

    delayUs(rxSettling);
}

void RF24_ESB::enterShutdownMode() {
    uint8_t config;

    ce.clear();

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    clearBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));
}

void RF24_ESB::enterStandbyMode() {
    uint8_t config;

    ce.clear();

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));
}

void RF24_ESB::enterTxMode() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    clearBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    ce.set();

    delayUs(txSettling);
}

uint8_t RF24_ESB::queuePackage(uint8_t bytes[], size_t numBytes, txCallback_t callback,
                               void *user) {
    if (_txBufferStart < _txBufferEnd) return (1);

    _txBuffer      = bytes;
    _txBufferStart = 0;
    _txBufferEnd   = numBytes;
    _txCallback    = callback;
    _txUser        = user;

    enableDataPipe(0);

    return (0);
}

uint8_t RF24_ESB::queuePackage2(RF24_Package_t package) {
    uint8_t status = cmd_NOP();

    if (readBit<uint8_t>(status, STATUS_TX_FULL)) return (1);

    cmd_W_TX_PAYLOAD(package.bytes, package.numBytes);

    return (0);
}

RF24_Status_t RF24_ESB::startListening(uint8_t pipe, Queue<RF24_Package_t> &rxQueue) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    this->rxQueue[pipe] = &rxQueue;
    enableDataPipe(pipe);

    return (RF24_Success);
}

RF24_Status_t RF24_ESB::stopListening(uint8_t pipe) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    disableDataPipe(pipe);
    this->rxQueue[pipe] = NULL;

    return (RF24_Success);
}

// ----- getters and setters --------------------------------------------------

uint8_t RF24_ESB::getPackageLossCounter() {
    uint8_t observe_tx;

    cmd_R_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT);
    cmd_W_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));

    return (observe_tx);
}

uint8_t RF24_ESB::getRetransmissionCounter() {
    uint8_t observe_tx;

    cmd_R_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT);
    cmd_W_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));

    return (observe_tx);
}

RF24_Status_t RF24_ESB::enableDataPipe(uint8_t pipe) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    uint8_t en_rxaddr;

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    setBit_eq<uint8_t>(en_rxaddr, pipe);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    // TODO: __MAKE_SURE();

    return (RF24_Success);
}

RF24_Status_t RF24_ESB::disableDataPipe(uint8_t pipe) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    uint8_t en_rxaddr;

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    clearBit_eq<uint8_t>(en_rxaddr, pipe);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    // TODO: __MAKE_SURE();

    return (RF24_Success);
}

RF24_Status_t RF24_ESB::enableDynamicPayloadLength(uint8_t pipe) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    uint8_t dynpd;

    cmd_R_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));
    setBit_eq<uint8_t>(dynpd, pipe);
    cmd_W_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));

    // TODO: __MAKE_SURE();

    return (RF24_Success);
}

RF24_Status_t RF24_ESB::disableDynamicPayloadLength(uint8_t pipe) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    uint8_t dynpd;

    cmd_R_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));
    clearBit_eq<uint8_t>(dynpd, pipe);
    cmd_W_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));

    // TODO: __MAKE_SURE();

    return (RF24_Success);
}

RF24_CRCConfig_t RF24_ESB::getCrcConfig() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));

    if (readBit<uint8_t>(config, CONFIG_EN_CRC) == false) {
        return (RF24_CRCConfig_DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (RF24_CrcConfig_2Bytes);
    } else {
        return (RF24_CRCConfig_1Byte);
    }
}

RF24_Status_t RF24_ESB::setCrcConfig(RF24_CRCConfig_t crcConfig) {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));

    switch (crcConfig) {
        case RF24_CRCConfig_DISABLED: {
            clearBit_eq<uint8_t>(config, CONFIG_EN_CRC);
        } break;
        case RF24_CRCConfig_1Byte: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            clearBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
        case RF24_CrcConfig_2Bytes: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            setBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
    }

    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    __BOUNCE(crcConfig == getCrcConfig(), RF24_Failure);

    return (RF24_Success);
}

uint8_t RF24_ESB::getChannel() {
    uint8_t channel;

    cmd_R_REGISTER(Register_RF_CH, &channel, sizeof(channel));

    return (channel);
}

RF24_Status_t RF24_ESB::setChannel(uint8_t channel) {
    __BOUNCE(channel < 0x80, RF24_UnknownChannelIndex);

    cmd_W_REGISTER(Register_RF_CH, &channel, sizeof(channel));

    __BOUNCE(channel == getChannel(), RF24_Failure);

    return (RF24_Success);
}

RF24_DataRate_t RF24_ESB::getDataRate() {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW)) {
        return (RF24_DataRate_250KBPS);
    }

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH)) {
        return (RF24_DataRate_2MBPS);
    }

    return (RF24_DataRate_1MBPS);
}

RF24_Status_t RF24_ESB::setDataRate(RF24_DataRate_t dataRate) {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    switch (dataRate) {
        case (RF24_DataRate_1MBPS): {
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
        case RF24_DataRate_2MBPS: {
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            setBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
        case RF24_DataRate_250KBPS: {
            setBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
    }

    cmd_W_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    __BOUNCE(dataRate == getDataRate(), RF24_Failure);

    return (RF24_Success);
}

RF24_OutputPower_t RF24_ESB::getOutputPower() {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));
    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    // TODO: Make sure that the following cast is save;
    assert(rf_setup < 4);

    return (static_cast<RF24_OutputPower_t>(rf_setup));
}

RF24_Status_t RF24_ESB::setOutputPower(RF24_OutputPower_t outputPower) {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));
    AND_eq(rf_setup, INVERT<uint8_t>(RF_SETUP_RF_PWR_MASK));
    OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(outputPower, RF_SETUP_RF_PWR));
    cmd_W_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    __BOUNCE(outputPower == getOutputPower(), RF24_Failure);

    return (RF24_Success);
}

uint8_t RF24_ESB::getRetryCount() {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));
    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

RF24_Status_t RF24_ESB::setRetryCount(uint8_t count) {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(__MAX(count, 0xF), SETUP_RETR_ARC));
    cmd_W_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    __BOUNCE(count == getRetryCount(), RF24_Failure);

    return (RF24_Success);
}

uint8_t RF24_ESB::getRetryDelay() {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));
    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

RF24_Status_t RF24_ESB::setRetryDelay(uint8_t delay) {
    uint8_t setup_retr;

    cmd_R_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(__MAX(delay, 0xF), SETUP_RETR_ARD));
    cmd_W_REGISTER(Register_SETUP_RETR, &setup_retr, sizeof(setup_retr));

    __BOUNCE(delay == getRetryDelay(), RF24_Failure);

    return (RF24_Success);
}

uint32_t RF24_ESB::getTxBaseAddress() {
    uint8_t address[TX_ADDR_LENGTH];

    cmd_R_REGISTER(Register_TX_ADDR, address, TX_ADDR_LENGTH);

    return (*reinterpret_cast<uint32_t *>(&address[1]));
}

RF24_Status_t RF24_ESB::setTxBaseAddress(uint32_t baseAddress) {
    uint8_t address[TX_ADDR_LENGTH];

    cmd_R_REGISTER(Register_TX_ADDR, address, TX_ADDR_LENGTH);
    memcpy(&address[1], &baseAddress, sizeof(baseAddress));
    cmd_W_REGISTER(Register_TX_ADDR, address, TX_ADDR_LENGTH);

    __BOUNCE(baseAddress == getTxBaseAddress(), RF24_Failure);

    setRxBaseAddress_0(baseAddress);

    return (RF24_Success);
}

uint8_t RF24_ESB::getTxAddress() {
    uint8_t addressPrefix;

    cmd_R_REGISTER(Register_TX_ADDR, &addressPrefix, 1);

    return (addressPrefix);
}

RF24_Status_t RF24_ESB::setTxAddress(uint8_t addressPrefix) {
    cmd_W_REGISTER(Register_TX_ADDR, &addressPrefix, 1);

    setRxAddress(0, addressPrefix);

    __BOUNCE(addressPrefix == getTxAddress(), RF24_Failure);

    return (RF24_Success);
}

uint32_t RF24_ESB::getRxBaseAddress_0() {
    uint8_t address[RX_ADDR_P0_LENGTH];

    cmd_R_REGISTER(Register_RX_ADDR_P0, address, RX_ADDR_P0_LENGTH);

    return (*reinterpret_cast<uint32_t *>(&address[1]));
}

RF24_Status_t RF24_ESB::setRxBaseAddress_0(uint32_t baseAddress) {
    uint8_t address[RX_ADDR_P0_LENGTH];

    cmd_R_REGISTER(Register_RX_ADDR_P0, address, RX_ADDR_P0_LENGTH);
    memcpy(&address[1], &baseAddress, sizeof(baseAddress));
    cmd_W_REGISTER(Register_RX_ADDR_P0, address, RX_ADDR_P0_LENGTH);

    __BOUNCE(baseAddress == getRxBaseAddress_0(), RF24_Failure);

    return (RF24_Success);
}

uint32_t RF24_ESB::getRxBaseAddress_1() {
    uint8_t address[RX_ADDR_P0_LENGTH];

    cmd_R_REGISTER(Register_RX_ADDR_P1, address, RX_ADDR_P0_LENGTH);

    return (*reinterpret_cast<uint32_t *>(&address[1]));
}

RF24_Status_t RF24_ESB::setRxBaseAddress_1(uint32_t baseAddress) {
    uint8_t address[RX_ADDR_P1_LENGTH];

    cmd_R_REGISTER(Register_RX_ADDR_P1, address, RX_ADDR_P1_LENGTH);
    memcpy(&address[1], &baseAddress, sizeof(baseAddress));
    cmd_W_REGISTER(Register_RX_ADDR_P1, address, RX_ADDR_P1_LENGTH);

    __BOUNCE(baseAddress == getRxBaseAddress_1(), RF24_Failure);

    return (RF24_Success);
}

uint8_t RF24_ESB::getRxAddress(uint8_t pipe) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    uint8_t address;

    switch (pipe) {
        case 0: {
            cmd_R_REGISTER(Register_RX_ADDR_P0, &address, 1);
        } break;
        case 1: {
            cmd_R_REGISTER(Register_RX_ADDR_P1, &address, 1);
        } break;
        case 2: {
            cmd_R_REGISTER(Register_RX_ADDR_P2, &address, 1);
        } break;
        case 3: {
            cmd_R_REGISTER(Register_RX_ADDR_P3, &address, 1);
        } break;
        case 4: {
            cmd_R_REGISTER(Register_RX_ADDR_P4, &address, 1);
        } break;
        case 5: {
            cmd_R_REGISTER(Register_RX_ADDR_P5, &address, 1);
        } break;
    }

    return (address);
}

RF24_Status_t RF24_ESB::setRxAddress(uint8_t pipe, uint8_t address) {
    __BOUNCE(pipe < 6, RF24_UnknownPipeIndex);

    enableDynamicPayloadLength(pipe);

    switch (pipe) {
        case 0: {
            cmd_W_REGISTER(Register_RX_ADDR_P0, &address, 1);
        } break;
        case 1: {
            cmd_W_REGISTER(Register_RX_ADDR_P1, &address, 1);
        } break;
        case 2: {
            cmd_W_REGISTER(Register_RX_ADDR_P2, &address, 1);
        } break;
        case 3: {
            cmd_W_REGISTER(Register_RX_ADDR_P3, &address, 1);
        } break;
        case 4: {
            cmd_W_REGISTER(Register_RX_ADDR_P4, &address, 1);
        } break;
        case 5: {
            cmd_W_REGISTER(Register_RX_ADDR_P5, &address, 1);
        } break;
    }

    __BOUNCE(address == getRxAddress(pipe), RF24_Failure);

    return (RF24_Success);
}

} /* namespace xXx */
