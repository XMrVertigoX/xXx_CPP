#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <xXx/components/wireless/rf24/rf24.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define __BOUNCE(expression, statement) \
    if (expression) return (statement)

static const uint8_t addressPrefixLength = 1;
static const uint8_t baseAddressOffset   = 1;

namespace xXx {

static inline uint8_t extractPipe(uint8_t status) {
    AND_eq<uint8_t>(status, STATUS_RX_P_NO_MASK);
    RIGHT_eq<uint8_t>(status, STATUS_RX_P_NO);

    return (status);
}

RF24::RF24(ISpi &spi, IGpio &ce, IGpio &irq)
    : RF24_BASE(spi), ce(ce), irq(irq), rxBuffer(CircularBuffer<RF24_DataPackage_t>(6)) {}

RF24::~RF24() {}

void RF24::setup() {
    uint8_t tmp;

    IGpio_Callback_t interruptFunction = [](void *user) {
        RF24 *self = static_cast<RF24 *>(user);
        self->increaseNotificationCounter();
    };

    // Enable dynamic payload length only
    R_REGISTER(RF24_Register::FEATURE, &tmp);
    clearBit_eq<uint8_t>(tmp, FEATURE_EN_DYN_ACK);
    clearBit_eq<uint8_t>(tmp, FEATURE_EN_ACK_PAY);
    setBit_eq<uint8_t>(tmp, FEATURE_EN_DPL);
    W_REGISTER(RF24_Register::FEATURE, &tmp);

    // Clear interrupts
    R_REGISTER(RF24_Register::STATUS, &tmp);
    setBit_eq<uint8_t>(tmp, STATUS_MAX_RT);
    setBit_eq<uint8_t>(tmp, STATUS_RX_DR);
    setBit_eq<uint8_t>(tmp, STATUS_TX_DS);
    W_REGISTER(RF24_Register::STATUS, &tmp);

    FLUSH_TX();
    FLUSH_RX();

    irq.enableInterrupt(interruptFunction, this);
}

void RF24::loop() {
    uint8_t status;

    if (rxBuffer.itemsAvailable()) {
        RF24_DataPackage_t package;
        rxBuffer.pop(package);

        if (rxCallback[package.pipe]) {
            rxCallback[package.pipe](package, rxUser[package.pipe]);
        }
    }

    if (!decreaseNotificationCounter()) return;

    R_REGISTER(RF24_Register::STATUS, &status);
    if (readBit<uint8_t>(status, STATUS_MAX_RT)) handle_MAX_RT(status);
    if (readBit<uint8_t>(status, STATUS_TX_DS)) handle_TX_DS(status);
    if (readBit<uint8_t>(status, STATUS_RX_DR)) handle_RX_DR(status);
    W_REGISTER(RF24_Register::STATUS, &status);
}

bool RF24::increaseNotificationCounter() {
    __BOUNCE(notificationCounter == __UINT8_MAX__, false);

    notificationCounter++;

    return (true);
}

bool RF24::decreaseNotificationCounter() {
    __BOUNCE(notificationCounter == 0, false);

    notificationCounter--;

    return (true);
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
    bool success;

    package.pipe = extractPipe(status);
    __BOUNCE(package.pipe > 5, RF24_Status::Failure);

    R_RX_PL_WID(package.numBytes);
    __BOUNCE(package.numBytes > rxFifoSize, RF24_Status::Failure);

    R_RX_PAYLOAD(package.bytes, package.numBytes);

    success = rxBuffer.push(package);
    __BOUNCE(!success, RF24_Status::Failure);

    return (RF24_Status::Success);
}

RF24_Status RF24::writeTxFifo(uint8_t status) {
    //    RF24_DataPackage_t package;

    __BOUNCE(readBit<uint8_t>(status, STATUS_TX_FULL), RF24_Status::Failure);

    // TODO: this->txQueue->dequeue(package);

    //    W_TX_PAYLOAD(package.bytes, package.numBytes);

    return (RF24_Status::Success);
}

void RF24::enterRxMode() {
    uint8_t config;

    R_REGISTER(RF24_Register::CONFIG, &config);
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    setBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    W_REGISTER(RF24_Register::CONFIG, &config);

    ce.set();

    delayUs(rxSettling);
}

void RF24::enterShutdownMode() {
    uint8_t config;

    ce.clear();

    R_REGISTER(RF24_Register::CONFIG, &config);
    clearBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    W_REGISTER(RF24_Register::CONFIG, &config);
}

void RF24::enterStandbyMode() {
    uint8_t config;

    ce.clear();

    R_REGISTER(RF24_Register::CONFIG, &config);
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    W_REGISTER(RF24_Register::CONFIG, &config);
}

void RF24::enterTxMode() {
    uint8_t config;

    R_REGISTER(RF24_Register::CONFIG, &config);
    setBit_eq<uint8_t>(config, CONFIG_PWR_UP);
    clearBit_eq<uint8_t>(config, CONFIG_PRIM_RX);
    W_REGISTER(RF24_Register::CONFIG, &config);

    ce.set();

    delayUs(txSettling);
}

RF24_Status RF24::startListening(uint8_t pipe, RF24_RxCallback_t callback, void *user) {
    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    rxCallback[pipe] = callback;
    rxUser[pipe]     = user;

    enableDynamicPayloadLength(pipe);
    enableAutoAcknowledgment(pipe);
    enableDataPipe(pipe);

    return (RF24_Status::Success);
}

RF24_Status RF24::stopListening(uint8_t pipe) {
    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    rxCallback[pipe] = NULL;
    rxUser[pipe]     = NULL;

    enableDynamicPayloadLength(pipe, false);
    enableAutoAcknowledgment(pipe, false);
    enableDataPipe(pipe, false);

    return (RF24_Status::Success);
}

uint8_t RF24::getPackageLossCounter() {
    uint8_t observe_tx;

    R_REGISTER(RF24_Register::OBSERVE_TX, &observe_tx);
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_PLOS_CNT);

