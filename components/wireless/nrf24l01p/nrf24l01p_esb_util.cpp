#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_esb.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

namespace xXx {

static inline bool isPipeIndexValid(uint8_t pipeIndex) {
    if (pipeIndex > 5) {
        return (false);
    } else {
        return (true);
    }
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

void nRF24L01P_ESB::enableDataPipe(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));

    setSingleBit(Register_EN_RXADDR, pipeIndex);
    setSingleBit(Register_DYNPD, pipeIndex);
}

void nRF24L01P_ESB::disableDataPipe(uint8_t pipeIndex) {
    assert(isPipeIndexValid(pipeIndex));

    clearSingleBit(Register_EN_RXADDR, pipeIndex);
    clearSingleBit(Register_DYNPD, pipeIndex);
}

// ----- getters and setters --------------------------------------------------

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

void nRF24L01P_ESB::setCrcConfig(CRCConfig_t crc) {
    uint8_t config = readShortRegister(Register_CONFIG);

    switch (crc) {
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
}

int8_t nRF24L01P_ESB::getChannel() {
    int8_t channel = readShortRegister(Register_RF_CH);

    return (channel);
}

void nRF24L01P_ESB::setChannel(int8_t channel) {
    if (channel < 0) {
        writeShortRegister(Register_RF_CH, channel);
    }
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
}

OutputPower_t nRF24L01P_ESB::getOutputPower() {
    uint8_t rf_setup = readShortRegister(Register_RF_SETUP);

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    return (static_cast<OutputPower_t>(rf_setup));
}

void nRF24L01P_ESB::setOutputPower(OutputPower_t level) {
    uint8_t rf_setup = readShortRegister(Register_RF_SETUP);

    AND_eq(rf_setup, INVERT<uint8_t>(RF_SETUP_RF_PWR_MASK));
    OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(level, RF_SETUP_RF_PWR));

    writeShortRegister(Register_RF_SETUP, rf_setup);
}

uint8_t nRF24L01P_ESB::getRetryCount() {
    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARC_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARC);

    return (setup_retr);
}

void nRF24L01P_ESB::setRetryCount(uint8_t count) {
    assert(count <= 0xF);

    uint8_t setup_retr;

    setup_retr = readShortRegister(Register_SETUP_RETR);

    LEFT_eq<uint8_t>(count, SETUP_RETR_ARC);
    AND_eq<uint8_t>(setup_retr, INVERT<uint8_t>(SETUP_RETR_ARC_MASK));
    OR_eq<uint8_t>(setup_retr, count);

    writeShortRegister(Register_SETUP_RETR, setup_retr);
}

uint8_t nRF24L01P_ESB::getRetryDelay() {
    uint8_t setup_retr = readShortRegister(Register_SETUP_RETR);

    AND_eq<uint8_t>(setup_retr, SETUP_RETR_ARD_MASK);
    RIGHT_eq<uint8_t>(setup_retr, SETUP_RETR_ARD);

    return (setup_retr);
}

void nRF24L01P_ESB::setRetryDelay(uint8_t delay) {
    assert(delay <= 0xF);

    uint8_t setup_retr;

    setup_retr = readShortRegister(Register_SETUP_RETR);

    LEFT_eq<uint8_t>(delay, SETUP_RETR_ARD);
    AND_eq<uint8_t>(setup_retr, INVERT<uint8_t>(SETUP_RETR_ARD_MASK));
    OR_eq<uint8_t>(setup_retr, delay);

    writeShortRegister(Register_SETUP_RETR, setup_retr);
}

uint64_t nRF24L01P_ESB::getRxAddress(uint8_t pipe) {
    if (pipe > 5) {
        return (0);
    }

    Register_t addressRegister;
    uint64_t addressLength;

    switch (pipe) {
        case 0: {
            addressRegister = Register_RX_ADDR_P0;
            addressLength   = RX_ADDR_P0_LENGTH;
        } break;
        case 1: {
            addressRegister = Register_RX_ADDR_P1;
            addressLength   = RX_ADDR_P1_LENGTH;
        } break;
        case 2: {
            addressRegister = Register_RX_ADDR_P2;
            addressLength   = RX_ADDR_P2_LENGTH;
        } break;
        case 3: {
            addressRegister = Register_RX_ADDR_P3;
            addressLength   = RX_ADDR_P3_LENGTH;
        } break;
        case 4: {
            addressRegister = Register_RX_ADDR_P4;
            addressLength   = RX_ADDR_P4_LENGTH;
        } break;
        case 5: {
            addressRegister = Register_RX_ADDR_P5;
            addressLength   = RX_ADDR_P5_LENGTH;
        } break;
    }

    uint64_t address = 0;
    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_R_REGISTER(addressRegister, rx_addr, addressLength);

    return (address);
}

void nRF24L01P_ESB::setRxAddress(uint8_t pipe, uint64_t address) {
    if (pipe > 5) {
        return;
    }

    Register_t addressRegister;
    uint64_t addressLength;

    switch (pipe) {
        case 0: {
            addressRegister = Register_RX_ADDR_P0;
            addressLength   = RX_ADDR_P0_LENGTH;
        } break;
        case 1: {
            addressRegister = Register_RX_ADDR_P1;
            addressLength   = RX_ADDR_P1_LENGTH;
        } break;
        case 2: {
            addressRegister = Register_RX_ADDR_P2;
            addressLength   = RX_ADDR_P2_LENGTH;
        } break;
        case 3: {
            addressRegister = Register_RX_ADDR_P3;
            addressLength   = RX_ADDR_P3_LENGTH;
        } break;
        case 4: {
            addressRegister = Register_RX_ADDR_P4;
            addressLength   = RX_ADDR_P4_LENGTH;
        } break;
        case 5: {
            addressRegister = Register_RX_ADDR_P5;
            addressLength   = RX_ADDR_P5_LENGTH;
        } break;
    }

    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(addressRegister, rx_addr, addressLength);
}

uint64_t nRF24L01P_ESB::getTxAddress() {
    uint64_t address = 0;
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_R_REGISTER(Register_TX_ADDR, tx_addr, TX_ADDR_LENGTH);

    return (address);
}

void nRF24L01P_ESB::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(Register_TX_ADDR, tx_addr, TX_ADDR_LENGTH);
}

} /* namespace xXx */
