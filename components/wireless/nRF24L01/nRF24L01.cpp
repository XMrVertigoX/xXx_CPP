#include <assert.h>
#include <string.h>

#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#include "nRF24L01.hpp"
#include "nRF24L01_definitions.hpp"
#include "util.hpp"

nRF24L01::nRF24L01(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq), payload_size(32),
      ack_payload_available(false), dynamic_payloads_enabled(false),
      pipe0_reading_address(0), ack_payload_length(0) {}

nRF24L01::~nRF24L01() {
    // TODO: Shutdown device
}

void nRF24L01::setChannel(uint8_t channel) {
    /*
     * TODO: This method could take advantage of the 'wide_band' calculation
     * done in setChannel() to require certain channel spacing.
     */
    const uint8_t max_channel = 127;
    writeShortRegister(RF_CH, min(channel, max_channel));
}

void nRF24L01::setPayloadSize(uint8_t size) {
    payload_size = min(size, maxPayloadSize);
}

uint8_t nRF24L01::getPayloadSize() {
    return (payload_size);
}

void nRF24L01::clearIRQs() {
    uint8_t status = 0x00;

    setBit_r(status, RX_DR);
    setBit_r(status, TX_DS);
    setBit_r(status, MAX_RT);

    writeShortRegister(STATUS, status);
}

void nRF24L01::init() {
    _ce.clear();

    setPowerState(true);

    /*
     * Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make
     * testing a little easier.
     *
     * WARNING: If this is ever lowered, either 250KBS mode with AA is broken
     * or maximum packet sizes must never be used. See documentation for a more
     * complete explanation.
     */
    // setRetries(0b0100, 0b1111);

    auto irqFunction = LAMBDA(void *user) {
        nRF24L01 *self = static_cast<nRF24L01 *>(user);
        self->clearIRQs();
    };

    _irq.enableInterrupt(irqFunction, this);
}

void nRF24L01::startListening() {
    setSingleBit(CONFIG, STATIC_CAST(CONFIG_t::PRIM_RX));
    _ce.set();
}

void nRF24L01::stopListening() {
    _ce.clear();

    cmd_FLUSH_TX();
    cmd_FLUSH_RX();
}

