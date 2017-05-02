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
    if (expression) return (statement)

namespace xXx {

static inline uint8_t extractPipe(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

RF24_ESB::RF24_ESB(ISpi &spi, IGpio &ce, IGpio &irq) : nRF24L01P_BASE(spi), ce(ce), irq(irq) {}

RF24_ESB::~RF24_ESB() {}

void RF24_ESB::setup() {
    LOG("%p: %s", this, __PRETTY_FUNCTION__);

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

    irq.enableInterrupt([](void *user) { static_cast<RF24_ESB *>(user)->notifyFromISR(); }, this);
}

void RF24_ESB::loop() {
    uint8_t status = cmd_NOP();

    if (readBit<uint8_t>(status, STATUS_MAX_RT)) {
        handle_MAX_RT(status);
    }

    if (readBit<uint8_t>(status, STATUS_TX_DS)) {
        handle_TX_DS(status);
    }

    if (readBit<uint8_t>(status, STATUS_RX_DR)) {
        handle_RX_DR(status);
    }

    wait();
}

RF24_Status_t RF24_ESB::readRxFifo(uint8_t status) {
    RF24_Package_t package;

    uint8_t pipe = extractPipe(status);
    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);

    cmd_R_RX_PL_WID(package.numBytes);
    __BOUNCE(package.numBytes > rxFifoSize, RF24_Status_Failure);

    cmd_R_RX_PAYLOAD(package.bytes, package.numBytes);
    __BOUNCE(this->rxQueue[pipe] == NULL, RF24_Status_NoRxQueueSet);

    // TODO: Timeout?
    this->rxQueue[pipe]->enqueue(package);

    return (RF24_Status_Success);
}

RF24_Status_t RF24_ESB::writeTxFifo(uint8_t status) {
    __BOUNCE(readBit<uint8_t>(status, STATUS_TX_FULL), RF24_Status_Failure);

    RF24_Package_t package;
    this->txQueue->dequeue(package);

    cmd_W_TX_PAYLOAD(package.bytes, package.numBytes);

    return (RF24_Status_Success);
}

void RF24_ESB::handle_MAX_RT(uint8_t status) {
    cmd_FLUSH_TX();

    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void RF24_ESB::handle_RX_DR(uint8_t status) {
    if (readRxFifo(status)) {
        cmd_FLUSH_RX();
    }

    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
}

void RF24_ESB::handle_TX_DS(uint8_t status) {
    // FUTURE: If buffer longer than 32 bytes, transfer next chunk from here

    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    cmd_W_REGISTER(Register_STATUS, &status, sizeof(status));
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

// ----- getters and setters --------------------------------------------------

uint8_t RF24_ESB::getPackageLossCounter() {
    uint8_t observe_tx;

    cmd_R_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT);

    return (observe_tx);
}

uint8_t RF24_ESB::getRetransmissionCounter() {
    uint8_t observe_tx;

    cmd_R_REGISTER(Register_OBSERVE_TX, &observe_tx, sizeof(observe_tx));
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT);

    return (observe_tx);
}

RF24_Status_t RF24_ESB::enableRxDataPipe(uint8_t pipe, Queue<RF24_Package_t> &rxQueue) {
    uint8_t en_rxaddr;

    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    setBit_eq<uint8_t>(en_rxaddr, pipe);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    enableDynamicPayloadLength(pipe);

    this->rxQueue[pipe] = &rxQueue;

    // TODO: Verify

    return (RF24_Status_Success);
}

RF24_Status_t RF24_ESB::disableRxDataPipe(uint8_t pipe) {
    uint8_t en_rxaddr;

    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    clearBit_eq<uint8_t>(en_rxaddr, pipe);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    disableDynamicPayloadLength(pipe);

    this->rxQueue[pipe] = NULL;

    // TODO: Verify

    return (RF24_Status_Success);
}

RF24_Status_t RF24_ESB::enableTxDataPipe(Queue<RF24_Package_t> &txQueue) {
    uint8_t en_rxaddr;

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    setBit_eq<uint8_t>(en_rxaddr, 0);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    this->txQueue = &txQueue;

    // TODO: Verify

    return (RF24_Status_Success);
}

RF24_Status_t RF24_ESB::disableTxDataPipe() {
    uint8_t en_rxaddr;

    cmd_R_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));
    clearBit_eq<uint8_t>(en_rxaddr, 0);
    cmd_W_REGISTER(Register_EN_RXADDR, &en_rxaddr, sizeof(en_rxaddr));

    this->txQueue = NULL;

    // TODO: Verify

    return (RF24_Status_Success);
}

