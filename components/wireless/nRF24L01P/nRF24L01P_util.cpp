#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nRF24L01P_register_map.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

// ----- Private helper functions ---------------------------------------------

uint8_t nRF24L01P::readShortRegister(Register_t address) {
    uint8_t result;

    cmd_R_REGISTER(address, &result, 1);

    return (result);
}

uint8_t nRF24L01P::writeShortRegister(Register_t address, uint8_t reg) {
    uint8_t status;

    status = cmd_W_REGISTER(address, &reg, 1);

    return (status);
}

void nRF24L01P::clearSingleBit(Register_t address, uint8_t bitIndex) {
    uint8_t reg = readShortRegister(address);
    clearBit_r(reg, bitIndex);
    writeShortRegister(address, reg);
}

void nRF24L01P::setSingleBit(Register_t address, uint8_t bitIndex) {
    uint8_t reg = readShortRegister(address);
    setBit_r(reg, bitIndex);
    writeShortRegister(address, reg);
}

uint8_t nRF24L01P::getPayloadLength() {
    uint8_t rxNumBytes;

    cmd_R_RX_PL_WID(rxNumBytes);

    return (rxNumBytes);
}

// ----- Public getters and setters -------------------------------------------

Crc_t nRF24L01P::getCrcConfig() {
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

void nRF24L01P::setCrcConfig(Crc_t crc) {
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

    assert(crc == getCrcConfig());
}

void nRF24L01P::setChannel(uint8_t channel) {
    if (channel < VALUE_8(RF_CH_t::RF_CH_MASK)) {
        writeShortRegister(Register_t::RF_CH, channel);
    } else {
        LOG("Channel index invalid: %d", channel);
    }
}

DataRate_t nRF24L01P::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    if (readBit(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_LOW))) {
        return (DataRate_t::DataRate_250KBPS);
    }

    if (readBit(rfSetup, VALUE_8(RF_SETUP_t::RF_DR_HIGH))) {
        return (DataRate_t::DataRate_2MBPS);
    }

    return (DataRate_t::DataRate_1MBPS);
}

void nRF24L01P::setDataRate(DataRate_t dataRate) {
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

    assert(dataRate == getDataRate());
}

void nRF24L01P::setOutputPower(OutputPower_t level) {
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

void nRF24L01P::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    bitwiseAND_r(delay, VALUE_8(SETUP_RETR_t::ARD_MASK));
    shiftLeft_r(delay, VALUE_8(SETUP_RETR_t::ARD));

    bitwiseAND_r(count, VALUE_8(SETUP_RETR_t::ARC_MASK));
    shiftLeft_r(count, VALUE_8(SETUP_RETR_t::ARC));

    setup_retr = bitwiseOR(delay, count);

    writeShortRegister(Register_t::SETUP_RETR, setup_retr);
}

void nRF24L01P::setRxAddress(uint8_t pipe, uint64_t address) {
    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    switch (pipe) {
        case 0: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P0, rx_addr,
                           VALUE_64(RX_ADDR_P0_t::LENGTH));
        } break;
        case 1: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P1, rx_addr,
                           VALUE_64(RX_ADDR_P1_t::LENGTH));
        } break;
        case 2: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P2, rx_addr,
                           VALUE_64(RX_ADDR_P2_t::LENGTH));
        } break;
        case 3: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P3, rx_addr,
                           VALUE_64(RX_ADDR_P3_t::LENGTH));
        } break;
        case 4: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P4, rx_addr,
                           VALUE_64(RX_ADDR_P4_t::LENGTH));
        } break;
        case 5: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P5, rx_addr,
                           VALUE_64(RX_ADDR_P5_t::LENGTH));
        } break;
    }
}

void nRF24L01P::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(Register_t::TX_ADDR, tx_addr, VALUE_64(TX_ADDR_t::LENGTH));
}