void nRF24L01::setPowerState(bool enable) {
    uint8_t config = readShortRegister(CONFIG);

    if (enable) {
        setBit_r(config, STATIC_CAST(CONFIG_t::PWR_UP));
    } else {
        clearBit_r(config, STATIC_CAST(CONFIG_t::PWR_UP));
    }

    writeShortRegister(CONFIG, config);

    assert(config == readShortRegister(CONFIG));

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

/******************************************************************/

bool nRF24L01::write(uint8_t *bytes, size_t numBytes) {
    bool result = false;

    // Begin the write
    startWrite(bytes, numBytes);

    // ------------
    // At this point we could return from a non-blocking write, and then call
    // the rest after an interrupt

    // Instead, we are going to block here until we get TX_DS (transmission completed and ack'd)
    // or MAX_RT (maximum retries, transmission failed).  Also, we'll timeout in case the radio
    // is flaky and we get neither.

    // IN the end, the send should be blocking.  It comes back in 60ms worst case, or much faster
    // if I tighted up the retry logic.  (Default settings will be 1500us.
    // Monitor the send
    // uint8_t observe_tx;
    uint8_t status;
    uint32_t sent_at       = getMillis();
    const uint32_t timeout = 500; //ms to wait for timeout
    do {
        // status = readShortRegister(RF24_MemoryMap::OBSERVE_TX, &observe_tx, 1);
        status = cmd_NOP();
    } while (!(status & ((1 << TX_DS) | (1 << MAX_RT))) &&
             (getMillis() - sent_at < timeout));

    // The part above is what you could recreate with your own interrupt handler,
    // and then call this when you got an interrupt
    // ------------

    // Call this when you get an interrupt
    // The status tells us three things
    // * The send was successful (TX_DS)
    // * The send failed, too many retries (MAX_RT)
    // * There is an ack packet waiting (RX_DR)
    bool tx_ok, tx_fail;
    whatHappened(tx_ok, tx_fail, ack_payload_available);

    result = tx_ok;

    // Handle the ack packet
    if (ack_payload_available) {
        ack_payload_length = getDynamicPayloadSize();
    }

    // Yay, we are done.

    // Flush bytesfers (Is this a relic of past experimentation, and not needed anymore??)
    cmd_FLUSH_TX();

    // setPowerState(false);

    return (result);
}

void nRF24L01::startWrite(uint8_t *bytes, size_t numBytes) {
    cmd_W_TX_PAYLOAD(bytes, numBytes);
    clearSingleBit(CONFIG, STATIC_CAST(CONFIG_t::PRIM_RX));

    _ce.set();
    delayUs(10);
    _ce.clear();
}

uint8_t nRF24L01::getDynamicPayloadSize() {
    uint8_t result;

    cmd_R_RX_PL_WID(&result, 1);

    return (result);
}

bool nRF24L01::available() {
    return (available(NULL));
}

// XXX
bool nRF24L01::available(uint8_t *pipe_num) {
    uint8_t status = cmd_NOP();

    // Too noisy, enable if you really want lots o data!!
    //IF_SERIAL_DEBUG(print_status(status));

    bool result = (status & (1 << RX_DR));

    if (result) {
        // If the caller wants the pipe number, include that
        if (pipe_num) *pipe_num = (status >> RX_P_NO) & 0b111;

        // Clear the status bit

        // ??? Should this REALLY be cleared now?  Or wait until we
        // actually READ the payload?

        writeShortRegister(STATUS, (1 << RX_DR));

        // Handle ack payload receipt
        if (status & (1 << TX_DS)) {
            writeShortRegister(STATUS, (1 << TX_DS));
        }
    }

    return (result);
}

bool nRF24L01::read(uint8_t *bytes, size_t numBytes) {
    uint8_t fifo_status = readShortRegister(FIFO_STATUS);

    // Fetch the payload
    cmd_R_RX_PAYLOAD(bytes, numBytes);

    // Was this the last of the data available?
    return (readBit(fifo_status, RX_EMPTY));
}

// XXX
void nRF24L01::whatHappened(bool &tx_ok, bool &tx_fail, bool &rx_ready) {
    uint8_t status = cmd_NOP();

    // Clear flags
    writeShortRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));

    tx_ok    = status & (1 << TX_DS);
    tx_fail  = status & (1 << MAX_RT);
    rx_ready = status & (1 << RX_DR);
}

void nRF24L01::setTxAddress(uint64_t address) {
    uint8_t *tx_addr = reinterpret_cast<uint8_t *>(&address);

    cmd_W_REGISTER(TX_ADDR, tx_addr, 5);
}

void nRF24L01::setRxAddress(uint8_t child, uint64_t address) {
    uint8_t *rx_addr = reinterpret_cast<uint8_t *>(&address);

    // TODO
    switch (child) {
        case 0: {
            cmd_W_REGISTER(RX_ADDR_P0, rx_addr, 5);
            writeShortRegister(RX_PW_P0, 32);
            writeShortRegister(EN_RXADDR, STATIC_CAST(EN_RXADDR_t::ERX_P0));
        } break;
    }
}

// TODO: Find in data sheet
void nRF24L01::toggle_features() {
    // uint8_t command = RF24_Command_ACTIVATE;
    // uint8_t foo     = 0x73;
    // uint8_t result;
    // uint8_t status = transmit(command, &foo, NULL, 1);
}

