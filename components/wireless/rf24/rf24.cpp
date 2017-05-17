#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xXx/components/wireless/rf24/rf24.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define __LIMIT(value, limit) (value > limit ? limit : value)

#define __BOUNCE(expression, statement) \
    if (expression) return (statement)

#define __READ_REGISTER(reg, val) R_REGISTER(reg, &val, sizeof(val))
#define __READ_REGISTER_VARIO(reg, val, len) R_REGISTER(reg, &val, len)

#define __WRITE_REGISTER(reg, val) W_REGISTER(reg, &val, sizeof(val))
#define __WRITE_REGISTER_VARIO(reg, val, len) W_REGISTER(reg, &val, len)

namespace xXx {

static inline uint8_t extractPipe(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

RF24::RF24(ISpi& spi, IGpio& ce, IGpio& irq) : RF24_BASE(spi), ce(ce), irq(irq) {}

RF24::~RF24() {}

RF24_Status RF24::notify() {
    __BOUNCE(notificationCounter == __UINT8_MAX__, RF24_Status::Failure);

    notificationCounter++;

    return (RF24_Status::Success);
}

void RF24::setup() {
    uint8_t tmp;

    // Enable dynamic payload length only
    __READ_REGISTER(RF24_Register::FEATURE, tmp);
    clearBit_eq<uint8_t>(tmp, FEATURE_EN_DYN_ACK);
    clearBit_eq<uint8_t>(tmp, FEATURE_EN_ACK_PAY);
    setBit_eq<uint8_t>(tmp, FEATURE_EN_DPL);
    __WRITE_REGISTER(RF24_Register::FEATURE, tmp);

    // Clear interrupts
    __READ_REGISTER(RF24_Register::STATUS, tmp);
    setBit_eq<uint8_t>(tmp, STATUS_MAX_RT);
    setBit_eq<uint8_t>(tmp, STATUS_RX_DR);
    setBit_eq<uint8_t>(tmp, STATUS_TX_DS);
    __WRITE_REGISTER(RF24_Register::STATUS, tmp);

    FLUSH_TX();
    FLUSH_RX();

    IGpio_Callback_t interruptFunction = [](void* user) {
        RF24* self = static_cast<RF24*>(user);
        self->notify();
    };

    irq.enableInterrupt(interruptFunction, this);
}

void RF24::loop() {
    uint8_t status;

    // TODO: Callback logic here

    if (notificationCounter == 0) return;

    __READ_REGISTER(RF24_Register::STATUS, status);
    if (readBit<uint8_t>(status, STATUS_MAX_RT)) handle_MAX_RT(status);
    if (readBit<uint8_t>(status, STATUS_TX_DS)) handle_TX_DS(status);
    if (readBit<uint8_t>(status, STATUS_RX_DR)) handle_RX_DR(status);
    __WRITE_REGISTER(RF24_Register::STATUS, status);

    notificationCounter--;
}

void RF24::handle_MAX_RT(uint8_t status) {
    (void)status;

    FLUSH_TX();
}

void RF24::handle_TX_DS(uint8_t status) {
    (void)status;
}

void RF24::handle_RX_DR(uint8_t status) {
    RF24_Status error = readRxFifo(status);
    if (error != RF24_Status::Success) FLUSH_RX();
}

RF24_Status RF24::readRxFifo(uint8_t status) {
    RF24_DataPackage_t package;

    uint8_t pipe = extractPipe(status);
    __BOUNCE(pipe > 5, RF24_Status::Failure);

    R_RX_PL_WID(package.numBytes);
    __BOUNCE(package.numBytes > rxFifoSize, RF24_Status::Failure);

    R_RX_PAYLOAD(package.bytes, package.numBytes);
    __BOUNCE(this->rxQueue[pipe] == NULL, RF24_Status::Failure);

    this->rxQueue[pipe]->enqueue(package);

    return (RF24_Status::Success);
}

RF24_Status RF24::writeTxFifo(uint8_t status) {
    RF24_DataPackage_t package;

    __BOUNCE(readBit<uint8_t>(status, STATUS_TX_FULL), RF24_Status::Failure);

    this->txQueue->dequeue(package);

    W_TX_PAYLOAD(package.bytes, package.numBytes);

    return (RF24_Status::Success);
}

void RF24::enterRxMode() {
    uint8_t config;

    __READ_REGISTER(RF24_Register::CONFIG, config);
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    setBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    __WRITE_REGISTER(RF24_Register::CONFIG, config);

    ce.set();

    delayUs(rxSettling);
}

void RF24::enterShutdownMode() {
    uint8_t config;

    ce.clear();

    __READ_REGISTER(RF24_Register::CONFIG, config);
    clearBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    __WRITE_REGISTER(RF24_Register::CONFIG, config);
}

void RF24::enterStandbyMode() {
    uint8_t config;

    ce.clear();

    __READ_REGISTER(RF24_Register::CONFIG, config);
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    __WRITE_REGISTER(RF24_Register::CONFIG, config);
}

void RF24::enterTxMode() {
    uint8_t config;

    __READ_REGISTER(RF24_Register::CONFIG, config);
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    clearBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    __WRITE_REGISTER(RF24_Register::CONFIG, config);

    ce.set();

    delayUs(txSettling);
}

uint8_t RF24::getPackageLossCounter() {
    uint8_t observe_tx;

    __READ_REGISTER(RF24_Register::OBSERVE_TX, observe_tx);
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT);

    return (observe_tx);
}

uint8_t RF24::getRetransmissionCounter() {
    uint8_t observe_tx;

    __READ_REGISTER(RF24_Register::OBSERVE_TX, observe_tx);
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT);