    return (observe_tx);
}

uint8_t RF24::getRetransmissionCounter() {
    uint8_t observe_tx;

    R_REGISTER(RF24_Register::OBSERVE_TX, &observe_tx);
    AND_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT_MASK);
    RIGHT_eq<uint8_t>(observe_tx, OBSERVE_TX_ARC_CNT);

    return (observe_tx);
}

RF24_Status RF24::enableDynamicPayloadLength(uint8_t pipe, bool enable) {
    uint8_t dynpd;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    R_REGISTER(RF24_Register::DYNPD, &dynpd);

    if (enable) {
        setBit_eq<uint8_t>(dynpd, pipe);
    } else {
        clearBit_eq<uint8_t>(dynpd, pipe);
    }

    W_REGISTER(RF24_Register::DYNPD, &dynpd);

    // TODO: Verify

    return (RF24_Status::Success);
}

RF24_CRCConfig RF24::getCrcConfig() {
    uint8_t config;

    R_REGISTER(RF24_Register::CONFIG, &config);

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

    R_REGISTER(RF24_Register::CONFIG, &config);

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

    W_REGISTER(RF24_Register::CONFIG, &config);

    __BOUNCE(crcConfig != getCrcConfig(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getChannel() {
    uint8_t channel;

    R_REGISTER(RF24_Register::RF_CH, &channel);

    __BOUNCE(channel > 127, __UINT8_MAX__);

    return (channel);
}

RF24_Status RF24::setChannel(uint8_t channel) {
    __BOUNCE(channel > 127, RF24_Status::UnknownChannel);

    W_REGISTER(RF24_Register::RF_CH, &channel);

    __BOUNCE(channel != getChannel(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_DataRate RF24::getDataRate() {
    uint8_t rf_setup;

    R_REGISTER(RF24_Register::RF_SETUP, &rf_setup);

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

    R_REGISTER(RF24_Register::RF_SETUP, &rf_setup);

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

    W_REGISTER(RF24_Register::RF_SETUP, &rf_setup);

    __BOUNCE(dataRate != getDataRate(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_OutputPower RF24::getOutputPower() {
    uint8_t rf_setup;

    R_REGISTER(RF24_Register::RF_SETUP, &rf_setup);

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

    R_REGISTER(RF24_Register::RF_SETUP, &rf_setup);

    // Default value
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
            // Default value
        } break;
    }

    W_REGISTER(RF24_Register::RF_SETUP, &rf_setup);

    __BOUNCE(outputPower != getOutputPower(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getRetryCount() {
    uint8_t setup_retr;

    R_REGISTER(RF24_Register::SETUP_RETR, &setup_retr);
    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

RF24_Status RF24::setRetryCount(uint8_t count) {
    uint8_t setup_retr;

    __BOUNCE(count > 0xF, RF24_Status::Failure);

    R_REGISTER(RF24_Register::SETUP_RETR, &setup_retr);
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(count, SETUP_RETR_ARC));
    W_REGISTER(RF24_Register::SETUP_RETR, &setup_retr);

    __BOUNCE(count != getRetryCount(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getRetryDelay() {
    uint8_t setup_retr;

    R_REGISTER(RF24_Register::SETUP_RETR, &setup_retr);
    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

RF24_Status RF24::setRetryDelay(uint8_t delay) {
    uint8_t setup_retr;

    __BOUNCE(delay > 0xF, RF24_Status::Failure);

    R_REGISTER(RF24_Register::SETUP_RETR, &setup_retr);
    OR_eq<uint8_t>(setup_retr, LEFT<uint8_t>(delay, SETUP_RETR_ARD));
    W_REGISTER(RF24_Register::SETUP_RETR, &setup_retr);

    __BOUNCE(delay != getRetryDelay(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint32_t RF24::getRxBaseAddress(uint8_t pipe) {
    uint32_t baseAddress      = 0;
    uint8_t baseAddressLength = addressLength - addressPrefixLength;
    uint8_t buffer[addressLength];

    // TODO: Verify pipe

    if (pipe > 0) {
        R_REGISTER(RF24_Register::RX_ADDR_P1, buffer, addressLength);
    } else {
        R_REGISTER(RF24_Register::RX_ADDR_P0, buffer, addressLength);
    }

    memcpy(&baseAddress, &buffer[baseAddressOffset], baseAddressLength);

    return (baseAddress);
}

RF24_Status RF24::setRxBaseAddress(uint8_t pipe, uint32_t baseAddress) {
    uint8_t baseAddressLength = addressLength - addressPrefixLength;
    uint8_t buffer[addressLength];

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    if (pipe > 0) {
        R_REGISTER(RF24_Register::RX_ADDR_P1, buffer, addressLength);
    } else {
        R_REGISTER(RF24_Register::RX_ADDR_P0, buffer, addressLength);
    }

    memcpy(&buffer[baseAddressOffset], &baseAddress, baseAddressLength);

    if (pipe > 0) {
        W_REGISTER(RF24_Register::RX_ADDR_P1, buffer, addressLength);
    } else {
        W_REGISTER(RF24_Register::RX_ADDR_P0, buffer, addressLength);
    }

    __BOUNCE(baseAddress != getRxBaseAddress(pipe), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint32_t RF24::getTxBaseAddress() {
    uint32_t baseAddress      = 0;
    uint8_t baseAddressLength = addressLength - addressPrefixLength;
    uint8_t buffer[addressLength];

    R_REGISTER(RF24_Register::TX_ADDR, buffer, addressLength);
    memcpy(&baseAddress, &buffer[baseAddressOffset], baseAddressLength);

    return (baseAddress);
}

RF24_Status RF24::setTxBaseAddress(uint32_t baseAddress) {
    uint8_t baseAddressLength = addressLength - addressPrefixLength;
    uint8_t buffer[addressLength];

    R_REGISTER(RF24_Register::TX_ADDR, buffer, addressLength);
    memcpy(&buffer[baseAddressOffset], &baseAddress, baseAddressLength);
    W_REGISTER(RF24_Register::TX_ADDR, buffer, addressLength);

    __BOUNCE(baseAddress != getTxBaseAddress(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getRxAddress(uint8_t pipe) {
    uint8_t address = 0;

    // TODO: Verify pipe

    switch (pipe) {
        case 0: R_REGISTER(RF24_Register::RX_ADDR_P0, &address, addressPrefixLength); break;
        case 1: R_REGISTER(RF24_Register::RX_ADDR_P1, &address, addressPrefixLength); break;
        case 2: R_REGISTER(RF24_Register::RX_ADDR_P2, &address, addressPrefixLength); break;
        case 3: R_REGISTER(RF24_Register::RX_ADDR_P3, &address, addressPrefixLength); break;
        case 4: R_REGISTER(RF24_Register::RX_ADDR_P4, &address, addressPrefixLength); break;
        case 5: R_REGISTER(RF24_Register::RX_ADDR_P5, &address, addressPrefixLength); break;
    }

    return (address);
}

RF24_Status RF24::setRxAddress(uint8_t pipe, uint8_t address) {
    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    switch (pipe) {
        case 0: W_REGISTER(RF24_Register::RX_ADDR_P0, &address, addressPrefixLength); break;
        case 1: W_REGISTER(RF24_Register::RX_ADDR_P1, &address, addressPrefixLength); break;
        case 2: W_REGISTER(RF24_Register::RX_ADDR_P2, &address, addressPrefixLength); break;
        case 3: W_REGISTER(RF24_Register::RX_ADDR_P3, &address, addressPrefixLength); break;
        case 4: W_REGISTER(RF24_Register::RX_ADDR_P4, &address, addressPrefixLength); break;
        case 5: W_REGISTER(RF24_Register::RX_ADDR_P5, &address, addressPrefixLength); break;
    }

    __BOUNCE(address != getRxAddress(pipe), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

uint8_t RF24::getTxAddress() {
    uint8_t address = 0;

    R_REGISTER(RF24_Register::TX_ADDR, &address, addressPrefixLength);

    return (address);
}

RF24_Status RF24::setTxAddress(uint8_t address) {
    W_REGISTER(RF24_Register::TX_ADDR, &address, addressPrefixLength);

    __BOUNCE(address != getTxAddress(), RF24_Status::VerificationFailed);

    return (RF24_Status::Success);
}

RF24_Status RF24::enableAutoAcknowledgment(uint8_t pipe, bool enable) {
    uint8_t en_aa;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    R_REGISTER(RF24_Register::EN_AA, &en_aa);

    if (enable) {
        setBit_eq<uint8_t>(en_aa, pipe);
    } else {
        clearBit_eq<uint8_t>(en_aa, pipe);
    }

    W_REGISTER(RF24_Register::EN_AA, &en_aa);

    // TODO: Verify

    return (RF24_Status::Success);
}

RF24_Status RF24::enableDataPipe(uint8_t pipe, bool enable) {
    uint8_t en_rxaddr;

    __BOUNCE(pipe > 5, RF24_Status::UnknownPipe);

    R_REGISTER(RF24_Register::EN_RXADDR, &en_rxaddr);

    if (enable) {
        setBit_eq<uint8_t>(en_rxaddr, pipe);
    } else {
        clearBit_eq<uint8_t>(en_rxaddr, pipe);
    }

    W_REGISTER(RF24_Register::EN_RXADDR, &en_rxaddr);

    // TODO: Verify

    return (RF24_Status::Success);
}

} /* namespace xXx */
