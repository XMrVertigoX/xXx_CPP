#include <assert.h>
#include <stdint.h>

#include <xXx/components/wireless/nRF24L01P/nRF24L01P.hpp>
#include <xXx/components/wireless/nRF24L01P/nRF24L01P_definitions.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/bitoperations.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

nRF24L01P::nRF24L01P(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq) {}

nRF24L01P::~nRF24L01P() {}

void nRF24L01P::enterRxMode() {
    auto irqFunction = LAMBDA(void *user) {
        nRF24L01P *self = static_cast<nRF24L01P *>(user);
        uint8_t status  = self->cmd_NOP();
        uint8_t pipe    = status;

        if (readBit(status, VALUE(STATUS_t::RX_DR))) {
            uint8_t rxBytes[32];

            self->cmd_R_RX_PAYLOAD(rxBytes, sizeof(rxBytes));

            bitwiseAND_r(pipe, 0b1110); // TODO: Use macro for mask
            shiftRight_r(pipe, VALUE(STATUS_t::RX_P_NO));

            LOG("%d: %s", pipe, rxBytes);
        }

        self->cmd_W_REGISTER(Register_t::STATUS, &status, sizeof(status));
    };

    setSingleBit(Register_t::CONFIG, VALUE(CONFIG_t::PRIM_RX));

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

        if (readBit(status, VALUE(STATUS_t::MAX_RT))) {
            self->leaveTxMode();
            // self->cmd_FLUSH_TX();

            LOG("%p: MAX_RT", self);
        }

        if (readBit(status, VALUE(STATUS_t::TX_DS))) {
            self->leaveTxMode();
            // self->cmd_FLUSH_TX();

            LOG("%p: TX_DS", self);
        }

        self->cmd_W_REGISTER(Register_t::STATUS, &status, sizeof(status));
    };

    clearSingleBit(Register_t::CONFIG, VALUE(CONFIG_t::PRIM_RX));

    _irq.enableInterrupt(irqFunction, this);
    _ce.set();

    delayUs(130);
}

void nRF24L01P::leaveTxMode() {
    _irq.disableInterrupt();
    _ce.clear();
}

void nRF24L01P::init() {
    _ce.clear();

    // Clear IRQs
    writeShortRegister(Register_t::STATUS, cmd_NOP());

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();
}

// TODO
void nRF24L01P::configureRxPipe(uint8_t pipe, uint64_t address) {
    switch (pipe) {
        case 0: {
            setSingleBit(Register_t::EN_RXADDR, VALUE(EN_RXADDR_t::ERX_P0));
            writeShortRegister(Register_t::RX_PW_P0, maxPayloadSize);
        } break;
        case 1: {
            setSingleBit(Register_t::EN_RXADDR, VALUE(EN_RXADDR_t::ERX_P1));
            writeShortRegister(Register_t::RX_PW_P1, maxPayloadSize);
        } break;
        case 2: {
            setSingleBit(Register_t::EN_RXADDR, VALUE(EN_RXADDR_t::ERX_P2));
            writeShortRegister(Register_t::RX_PW_P2, maxPayloadSize);
        } break;
        case 3: {
            setSingleBit(Register_t::EN_RXADDR, VALUE(EN_RXADDR_t::ERX_P3));
            writeShortRegister(Register_t::RX_PW_P3, maxPayloadSize);
        } break;
        case 4: {
            setSingleBit(Register_t::EN_RXADDR, VALUE(EN_RXADDR_t::ERX_P4));
            writeShortRegister(Register_t::RX_PW_P4, maxPayloadSize);
        } break;
        case 5: {
            setSingleBit(Register_t::EN_RXADDR, VALUE(EN_RXADDR_t::ERX_P5));
            writeShortRegister(Register_t::RX_PW_P5, maxPayloadSize);
        } break;
    }
}

void nRF24L01P::configureTxPipe(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    bitwiseAND_r(address, longAddressMask);

    if (address) {
        cmd_W_REGISTER(Register_t::TX_ADDR, tx_addr, 5);
    } else {
        cmd_R_REGISTER(Register_t::TX_ADDR, tx_addr, 5);
    }

    cmd_W_REGISTER(Register_t::RX_ADDR_P0, tx_addr, 5);
}

void nRF24L01P::startListening() {
    enterRxMode();
}

void nRF24L01P::stopListening() {
    leaveRxMode();
}

void nRF24L01P::startWrite(uint8_t *bytes, size_t numBytes) {
    cmd_W_TX_PAYLOAD(bytes, numBytes);

    enterTxMode();
}