    return (observe_tx);
}

RF24_Status RF24::configureRxDataPipe(uint8_t pipe, Queue<RF24_DataPackage_t>* rxQueue) {
    uint8_t en_rxaddr;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    __READ_REGISTER(RF24_Register::EN_RXADDR, en_rxaddr);
    setBit_eq<uint8_t>(en_rxaddr, pipe);
    __WRITE_REGISTER(RF24_Register::EN_RXADDR, en_rxaddr);

    // TODO: Verify

    enableDynamicPayloadLength(pipe);

    this->rxQueue[pipe] = rxQueue;

    return (RF24_Status::Success);
}

RF24_Status RF24::disableRxDataPipe(uint8_t pipe) {
    uint8_t en_rxaddr;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    __READ_REGISTER(RF24_Register::EN_RXADDR, en_rxaddr);
    clearBit_eq<uint8_t>(en_rxaddr, pipe);
    __WRITE_REGISTER(RF24_Register::EN_RXADDR, en_rxaddr);

    // TODO: Verify

    disableDynamicPayloadLength(pipe);

    this->rxQueue[pipe] = NULL;

    return (RF24_Status::Success);
}

RF24_Status RF24::configureTxDataPipe(Queue<RF24_DataPackage_t>* txQueue) {
    this->txQueue = txQueue;

    return (configureRxDataPipe(0, NULL));
}

RF24_Status RF24::disableTxDataPipe() {
    this->txQueue = NULL;

    return (disableRxDataPipe(0));
}

RF24_Status RF24::enableDynamicPayloadLength(uint8_t pipe) {
    uint8_t dynpd;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    __READ_REGISTER(RF24_Register::DYNPD, dynpd);
    setBit_eq<uint8_t>(dynpd, pipe);
    __WRITE_REGISTER(RF24_Register::DYNPD, dynpd);

    // TODO: Verify

    return (RF24_Status::Success);
}

RF24_Status RF24::disableDynamicPayloadLength(uint8_t pipe) {
    uint8_t dynpd;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    __READ_REGISTER(RF24_Register::DYNPD, dynpd);
    clearBit_eq<uint8_t>(dynpd, pipe);
    __WRITE_REGISTER(RF24_Register::DYNPD, dynpd);

    // TODO: Verify

    return (RF24_Status::Success);
}