// XXX
void nRF24L01::enableDynamicPayloads() {
    // Enable dynamic payload throughout the system
    uint8_t feature = readShortRegister(FEATURE);
    setBit_r(feature, EN_DPL);
    writeShortRegister(FEATURE, feature);

    // If it didn't work, the features are not enabled
    if (!readShortRegister(FEATURE)) {
        // So enable them and try again
        toggle_features();
        readShortRegister(FEATURE);
        setBit_r(feature, EN_DPL);
        writeShortRegister(FEATURE, feature);
    }

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    uint8_t dynpd = readShortRegister(DYNPD);
    setBit_r(dynpd, DPL_P0);
    setBit_r(dynpd, DPL_P1);
    setBit_r(dynpd, DPL_P2);
    setBit_r(dynpd, DPL_P3);
    setBit_r(dynpd, DPL_P4);
    setBit_r(dynpd, DPL_P5);
    writeShortRegister(DYNPD, dynpd);

    dynamic_payloads_enabled = true;
}

// XXX
void nRF24L01::enableAckPayload() {
    //
    // enable ack payload and dynamic payload features
    //
    uint8_t feature = readShortRegister(FEATURE);
    setBit_r(feature, EN_ACK_PAY);
    setBit_r(feature, EN_DPL);
    writeShortRegister(FEATURE, feature);

    // If it didn't work, the features are not enabled
    if (!readShortRegister(FEATURE)) {
        // So enable them and try again
        toggle_features();
        readShortRegister(FEATURE);
        setBit_r(feature, EN_ACK_PAY);
        setBit_r(feature, EN_DPL);
        writeShortRegister(FEATURE, feature);
    }

    LOG("FEATURE=%i\r\n", readShortRegister(FEATURE));

    //
    // Enable dynamic payload on pipes 0 & 1
    //
    uint8_t dynpd = readShortRegister(DYNPD);
    setBit_r(dynpd, DPL_P0);
    setBit_r(dynpd, DPL_P1);
    writeShortRegister(DYNPD, dynpd);
}

// XXX
void nRF24L01::writeAckPayload(uint8_t pipe, uint8_t *bytes, size_t numBytes) {
    uint8_t data_len = min(numBytes, maxPayloadSize);
    uint8_t status   = cmd_W_ACK_PAYLOAD(pipe, bytes, numBytes);
}

// XXX
bool nRF24L01::isAckPayloadAvailable() {
    bool result           = ack_payload_available;
    ack_payload_available = false;
    return (result);
}

// TODO: Split into two functions: enable and disable.
void nRF24L01::setAutoAck(bool enable) {
    if (enable)
        writeShortRegister(EN_AA, 0b111111);
    else
        writeShortRegister(EN_AA, 0);
}

/*
 * TODO: Use enum for pipes, split into two functions
 */
void nRF24L01::setAutoAck(uint8_t pipe, bool enable) {
    uint8_t en_aa = readShortRegister(EN_AA);

    if (pipe >= 6) {
        return;
    }

    if (enable) {
        en_aa |= (1 << pipe);
    } else {
        en_aa &= ~(1 << pipe);
    }

    writeShortRegister(EN_AA, en_aa);

    assert(en_aa == readShortRegister(EN_AA));
}

// XXX
bool nRF24L01::testRPD() {
    uint8_t rpd = readShortRegister(RPD);

    return ((bool)rpd);
}

void nRF24L01::setPowerLevel(PowerLevel_t level) {
    uint8_t rf_setup = readShortRegister(RF_SETUP);

    switch (level) {
        case RF24_PA_18dBm: {
            clearBit_r(rf_setup, RF_PWR_LOW);
            clearBit_r(rf_setup, RF_PWR_HIGH);
        } break;
        case RF24_PA_12dBm: {
            setBit_r(rf_setup, RF_PWR_LOW);
            clearBit_r(rf_setup, RF_PWR_HIGH);
        } break;
        case RF24_PA_6dBm: {
            clearBit_r(rf_setup, RF_PWR_LOW);
            setBit_r(rf_setup, RF_PWR_HIGH);
        } break;
        case RF24_PA_0dBm: {
            setBit_r(rf_setup, RF_PWR_LOW);
            setBit_r(rf_setup, RF_PWR_HIGH);
        } break;
    }

    writeShortRegister(RF_SETUP, rf_setup);

    assert(rf_setup == readShortRegister(RF_SETUP));
}