RF24_Status_t RF24_ESB::enableDynamicPayloadLength(uint8_t pipe) {
    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);

    uint8_t dynpd;

    cmd_R_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));
    setBit_eq<uint8_t>(dynpd, pipe);
    cmd_W_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));

    // TODO: Verify

    return (RF24_Status_Success);
}

RF24_Status_t RF24_ESB::disableDynamicPayloadLength(uint8_t pipe) {
    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);

    uint8_t dynpd;

    cmd_R_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));
    clearBit_eq<uint8_t>(dynpd, pipe);
    cmd_W_REGISTER(Register_DYNPD, &dynpd, sizeof(dynpd));

    // TODO: Verify

    return (RF24_Status_Success);
}

RF24_CRCConfig RF24_ESB::getCrcConfig() {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));

    if (readBit<uint8_t>(config, CONFIG_EN_CRC) == false) {
        return (RF24_CRCConfig::CRC_DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (RF24_CRCConfig::CRC_2Bytes);
    } else {
        return (RF24_CRCConfig::CRC_1Byte);
    }
}

RF24_Status_t RF24_ESB::setCrcConfig(RF24_CRCConfig crcConfig) {
    uint8_t config;

    cmd_R_REGISTER(Register_CONFIG, &config, sizeof(config));

    switch (crcConfig) {
        case RF24_CRCConfig::CRC_DISABLED: {
            clearBit_eq<uint8_t>(config, CONFIG_EN_CRC);
        } break;
        case RF24_CRCConfig::CRC_1Byte: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            clearBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
        case RF24_CRCConfig::CRC_2Bytes: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            setBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
    }

    cmd_W_REGISTER(Register_CONFIG, &config, sizeof(config));

    __BOUNCE(crcConfig != getCrcConfig(), RF24_Status_VerificationFailed);

    return (RF24_Status_Success);
}

uint8_t RF24_ESB::getChannel() {
    uint8_t channel;

    cmd_R_REGISTER(Register_RF_CH, &channel, sizeof(channel));

    return (channel);
}

RF24_Status_t RF24_ESB::setChannel(uint8_t channel) {
    __BOUNCE(channel > 127, RF24_Status_UnknownChannelIndex);

    cmd_W_REGISTER(Register_RF_CH, &channel, sizeof(channel));

    __BOUNCE(channel != getChannel(), RF24_Status_VerificationFailed);

    return (RF24_Status_Success);
}

RF24_DataRate RF24_ESB::getDataRate() {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW)) {
        return (RF24_DataRate::DR_250KBPS);
    }

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH)) {
        return (RF24_DataRate::DR_2MBPS);
    }

    return (RF24_DataRate::DR_1MBPS);
}

RF24_Status_t RF24_ESB::setDataRate(RF24_DataRate dataRate) {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    switch (dataRate) {
        case RF24_DataRate::DR_1MBPS: {
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
        case RF24_DataRate::DR_2MBPS: {
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            setBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
        case RF24_DataRate::DR_250KBPS: {
            setBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH);
        } break;
    }

    cmd_W_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    __BOUNCE(dataRate != getDataRate(), RF24_Status_VerificationFailed);

    return (RF24_Status_Success);
}

RF24_OutputPower RF24_ESB::getOutputPower() {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    switch (rf_setup) {
        case 0: return (RF24_OutputPower::PWR_18dBm); break;
        case 1: return (RF24_OutputPower::PWR_12dBm); break;
        case 2: return (RF24_OutputPower::PWR_6dBm); break;
        default: return (RF24_OutputPower::PWR_0dBm); break;
    }
}

RF24_Status_t RF24_ESB::setOutputPower(RF24_OutputPower outputPower) {
    uint8_t rf_setup;

    cmd_R_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    // Reset value
    OR_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);

    switch (outputPower) {
        case RF24_OutputPower::PWR_18dBm: {
            clearBit_eq<uint8_t>(rf_setup, 1);
            clearBit_eq<uint8_t>(rf_setup, 2);
        } break;
        case RF24_OutputPower::PWR_12dBm: {
            clearBit_eq<uint8_t>(rf_setup, 2);
        } break;
        case RF24_OutputPower::PWR_6dBm: {
            clearBit_eq<uint8_t>(rf_setup, 1);
        } break;
        case RF24_OutputPower::PWR_0dBm: {
        } break;
    }

    cmd_W_REGISTER(Register_RF_SETUP, &rf_setup, sizeof(rf_setup));

    __BOUNCE(outputPower != getOutputPower(), RF24_Status_VerificationFailed);

    return (RF24_Status_Success);
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

    __BOUNCE(count != getRetryCount(), RF24_Status_VerificationFailed);

    return (RF24_Status_Success);
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

    __BOUNCE(delay != getRetryDelay(), RF24_Status_VerificationFailed);

    return (RF24_Status_Success);
}