void nRF24L01P::setRxAddress(uint8_t pipe, uint64_t address) {
    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    switch (pipe) {
        case 0: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P0, rx_addr, 5);
        } break;
        case 1: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P1, rx_addr, 5);
        } break;
        case 2: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P2, rx_addr, 1);
        } break;
        case 3: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P3, rx_addr, 1);
        } break;
        case 4: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P4, rx_addr, 1);
        } break;
        case 5: {
            cmd_W_REGISTER(Register_t::RX_ADDR_P5, rx_addr, 1);
        } break;
    }
}

void nRF24L01P::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(Register_t::TX_ADDR, tx_addr, 5);
}

void nRF24L01P::powerUp() {
    setSingleBit(Register_t::CONFIG, VALUE(CONFIG_t::PWR_UP));
    delayMs(5); // TODO: Find explanation in data sheet
}

void nRF24L01P::powerDown() {
    clearSingleBit(Register_t::CONFIG, VALUE(CONFIG_t::PWR_UP));
}

void nRF24L01P::setChannel(uint8_t channel) {
    assert(channel <= maxChannel);

    if (channel <= maxChannel) {
        writeShortRegister(Register_t::RF_CH, channel);
    } else {
        LOG("Channel index invalid: %d", channel);
    }
}

Crc_t nRF24L01P::getCrcConfig() {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    if (!readBit(config, VALUE(CONFIG_t::EN_CRC))) {
        return (Crc_t::DISABLED);
    }

    if (readBit(config, VALUE(CONFIG_t::CRCO))) {
        return (Crc_t::CRC16);
    } else {
        return (Crc_t::CRC8);
    }
}

void nRF24L01P::setCrcConfig(Crc_t crc) {
    uint8_t config = readShortRegister(Register_t::CONFIG);

    switch (crc) {
        case Crc_t::DISABLED: {
            clearBit_r(config, VALUE(CONFIG_t::EN_CRC));
        } break;
        case Crc_t::CRC8: {
            setBit_r(config, VALUE(CONFIG_t::EN_CRC));
            clearBit_r(config, VALUE(CONFIG_t::CRCO));
        } break;
        case Crc_t::CRC16: {
            setBit_r(config, VALUE(CONFIG_t::EN_CRC));
            setBit_r(config, VALUE(CONFIG_t::CRCO));
        } break;
        default: break;
    }

    writeShortRegister(Register_t::CONFIG, config);
}

DataRate_t nRF24L01P::getDataRate() {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    if (readBit(rfSetup, VALUE(RF_SETUP_t::RF_DR_LOW))) {
        return (DataRate_t::DataRate_250KBPS);
    }

    if (readBit(rfSetup, VALUE(RF_SETUP_t::RF_DR_HIGH))) {
        return (DataRate_t::DataRate_2MBPS);
    }

    return (DataRate_t::DataRate_1MBPS);
}

void nRF24L01P::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(Register_t::RF_SETUP);

    switch (dataRate) {
        case (DataRate_t::DataRate_1MBPS): {
            clearBit_r(rfSetup, VALUE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rfSetup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case DataRate_t::DataRate_2MBPS: {
            clearBit_r(rfSetup, VALUE(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rfSetup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case DataRate_t::DataRate_250KBPS: {
            setBit_r(rfSetup, VALUE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rfSetup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rfSetup);
}

void nRF24L01P::setOutputPower(OutputPower_t level) {
    uint8_t rf_setup = readShortRegister(Register_t::RF_SETUP);

    switch (level) {
        case OutputPower_t::PowerLevel_18dBm: {
            clearBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case OutputPower_t::PowerLevel_12dBm: {
            setBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_LOW));
            clearBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case OutputPower_t::PowerLevel_6dBm: {
            clearBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
        case OutputPower_t::PowerLevel_0dBm: {
            setBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_LOW));
            setBit_r(rf_setup, VALUE(RF_SETUP_t::RF_DR_HIGH));
        } break;
    }

    writeShortRegister(Register_t::RF_SETUP, rf_setup);
}

void nRF24L01P::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    bitwiseAND_r(delay, 0xf);
    shiftLeft_r(delay, VALUE(SETUP_RETR_t::ARD));

    bitwiseAND_r(count, 0xf);
    shiftLeft_r(count, VALUE(SETUP_RETR_t::ARC));

    setup_retr = bitwiseOR(delay, count);

    writeShortRegister(Register_t::SETUP_RETR, setup_retr);
}