PowerLevel_t nRF24L01::getPowerLevel() {
    uint8_t rfSetup = readShortRegister(RF_SETUP);

    bitwiseAND_r(rfSetup, RF_PWR_MASK);
    shiftRight_r(rfSetup, 1);

    return ((PowerLevel_t)rfSetup);
}

void nRF24L01::setDataRate(DataRate_t dataRate) {
    uint8_t rfSetup = readShortRegister(RF_SETUP);

    switch (dataRate) {
        case (DataRate_t::SPEED_1MBPS): {
            clearBit_r(rfSetup, RF_DR_LOW);
            clearBit_r(rfSetup, RF_DR_HIGH);
        } break;
        case DataRate_t::SPEED_2MBPS: {
            clearBit_r(rfSetup, RF_DR_LOW);
            setBit_r(rfSetup, RF_DR_HIGH);
        } break;
        case DataRate_t::SPEED_250KBPS: {
            setBit_r(rfSetup, RF_DR_LOW);
            clearBit_r(rfSetup, RF_DR_HIGH);
        } break;
    }

    writeShortRegister(RF_SETUP, rfSetup);

    assert(dataRate == getDataRate());
}

DataRate_t nRF24L01::getDataRate() {
    uint8_t rfSetup = readShortRegister(RF_SETUP);

    if (readBit(rfSetup, RF_DR_LOW)) {
        return (DataRate_t::SPEED_250KBPS);
    }

    if (readBit(rfSetup, RF_DR_HIGH)) {
        return (DataRate_t::SPEED_2MBPS);
    }

    return (DataRate_t::SPEED_1MBPS);
}

/*
 * TODO: This function also enables CRC!
 */
void nRF24L01::setCRCConfig(CRC_t crc) {
    uint8_t config = readShortRegister(CONFIG);

    switch (crc) {
        case RF24_CRC_DISABLED: {
            clearBit_r(config, STATIC_CAST(CONFIG_t::EN_CRC));
        } break;
        case RF24_CRC_8: {
            setBit_r(config, STATIC_CAST(CONFIG_t::EN_CRC));
            clearBit_r(config, STATIC_CAST(CONFIG_t::CRCO));
        } break;
        case RF24_CRC_16: {
            setBit_r(config, STATIC_CAST(CONFIG_t::EN_CRC));
            setBit_r(config, STATIC_CAST(CONFIG_t::CRCO));
        } break;
        default: break;
    }

    writeShortRegister(CONFIG, config);

    assert(config == readShortRegister(CONFIG));
}

/*
 * TODO: This function also checks whether CRC is enabled or not.
 */
CRC_t nRF24L01::getCRCConfig() {
    uint8_t config = readShortRegister(CONFIG);

    if (!readBit(config, STATIC_CAST(CONFIG_t::EN_CRC))) {
        return (RF24_CRC_DISABLED);
    }

    if (readBit(config, STATIC_CAST(CONFIG_t::CRCO))) {
        return (RF24_CRC_16);
    } else {
        return (RF24_CRC_8);
    }
}

void nRF24L01::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    bitwiseAND_r(delay, 0xf);
    shiftLeft_r(delay, STATIC_CAST(SETUP_RETR_t::ARD));

    bitwiseAND_r(count, 0xf);
    shiftLeft_r(count, STATIC_CAST(SETUP_RETR_t::ARC));

    setup_retr = bitwiseOR(delay, count);

    writeShortRegister(SETUP_RETR, setup_retr);

    assert(setup_retr == readShortRegister(SETUP_RETR));
}
