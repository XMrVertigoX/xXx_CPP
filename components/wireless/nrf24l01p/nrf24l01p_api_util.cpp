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
    clearBit_eq(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

void nRF24L01P_API::setSingleBit(Register_t reg, uint8_t bitIndex) {
    uint8_t tmp = readShortRegister(reg);
    setBit_eq(tmp, bitIndex);
    writeShortRegister(reg, tmp);
}

void nRF24L01P_API::clearInterrupts() {
    uint8_t status = cmd_NOP();
    setBit_eq<uint8_t>(status, STATUS_MAX_RT);
    setBit_eq<uint8_t>(status, STATUS_RX_DR);
    setBit_eq<uint8_t>(status, STATUS_TX_DS);
    writeShortRegister(Register_t::STATUS, status);
}

// ----- getters and setters --------------------------------------------------

Crc_t nRF24L01P_API::getCrcConfig() {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    if (!readBit<uint8_t>(config, CONFIG_EN_CRC)) {
        return (Crc_t::DISABLED);
    }

    if (readBit<uint8_t>(config, CONFIG_CRCO)) {
        return (Crc_t::CRC16);
    } else {
        return (Crc_t::CRC8);
    }
}

void nRF24L01P_API::setCrcConfig(Crc_t crc) {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    switch (crc) {
        case Crc_t::DISABLED: {
            clearBit_eq<uint8_t>(config, CONFIG_EN_CRC);
        } break;
        case Crc_t::CRC8: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            clearBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
        case Crc_t::CRC16: {
            setBit_eq<uint8_t>(config, CONFIG_EN_CRC);
            setBit_eq<uint8_t>(config, CONFIG_CRCO);
        } break;
    }

    writeShortRegister(Register_t::CONFIG, config);
}

uint8_t nRF24L01P_API::getChannel() {
    uint8_t channel = readShortRegister(Register_t::RF_CH);

    return (channel);
}

void nRF24L01P_API::setChannel(uint8_t channel) {
    if (channel <= RF_CH_MASK) {
        writeShortRegister(Register_t::RF_CH, channel);
    }
}

DataRate_t nRF24L01P_API::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    if (readBit<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW)) {
        return (DataRate_t::DataRate_250KBPS);
    }

    if (readBit<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH)) {
        return (DataRate_t::DataRate_2MBPS);
    }

    return (DataRate_t::DataRate_1MBPS);
}

void nRF24L01P_API::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    switch (dataRate) {
        case (DataRate_t::DataRate_1MBPS): {
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_t::DataRate_2MBPS: {
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            setBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
        case DataRate_t::DataRate_250KBPS: {
            setBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_LOW);
            clearBit_eq<uint8_t>(rfSetup, RF_SETUP_RF_DR_HIGH);
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rfSetup);
}

OutputPower_t nRF24L01P_API::getOutputPower() {
    uint8_t rf_setup = readShortRegister(Register_t::RF_SETUP);

    AND_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR_MASK);
    RIGHT_eq<uint8_t>(rf_setup, RF_SETUP_RF_PWR);

    switch (rf_setup) {
        case 0: return (OutputPower_t::PowerLevel_18dBm); break;
        case 1: return (OutputPower_t::PowerLevel_12dBm); break;
        case 2: return (OutputPower_t::PowerLevel_6dBm); break;
        default: return (OutputPower_t::PowerLevel_0dBm); break;
    }
}

void nRF24L01P_API::setOutputPower(OutputPower_t level) {
    uint8_t rf_setup = readShortRegister(Register_t::RF_SETUP);

    AND_eq(rf_setup, INVERT<uint8_t>(RF_SETUP_RF_PWR_MASK));

    switch (level) {
        case OutputPower_t::PowerLevel_18dBm: {
            OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(0b00, RF_SETUP_RF_PWR));
        } break;
        case OutputPower_t::PowerLevel_12dBm: {
            OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(0b01, RF_SETUP_RF_PWR));
        } break;
        case OutputPower_t::PowerLevel_6dBm: {
            OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(0b10, RF_SETUP_RF_PWR));
        } break;
        case OutputPower_t::PowerLevel_0dBm: {
            OR_eq<uint8_t>(rf_setup, LEFT<uint8_t>(0b11, RF_SETUP_RF_PWR));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rf_setup);
}

void nRF24L01P_API::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    AND_eq<uint8_t>(delay, SETUP_RETR_ARD_MASK);
    LEFT_eq<uint8_t>(delay, SETUP_RETR_ARD);

    AND_eq<uint8_t>(count, SETUP_RETR_ARC_MASK);
    LEFT_eq<uint8_t>(count, SETUP_RETR_ARC);

    setup_retr = OR<uint8_t>(delay, count);

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
            addressLength   = RX_ADDR_P0_LENGTH;
        } break;
        case 1: {
            addressRegister = Register_t::RX_ADDR_P1;
            addressLength   = RX_ADDR_P1_LENGTH;
        } break;
        case 2: {
            addressRegister = Register_t::RX_ADDR_P2;
            addressLength   = RX_ADDR_P2_LENGTH;
        } break;
        case 3: {
            addressRegister = Register_t::RX_ADDR_P3;
            addressLength   = RX_ADDR_P3_LENGTH;
        } break;
        case 4: {
            addressRegister = Register_t::RX_ADDR_P4;
            addressLength   = RX_ADDR_P4_LENGTH;
        } break;
        case 5: {
            addressRegister = Register_t::RX_ADDR_P5;
            addressLength   = RX_ADDR_P5_LENGTH;
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
            addressLength   = RX_ADDR_P0_LENGTH;
            addressMask     = RX_ADDR_P0_MASK;
        } break;
        case 1: {
            addressRegister = Register_t::RX_ADDR_P1;
            addressLength   = RX_ADDR_P1_LENGTH;
            addressMask     = RX_ADDR_P1_MASK;
        } break;
        case 2: {
            addressRegister = Register_t::RX_ADDR_P2;
            addressLength   = RX_ADDR_P2_LENGTH;
            addressMask     = RX_ADDR_P2_MASK;
        } break;
        case 3: {
            addressRegister = Register_t::RX_ADDR_P3;
            addressLength   = RX_ADDR_P3_LENGTH;
            addressMask     = RX_ADDR_P3_MASK;
        } break;
        case 4: {
            addressRegister = Register_t::RX_ADDR_P4;
            addressLength   = RX_ADDR_P4_LENGTH;
            addressMask     = RX_ADDR_P4_MASK;
        } break;
        case 5: {
            addressRegister = Register_t::RX_ADDR_P5;
            addressLength   = RX_ADDR_P5_LENGTH;
            addressMask     = RX_ADDR_P5_MASK;
        } break;
    }

    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(addressRegister, rx_addr, addressLength);
}

uint64_t nRF24L01P_API::getTxAddress() {
    uint64_t address = 0;
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_R_REGISTER(Register_t::TX_ADDR, tx_addr, TX_ADDR_LENGTH);

    return (address);
}

void nRF24L01P_API::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(Register_t::TX_ADDR, tx_addr, TX_ADDR_LENGTH);
}

} /* namespace xXx */
