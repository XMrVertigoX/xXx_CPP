#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nRF24L01P_definitions.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

nRF24L01P::nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq), payload_size(32),
      ack_payload_available(false), dynamic_payloads_enabled(false),
      pipe0_reading_address(0), ack_payload_length(0) {}

nRF24L01P::~nRF24L01P() {}

uint8_t nRF24L01P::getChannel() {
    return (readShortRegister(Register_t::RF_CH));
}

void nRF24L01P::setChannel(uint8_t channel) {
    bitwiseAND_r(channel, 0x7F); // TODO: Use macro for mask
    writeShortRegister(Register_t::RF_CH, channel);
}

void nRF24L01P::init() {
    _ce.clear();

    // Clear IRQs
    writeShortRegister(Register_t::STATUS, cmd_NOP());

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();
}

void nRF24L01P::enterRxMode() {
    auto irqFunction = LAMBDA(void *user) {
        nRF24L01P *self = static_cast<nRF24L01P *>(user);
        uint8_t status  = self->cmd_NOP();

        if (readBit(status, TYPE(STATUS_t::RX_DR))) {
            uint8_t rx[32];
            self->cmd_R_RX_PAYLOAD(rx, sizeof(rx));
            LOG("%s", rx, sizeof(rx));
        }

        self->writeShortRegister(Register_t::STATUS, status);
    };

    setSingleBit(Register_t::CONFIG, TYPE(CONFIG_t::PRIM_RX));

    _irq.enableInterrupt(irqFunction, this);
    _ce.set();

    delayUs(130);
}

void nRF24L01P::leaveRxMode() {
    _irq.disableInterrupt();
    _ce.clear();
}

void nRF24L01P::enterTxMode() {
    auto irqFunction = LAMBDA(void *user) {
        nRF24L01P *self = static_cast<nRF24L01P *>(user);
        uint8_t status  = self->cmd_NOP();

        if (readBit(status, TYPE(STATUS_t::MAX_RT))) {
            self->leaveTxMode();
            self->cmd_FLUSH_TX();

            LOG("%p: MAX_RT", self);
        }

        if (readBit(status, TYPE(STATUS_t::TX_DS))) {
            self->leaveTxMode();
            self->cmd_FLUSH_TX();

            LOG("%p: TX_DS", self);
        }

        self->writeShortRegister(Register_t::STATUS, status);
    };

    clearSingleBit(Register_t::CONFIG, TYPE(CONFIG_t::PRIM_RX));

    _irq.enableInterrupt(irqFunction, this);
    _ce.set();

    delayUs(130);
}

void nRF24L01P::leaveTxMode() {
    _irq.disableInterrupt();
    _ce.clear();
}

void nRF24L01P::setPowerState(bool enable) {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    if (enable) {
        setBit_r(config, TYPE(CONFIG_t::PWR_UP));
    } else {
        clearBit_r(config, TYPE(CONFIG_t::PWR_UP));
    }

    writeShortRegister(Register_t::CONFIG, config);

    assert(config == readShortRegister(Register_t::CONFIG));

    /*
	 * Must allow the radio time to settle else configuration bits will not
	 * necessarily stick. This is actually only required following power up but
	 * some settling time also appears to be required after resets too. For
	 * full coverage, we'll always assume the worst. Enabling 16b CRC is by far
	 * the most obvious case if the wrong timing is used - or skipped. Enabling
	 * 16b CRC is by far the most obvious case if the wrong timing is used - or
	 * skipped. Technically we require 4.5ms + 14us as a worst case. We'll just
	 * call it 5ms for good measure.
	 *
	 * WARNING: Delay is based on P-variant whereby non-P *may* require
	 * different timing.
	 */
    delayMs(5);
}

void nRF24L01P::startWrite(uint8_t *bytes, size_t numBytes) {
    cmd_W_TX_PAYLOAD(bytes, numBytes);

    enterTxMode();
}

void nRF24L01P::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(Register_t::TX_ADDR, tx_addr, 5);
}

void nRF24L01P::setRxAddress(uint8_t child, uint64_t address) {
    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    // TODO
    switch (child) {
        case 0: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P0, rx_addr, 5);
            writeShortRegister(Register_t::RX_PW_P0, 32);
            setSingleBit(Register_t::EN_RXADDR, TYPE(EN_RXADDR_t::ERX_P0));
        } break;
        case 1: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P1, rx_addr, 5);
            writeShortRegister(Register_t::RX_PW_P1, 32);
            setSingleBit(Register_t::EN_RXADDR, TYPE(EN_RXADDR_t::ERX_P1));
        } break;
    }
}

