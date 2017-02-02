#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_api.hpp>
#include <xXx/components/wireless/nrf24l01p/nrf24l01p_definitions.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

namespace xXx {

// ----- helper functions -----------------------------------------------------

uint8_t nRF24L01P_API::readShortRegister(Register_t reg) {
    uint8_t result;

    cmd_R_REGISTER(reg, &result, 1);

    return (result);
}

void nRF24L01P_API::writeShortRegister(Register_t reg, uint8_t val) {
    cmd_W_REGISTER(reg, &val, 1);
}

void nRF24L01P_API::clearSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    clearBit_r(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

void nRF24L01P_API::setSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    setBit_r(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

uint8_t nRF24L01P_API::getPayloadLength() {
    uint8_t rxNumBytes;

    cmd_R_RX_PL_WID(rxNumBytes);

    return (rxNumBytes);
}

void nRF24L01P_API::clearInterrupts() {
    uint8_t status = cmd_NOP();
    setBit_r(status, VALUE_8(STATUS_t::MAX_RT));
    setBit_r(status, VALUE_8(STATUS_t::RX_DR));
    setBit_r(status, VALUE_8(STATUS_t::TX_DS));
    writeShortRegister(Register_t::STATUS, status);
}

// ----- getters and setters --------------------------------------------------

Crc_t nRF24L01P_API::getCrcConfig() {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    if (!readBit(config, VALUE_8(CONFIG_t::EN_CRC))) {
        return (Crc_t::DISABLED);
    }

    if (readBit(config, VALUE_8(CONFIG_t::CRCO))) {
        return (Crc_t::CRC16);
    } else {
        return (Crc_t::CRC8);
    }
}

void nRF24L01P_API::setCrcConfig(Crc_t crc) {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    switch (crc) {
        case Crc_t::DISABLED: {
            clearBit_r(config, VALUE_8(CONFIG_t::EN_CRC));
        } break;
        case Crc_t::CRC8: {
            setBit_r(config, VALUE_8(CONFIG_t::EN_CRC));
            clearBit_r(config, VALUE_8(CONFIG_t::CRCO));
        } break;
        case Crc_t::CRC16: {
            setBit_r(config, VALUE_8(CONFIG_t::EN_CRC));
            setBit_r(config, VALUE_8(CONFIG_t::CRCO));
        } break;
    }

    writeShortRegister(Register_t::CONFIG, config);
}

uint8_t nRF24L01P_API::getChannel() {
    uint8_t channel = readShortRegister(Register_t::RF_CH);

    return (channel);
}

void nRF24L01P_API::setChannel(uint8_t channel) {
    if (channel <= VALUE_8(RF_CH_t::RF_CH_MASK)) {
        writeShortRegister(Register_t::RF_CH, channel);
    }
}

DataRate_t nRF24L01P_API::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    if (readBit(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_LOW))) {
        return (DataRate_t::DataRate_250KBPS);
    }

    if (readBit(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_HIGH))) {
        return (DataRate_t::DataRate_2MBPS);
    }

    return (DataRate_t::DataRate_1MBPS);
}