RF24_CRCConfig RF24::getCrcConfig() {
    uint8_t config;

    __READ_REGISTER(RF24_Register::CONFIG, config);

    if (readBit<uint8_t>(config, CONFIG_EN_CRC) == false) {
        return (RF24_CRCConfig::CRC_DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (RF24_CRCConfig::CRC_2Bytes);
    } else {
        return (RF24_CRCConfig::CRC_1Byte);
    }
}

RF24_Status RF24::setCrcConfig(RF24_CRCConfig crcConfig) {
    uint8_t config;

    __READ_REGISTER(RF24_Register::CONFIG, config);

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

    __WRITE_REGISTER(RF24_Register::CONFIG, config);

    __BOUNCE(crcConfig != getCrcConfig(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getChannel() {
    uint8_t channel;

    __READ_REGISTER(RF24_Register::RF_CH, channel);

    __BOUNCE(channel > 127, __UINT8_MAX__);

    return (channel);
}

RF24_Status RF24::setChannel(uint8_t channel) {
    __BOUNCE(channel > 127, RF24_Status::UnknownChannel);

    __WRITE_REGISTER(RF24_Register::RF_CH, channel);

    __BOUNCE(channel != getChannel(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_DataRate RF24::getDataRate() {
    uint8_t rf_setup;

    __READ_REGISTER(RF24_Register::RF_SETUP, rf_setup);

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_LOW)) {
        return (RF24_DataRate::DR_250KBPS);
    }

    if (readBit<uint8_t>(rf_setup, RF_SETUP_RF_DR_HIGH)) {
        return (RF24_DataRate::DR_2MBPS);
    }

    return (RF24_DataRate::DR_1MBPS);
}

RF24_Status RF24::setDataRate(RF24_DataRate dataRate) {
    uint8_t rf_setup;

    __READ_REGISTER(RF24_Register::RF_SETUP, rf_setup);

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

    __WRITE_REGISTER(RF24_Register::RF_SETUP, rf_setup);

    __BOUNCE(dataRate != getDataRate(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_OutputPower RF24::getOutputPower() {
    uint8_t rf_setup;

    __READ_REGISTER(RF24_Register::RF_SETUP, rf_setup);

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    switch (rf_setup) {
        case 0: return (RF24_OutputPower::PWR_18dBm); break;
        case 1: return (RF24_OutputPower::PWR_12dBm); break;
        case 2: return (RF24_OutputPower::PWR_6dBm); break;
        default: return (RF24_OutputPower::PWR_0dBm); break;
    }
}

RF24_Status RF24::setOutputPower(RF24_OutputPower outputPower) {
    uint8_t rf_setup;

    __READ_REGISTER(RF24_Register::RF_SETUP, rf_setup);

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

    __WRITE_REGISTER(RF24_Register::RF_SETUP, rf_setup);

    __BOUNCE(outputPower != getOutputPower(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getRetryCount() {
    uint8_t setup_retr;

    __READ_REGISTER(RF24_Register::SETUP_RETR, setup_retr);
    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

RF24_Status RF24::setRetryCount(uint8_t count) {
    uint8_t setup_retr;

    __READ_REGISTER(RF24_Register::SETUP_RETR, setup_retr);
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(__LIMIT(count, 0xF), SETUP_RETR_ARC));
    __WRITE_REGISTER(RF24_Register::SETUP_RETR, setup_retr);

    __BOUNCE(count != getRetryCount(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getRetryDelay() {
    uint8_t setup_retr;

    __READ_REGISTER(RF24_Register::SETUP_RETR, setup_retr);
    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

RF24_Status RF24::setRetryDelay(uint8_t delay) {
    uint8_t setup_retr;

    __READ_REGISTER(RF24_Register::SETUP_RETR, setup_retr);
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(__LIMIT(delay, 0xF), SETUP_RETR_ARD));
    __WRITE_REGISTER(RF24_Register::SETUP_RETR, setup_retr);

    __BOUNCE(delay != getRetryDelay(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_Address_t RF24::getTxAddress() {
    RF24_Address_t address = 0;

    __READ_REGISTER_VARIO(RF24_Register::TX_ADDR, address, TX_ADDR_LENGTH);

    return (address);
}

RF24_Status RF24::setTxAddress(RF24_Address_t address) {
    __BOUNCE(address > 0xFFFFFFFFFF, RF24_Status::UnknownAddress);

    __WRITE_REGISTER_VARIO(RF24_Register::TX_ADDR, address, TX_ADDR_LENGTH);

    __BOUNCE(address != getTxAddress(), RF24_Status::VerificationFailed);

    return (setRxAddress(0, address));
}

RF24_Address_t RF24::getRxAddress(uint8_t pipe) {
    RF24_Address_t address = 0;

    __BOUNCE(pipe > 5, __UINT64_MAX__);

    if (pipe == 0) {
        __READ_REGISTER_VARIO(RF24_Register::RX_ADDR_P0, address, RX_ADDR_P0_LENGTH);
    } else {
        __READ_REGISTER_VARIO(RF24_Register::RX_ADDR_P1, address, RX_ADDR_P1_LENGTH);

        switch (pipe) {
            case 2:
                __READ_REGISTER_VARIO(RF24_Register::RX_ADDR_P2, address, RX_ADDR_P2_LENGTH);
                break;
            case 3:
                __READ_REGISTER_VARIO(RF24_Register::RX_ADDR_P3, address, RX_ADDR_P3_LENGTH);
                break;
            case 4:
                __READ_REGISTER_VARIO(RF24_Register::RX_ADDR_P4, address, RX_ADDR_P4_LENGTH);
                break;
            case 5:
                __READ_REGISTER_VARIO(RF24_Register::RX_ADDR_P5, address, RX_ADDR_P5_LENGTH);
                break;
        }
    }

    return (address);
}

RF24_Status RF24::setRxAddress(uint8_t pipe, RF24_Address_t address) {
    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);
    __BOUNCE(address > 0xFFFFFFFFFF, RF24_Status::UnknownAddress);

    switch (pipe) {
        case 0: {
            __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P0, address, RX_ADDR_P0_LENGTH);
        } break;
        case 1: {
            __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P1, address, RX_ADDR_P1_LENGTH);
        } break;
        case 2: {
            __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P2, address, RX_ADDR_P2_LENGTH);
        } break;
        case 3: {
            __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P3, address, RX_ADDR_P3_LENGTH);
        } break;
        case 4: {
            __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P4, address, RX_ADDR_P4_LENGTH);
        } break;
        case 5: {
            __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P5, address, RX_ADDR_P5_LENGTH);
        } break;
    }

    // TODO: Beautify this solution!
    if (pipe > 1) {
        RF24_Address_t address_p1 = getRxAddress(1);
        memcpy(&address, &address_p1, 1);
        __WRITE_REGISTER_VARIO(RF24_Register::RX_ADDR_P1, address, RX_ADDR_P1_LENGTH);
    }

    __BOUNCE(address != getRxAddress(pipe), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_Status RF24::enableAutoAcknowledgment(uint8_t pipe, bool enable) {
    uint8_t en_aa;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    __READ_REGISTER(RF24_Register::EN_AA, en_aa);

    if (enable) {
        setBit_eq<uint8_t>(en_aa, pipe);
    } else {
        clearBit_eq<uint8_t>(en_aa, pipe);
    }

    __WRITE_REGISTER(RF24_Register::EN_AA, en_aa);

    // TODO: Verify

    return (RF24_Status::Success);
}

RF24_Status RF24::enableDataPipe(uint8_t pipe, bool enable) {
    uint8_t en_rxaddr;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    __READ_REGISTER(RF24_Register::EN_RXADDR, en_rxaddr);

    if (enable) {
        setBit_eq<uint8_t>(en_rxaddr, pipe);
    } else {
        clearBit_eq<uint8_t>(en_rxaddr, pipe);
    }

    __WRITE_REGISTER(RF24_Register::EN_RXADDR, en_rxaddr);

    // TODO: Verify

    return (RF24_Status::Success);
}

} /* namespace xXx */