void nRF24L01P::setPowerLevel(PowerLevel_t level) {
    uint8_t rf_setup = readShortRegister(Register_t::RF_SETUP);

    switch (level) {
        case PowerLevel_t::PowerLevel_18dBm: {
            clearBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case PowerLevel_t::PowerLevel_12dBm: {
            setBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case PowerLevel_t::PowerLevel_6dBm: {
            clearBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case PowerLevel_t::PowerLevel_0dBm: {
            setBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rf_setup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rf_setup);

    assert(rf_setup == readShortRegister(Register_t::RF_SETUP));
}

PowerLevel_t nRF24L01P::getPowerLevel() {
    uint8_t rf_Setup = readShortRegister(Register_t::RF_SETUP);

    bitwiseAND_r(rf_Setup, 0b00000110); // TODO: Use macro for mask
    shiftRight_r(rf_Setup, 1);          // TODO: Use macro for shift

    switch (rf_Setup) {
        case TYPE(PowerLevel_t::PowerLevel_18dBm): {
            return (PowerLevel_t::PowerLevel_18dBm);
        } break;
        case TYPE(PowerLevel_t::PowerLevel_12dBm): {
            return (PowerLevel_t::PowerLevel_12dBm);
        } break;
        case TYPE(PowerLevel_t::PowerLevel_6dBm): {
            return (PowerLevel_t::PowerLevel_6dBm);
        } break;
    }

    return (PowerLevel_t::PowerLevel_0dBm);
}

void nRF24L01P::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    switch (dataRate) {
        case (DataRate_t::DataRate_1MBPS): {
            clearBit_r(rfSetup, TYPE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rfSetup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case DataRate_t::DataRate_2MBPS: {
            clearBit_r(rfSetup, TYPE(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rfSetup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case DataRate_t::DataRate_250KBPS: {
            setBit_r(rfSetup, TYPE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rfSetup, TYPE(RF_SETUP_t::RF_DR_HIGH));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rfSetup);

    assert(dataRate == getDataRate());
}

DataRate_t nRF24L01P::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    if (readBit(rfSetup, TYPE(RF_SETUP_t::RF_DR_LOW))) {
        return (DataRate_t::DataRate_250KBPS);
    }

    if (readBit(rfSetup, TYPE(RF_SETUP_t::RF_DR_HIGH))) {
        return (DataRate_t::DataRate_2MBPS);
    }

    return (DataRate_t::DataRate_1MBPS);
}

void nRF24L01P::setCRCConfig(CRC_t crc) {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    switch (crc) {
        case CRC_t::DISABLED: {
            clearBit_r(config, TYPE(CONFIG_t::EN_CRC));
        } break;
        case CRC_t::CRC8: {
            setBit_r(config, TYPE(CONFIG_t::EN_CRC));
            clearBit_r(config, TYPE(CONFIG_t::CRCO));
        } break;
        case CRC_t::CRC16: {
            setBit_r(config, TYPE(CONFIG_t::EN_CRC));
            setBit_r(config, TYPE(CONFIG_t::CRCO));
        } break;
        default: break;
    }

    writeShortRegister(Register_t::CONFIG, config);

    assert(config == readShortRegister(Register_t::CONFIG));
}

CRC_t nRF24L01P::getCRCConfig() {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    if (!readBit(config, TYPE(CONFIG_t::EN_CRC))) {
        return (CRC_t::DISABLED);
    }

    if (readBit(config, TYPE(CONFIG_t::CRCO))) {
        return (CRC_t::CRC16);
    } else {
        return (CRC_t::CRC8);
    }
}

void nRF24L01P::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    bitwiseAND_r(delay, 0xf);
    shiftLeft_r(delay, TYPE(SETUP_RETR_t::ARD));

    bitwiseAND_r(count, 0xf);
    shiftLeft_r(count, TYPE(SETUP_RETR_t::ARC));

    setup_retr = bitwiseOR(delay, count);

    writeShortRegister(Register_t::SETUP_RETR, setup_retr);

    assert(setup_retr == readShortRegister(Register_t::SETUP_RETR));
}