void nRF24L01P_API::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    switch (dataRate) {
        case (DataRate_t::DataRate_1MBPS): {
            clearBit_r(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case DataRate_t::DataRate_2MBPS: {
            clearBit_r(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case DataRate_t::DataRate_250KBPS: {
            setBit_r(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rfSetup);
}

void nRF24L01P_API::setOutputPower(OutputPower_t level) {
    uint8_t rf_setup = readShortRegister(Register_t::RF_SETUP);

    switch (level) {
        case OutputPower_t::PowerLevel_18dBm: {
            clearBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case OutputPower_t::PowerLevel_12dBm: {
            setBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case OutputPower_t::PowerLevel_6dBm: {
            clearBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case OutputPower_t::PowerLevel_0dBm: {
            setBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rf_setup, VALUE_8(RF_SETUP_t::RF_DR_HIGH));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rf_setup);
}

void nRF24L01P_API::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    bitwiseAND_r(delay, VALUE_8(SETUP_RETR_t::ARD_MASK));
    shiftLeft_r(delay, VALUE_8(SETUP_RETR_t::ARD));

    bitwiseAND_r(count, VALUE_8(SETUP_RETR_t::ARC_MASK));
    shiftLeft_r(count, VALUE_8(SETUP_RETR_t::ARC));

    setup_retr = bitwiseOR(delay, count);

    writeShortRegister(Register_t::SETUP_RETR, setup_retr);
}

uint64_t nRF24L01P_API::getRxAddress(uint8_t pipe) {
    if (pipe > 5) {
        return (0);
    }

    Register_t addressRegister;
    uint64_t addressLength;

    switch (pipe) {
        case 0: {
            addressRegister = Register_t::RX_ADDR_P0;
            addressLength   = VALUE_64(RX_ADDR_P0_t::LENGTH);
        } break;
        case 1: {
            addressRegister = Register_t::RX_ADDR_P1;
            addressLength   = VALUE_64(RX_ADDR_P1_t::LENGTH);
        } break;
        case 2: {
            addressRegister = Register_t::RX_ADDR_P2;
            addressLength   = VALUE_64(RX_ADDR_P2_t::LENGTH);
        } break;
        case 3: {
            addressRegister = Register_t::RX_ADDR_P3;
            addressLength   = VALUE_64(RX_ADDR_P3_t::LENGTH);
        } break;
        case 4: {
            addressRegister = Register_t::RX_ADDR_P4;
            addressLength   = VALUE_64(RX_ADDR_P4_t::LENGTH);
        } break;
        case 5: {
            addressRegister = Register_t::RX_ADDR_P5;
            addressLength   = VALUE_64(RX_ADDR_P5_t::LENGTH);
        } break;
    }

    uint64_t address = 0;
    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_R_REGISTER(addressRegister, rx_addr, addressLength);

    return (address);
}

void nRF24L01P_API::setRxAddress(uint8_t pipe, uint64_t address) {
    if (pipe > 5) {
        return;
    }

    Register_t addressRegister;
    uint64_t addressLength;
    uint64_t addressMask;

    switch (pipe) {
        case 0: {
            addressRegister = Register_t::RX_ADDR_P0;
            addressLength   = VALUE_64(RX_ADDR_P0_t::LENGTH);
            addressMask     = VALUE_64(RX_ADDR_P0_t::MASK);
        } break;
        case 1: {
            addressRegister = Register_t::RX_ADDR_P1;
            addressLength   = VALUE_64(RX_ADDR_P1_t::LENGTH);
            addressMask     = VALUE_64(RX_ADDR_P1_t::MASK);
        } break;
        case 2: {
            addressRegister = Register_t::RX_ADDR_P2;
            addressLength   = VALUE_64(RX_ADDR_P2_t::LENGTH);
            addressMask     = VALUE_64(RX_ADDR_P2_t::MASK);
        } break;
        case 3: {
            addressRegister = Register_t::RX_ADDR_P3;
            addressLength   = VALUE_64(RX_ADDR_P3_t::LENGTH);
            addressMask     = VALUE_64(RX_ADDR_P3_t::MASK);
        } break;
        case 4: {
            addressRegister = Register_t::RX_ADDR_P4;
            addressLength   = VALUE_64(RX_ADDR_P4_t::LENGTH);
            addressMask     = VALUE_64(RX_ADDR_P4_t::MASK);
        } break;
        case 5: {
            addressRegister = Register_t::RX_ADDR_P5;
            addressLength   = VALUE_64(RX_ADDR_P5_t::LENGTH);
            addressMask     = VALUE_64(RX_ADDR_P5_t::MASK);
        } break;
    }

    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(addressRegister, rx_addr, addressLength);
}

uint64_t nRF24L01P_API::getTxAddress() {
    uint64_t address = 0;
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_R_REGISTER(Register_t::TX_ADDR, tx_addr, VALUE_64(TX_ADDR_t::LENGTH));

    return (address);
}

void nRF24L01P_API::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(Register_t::TX_ADDR, tx_addr, VALUE_64(TX_ADDR_t::LENGTH));
}

} /* namespace xXx */