RF24_Address_t RF24_ESB::getTxAddress() {
    RF24_Address_t address = 0;

    cmd_R_REGISTER(Register_TX_ADDR, &address, TX_ADDR_LENGTH);

    return (address);
}

RF24_Status_t RF24_ESB::setTxAddress(RF24_Address_t address) {
    __BOUNCE(address > 0xFFFFFFFFFF, RF24_Status_MalformedAddress);
    cmd_W_REGISTER(Register_TX_ADDR, &address, TX_ADDR_LENGTH);

    __BOUNCE(address != getTxAddress(), RF24_Status_TxAddressVerificationFailed);

    return (setRxAddress(0, address));
}

RF24_Address_t RF24_ESB::getRxAddress(uint8_t pipe) {
    RF24_Address_t address = 0;

    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);

    if (pipe == 0) {
        cmd_R_REGISTER(Register_RX_ADDR_P0, &address, RX_ADDR_P0_LENGTH);
    } else {
        cmd_R_REGISTER(Register_RX_ADDR_P1, &address, RX_ADDR_P1_LENGTH);

        switch (pipe) {
            case 2: cmd_R_REGISTER(Register_RX_ADDR_P2, &address, RX_ADDR_P2_LENGTH); break;
            case 3: cmd_R_REGISTER(Register_RX_ADDR_P3, &address, RX_ADDR_P3_LENGTH); break;
            case 4: cmd_R_REGISTER(Register_RX_ADDR_P4, &address, RX_ADDR_P4_LENGTH); break;
            case 5: cmd_R_REGISTER(Register_RX_ADDR_P5, &address, RX_ADDR_P5_LENGTH); break;
        }
    }

    return (address);
}

RF24_Status_t RF24_ESB::setRxAddress(uint8_t pipe, RF24_Address_t address) {
    __BOUNCE(pipe > 5, RF24_Status_UnknownPipeIndex);
    __BOUNCE(address > 0xFFFFFFFFFF, RF24_Status_MalformedAddress);

    switch (pipe) {
        case 0: {
            cmd_W_REGISTER(Register_RX_ADDR_P0, &address, RX_ADDR_P0_LENGTH);
        } break;
        case 1: {
            cmd_W_REGISTER(Register_RX_ADDR_P1, &address, RX_ADDR_P1_LENGTH);
        } break;
        case 2: {
            cmd_W_REGISTER(Register_RX_ADDR_P2, &address, RX_ADDR_P2_LENGTH);
        } break;
        case 3: {
            cmd_W_REGISTER(Register_RX_ADDR_P3, &address, RX_ADDR_P3_LENGTH);
        } break;
        case 4: {
            cmd_W_REGISTER(Register_RX_ADDR_P4, &address, RX_ADDR_P4_LENGTH);
        } break;
        case 5: {
            cmd_W_REGISTER(Register_RX_ADDR_P5, &address, RX_ADDR_P5_LENGTH);
        } break;
    }

    // TODO: Beautify this solution!
    if (pipe > 1) {
        RF24_Address_t address_p1 = getRxAddress(1);
        memcpy(&address, &address_p1, 1);
        cmd_W_REGISTER(Register_RX_ADDR_P1, &address, RX_ADDR_P1_LENGTH);
    }

    __BOUNCE(address != getRxAddress(pipe), RF24_Status_RxAddressVerificationFailed);

    return (RF24_Status_Success);
}

} /* namespace xXx */
