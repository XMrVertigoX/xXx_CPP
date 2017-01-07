#include <assert.h>
#include <string.h>

#include <xXx/components/wireless/nRF24L01/nRF24L01.hpp>
#include <xXx/components/wireless/nRF24L01/nRF24L01_constants.hpp>
#include <xXx/components/wireless/nRF24L01/nRF24L01_utilities.h>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/logging.hpp>

#include <nRF24L01_config.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

static const uint8_t dummy = 0xFF;

// XXX
static const nRF24L01_MemoryMap_t child_pipe[] = {
    nRF24L01_MemoryMap_t::RX_ADDR_P0, nRF24L01_MemoryMap_t::RX_ADDR_P1,
    nRF24L01_MemoryMap_t::RX_ADDR_P2, nRF24L01_MemoryMap_t::RX_ADDR_P3,
    nRF24L01_MemoryMap_t::RX_ADDR_P4, nRF24L01_MemoryMap_t::RX_ADDR_P5};
// XXX
static const nRF24L01_MemoryMap_t child_payload_size[] = {
    nRF24L01_MemoryMap_t::RX_PW_P0, nRF24L01_MemoryMap_t::RX_PW_P1,
    nRF24L01_MemoryMap_t::RX_PW_P2, nRF24L01_MemoryMap_t::RX_PW_P3,
    nRF24L01_MemoryMap_t::RX_PW_P4, nRF24L01_MemoryMap_t::RX_PW_P5};
// XXX
static const uint8_t child_pipe_enable[] = {RF24_ERX_P0, RF24_ERX_P1,
                                            RF24_ERX_P2, RF24_ERX_P3,
                                            RF24_ERX_P4, RF24_ERX_P5};

uint8_t nRF24L01::transmit(uint8_t command, uint8_t const txBytes[],
                           uint8_t rxBytes[], size_t numBytes) {
    uint8_t status;
    size_t dataNumBytes = numBytes + 1;
    uint8_t mosiBytes[dataNumBytes];
    uint8_t misoBytes[dataNumBytes];

    mosiBytes[0] = command;

    if (txBytes != NULL) {
        memcpy(&mosiBytes[1], txBytes, numBytes);
    } else {
        memset(&mosiBytes[1], dummy, numBytes);
    }

    _spi.transmit(mosiBytes, misoBytes, dataNumBytes);

    status = misoBytes[0];

    if (rxBytes != NULL) {
        memcpy(rxBytes, &misoBytes[1], numBytes);
    }

    return (status);
}

uint8_t nRF24L01::read_register(nRF24L01_MemoryMap_t address, uint8_t bytes[],
                                uint8_t numBytes) {
    uint8_t command = bitwiseOR(__castCMD(nRF24L01_Command_t::R_REGISTER),
                                __castMEM(address));
    uint8_t status = transmit(command, NULL, bytes, numBytes);

    return (status);
}

uint8_t nRF24L01::read_register(nRF24L01_MemoryMap_t address) {
    uint8_t result;

    read_register(address, &result, 1);

    return (result);
}

uint8_t nRF24L01::write_register(nRF24L01_MemoryMap_t address,
                                 uint8_t const bytes[], uint8_t numBytes) {
    uint8_t command = bitwiseOR(__castCMD(nRF24L01_Command_t::W_REGISTER),
                                __castMEM(address));
    uint8_t status = transmit(command, bytes, NULL, numBytes);

    return (status);
}

void nRF24L01::write_register(nRF24L01_MemoryMap_t address, uint8_t value) {
    write_register(address, &value, 1);
}

// XXX
uint8_t nRF24L01::write_payload(const uint8_t *buf, uint8_t len) {
    uint8_t data_len = min(len, payload_size);

    uint8_t tempBuffer[payload_size] = {};
    memcpy(tempBuffer, buf, data_len);

    uint8_t command = __castCMD(nRF24L01_Command_t::W_TX_PAYLOAD);
    uint8_t status  = transmit(command, tempBuffer, NULL, payload_size);

    return (status);
}

// XXX
uint8_t nRF24L01::read_payload(uint8_t *buf, uint8_t len) {
    uint8_t command = __castCMD(nRF24L01_Command_t::R_RX_PAYLOAD);
    uint8_t status  = transmit(command, NULL, buf, min(len, payload_size));

    return (status);
}

uint8_t nRF24L01::flush_rx(void) {
    uint8_t command = __castCMD(nRF24L01_Command_t::FLUSH_RX);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01::flush_tx(void) {
    uint8_t command = __castCMD(nRF24L01_Command_t::FLUSH_TX);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

uint8_t nRF24L01::getStatus(void) {
    uint8_t command = __castCMD(nRF24L01_Command_t::NOP);
    uint8_t status  = transmit(command, NULL, NULL, 0);

    return (status);
}

nRF24L01::nRF24L01(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq), /*wide_band(true),*/ p_variant(true),
      payload_size(32), ack_payload_available(false),
      dynamic_payloads_enabled(false), pipe0_reading_address(0),
      ack_payload_length(0) {}

void nRF24L01::setChannel(uint8_t channel) {
    /*
     * TODO: This method could take advantage of the 'wide_band' calculation
     * done in setChannel() to require certain channel spacing.
     */
    const uint8_t max_channel = 127;
    write_register(nRF24L01_MemoryMap_t::RF_CH, min(channel, max_channel));
}

void nRF24L01::setPayloadSize(uint8_t size) {
    payload_size = min(size, maxPayloadSize);
}

uint8_t nRF24L01::getPayloadSize(void) {
    return (payload_size);
}

void nRF24L01::resetIRQ() {
    uint8_t status = read_register(nRF24L01_MemoryMap_t::STATUS);
    setBit_r(status, RX_DR);
    setBit_r(status, TX_DS);
    setBit_r(status, MAX_RT);
    write_register(nRF24L01_MemoryMap_t::STATUS, status);
}

void nRF24L01::init(void) {
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
    setRetries(0b1111, 0b1111);

    setCRCLength(RF24_CRC_16);

    resetIRQ();

    // flush_rx();
    // flush_tx();
}

void nRF24L01::startListening(void) {
    uint8_t config = read_register(nRF24L01_MemoryMap_t::CONFIG);

    setBit_r(config, RF24_Config_PWR_UP);
    setBit_r(config, RF24_Config_PRIM_RX);

    write_register(nRF24L01_MemoryMap_t::CONFIG, config);

    resetIRQ();

    // Restore the pipe0 adddress, if exists
    if (pipe0_reading_address)
        write_register(nRF24L01_MemoryMap_t::RX_ADDR_P0,
                       reinterpret_cast<uint8_t *>(&pipe0_reading_address), 5);

    // Flush buffers
    flush_rx();
    flush_tx();

    // Go!
    _ce.set();

    // wait for the radio to come up (130us actually only needed)
    delayUs(rxSettling);
}

void nRF24L01::stopListening(void) {
    _ce.clear();
    flush_tx();
    flush_rx();
}

void nRF24L01::setPowerState(bool enable) {
    uint8_t config = read_register(nRF24L01_MemoryMap_t::CONFIG);

    if (enable) {
        setBit_r(config, RF24_Config_PWR_UP);
    } else {
        clearBit_r(config, RF24_Config_PWR_UP);
    }

    write_register(nRF24L01_MemoryMap_t::CONFIG, config);

    assert(config == read_register(nRF24L01_MemoryMap_t::CONFIG));

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

bool nRF24L01::write(const uint8_t *buf, uint8_t len) {
    bool result = false;

    // Begin the write
    startWrite(buf, len);

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
        // status = read_register(RF24_MemoryMap::OBSERVE_TX, &observe_tx, 1);
        status = getStatus();
    } while (!(status & (_BV(TX_DS) | _BV(MAX_RT))) &&
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

    // Flush buffers (Is this a relic of past experimentation, and not needed anymore??)
    flush_tx();

    // setPowerState(false);

    return (result);
}

void nRF24L01::startWrite(const uint8_t *buf, uint8_t len) {
    uint8_t config = read_register(nRF24L01_MemoryMap_t::CONFIG);

    setBit_r(config, RF24_Config_PWR_UP);
    setBit_r(config, RF24_Config_PRIM_RX);

    write_register(nRF24L01_MemoryMap_t::CONFIG, config);

    assert(config == read_register(nRF24L01_MemoryMap_t::CONFIG));

    delayUs(txSettling);

    write_payload(buf, len);

    _ce.set();

    delayUs(10);

    _ce.clear();
}

uint8_t nRF24L01::getDynamicPayloadSize(void) {
    uint8_t result;

    transmit(__castCMD(nRF24L01_Command_t::R_RX_PL_WID), NULL, &result, 1);

    return (result);
}

bool nRF24L01::available(void) {
    return (available(NULL));
}

// XXX
bool nRF24L01::available(uint8_t *pipe_num) {
    uint8_t status = getStatus();

    // Too noisy, enable if you really want lots o data!!
    //IF_SERIAL_DEBUG(print_status(status));

    bool result = (status & _BV(RX_DR));

    if (result) {
        // If the caller wants the pipe number, include that
        if (pipe_num) *pipe_num = (status >> RX_P_NO) & 0b111;

        // Clear the status bit

        // ??? Should this REALLY be cleared now?  Or wait until we
        // actually READ the payload?

        write_register(nRF24L01_MemoryMap_t::STATUS, _BV(RX_DR));

        // Handle ack payload receipt
        if (status & _BV(TX_DS)) {
            write_register(nRF24L01_MemoryMap_t::STATUS, _BV(TX_DS));
        }
    }

    return (result);
}

bool nRF24L01::read(uint8_t *buf, uint8_t len) {
    uint8_t fifo_status = read_register(nRF24L01_MemoryMap_t::FIFO_STATUS);

    // Fetch the payload
    read_payload(buf, len);

    // Was this the last of the data available?
    return (readBit(fifo_status, RX_EMPTY));
}

// XXX
void nRF24L01::whatHappened(bool &tx_ok, bool &tx_fail, bool &rx_ready) {
    uint8_t status = getStatus();

    // Clear flags
    write_register(nRF24L01_MemoryMap_t::STATUS,
                   _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    tx_ok    = status & _BV(TX_DS);
    tx_fail  = status & _BV(MAX_RT);
    rx_ready = status & _BV(RX_DR);
}

void nRF24L01::openWritingPipe(uint64_t address) {
    write_register(nRF24L01_MemoryMap_t::RX_ADDR_P0,
                   reinterpret_cast<uint8_t *>(&address), maxAddressLength);
    write_register(nRF24L01_MemoryMap_t::TX_ADDR,
                   reinterpret_cast<uint8_t *>(&address), maxAddressLength);
    write_register(nRF24L01_MemoryMap_t::RX_PW_P0,
                   min(payload_size, maxPayloadSize));
}

// XXX
void nRF24L01::openReadingPipe(uint8_t child, uint64_t address) {
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) pipe0_reading_address = address;

    if (child <= 6) {
        // For pipes 2-5, only write the LSB
        if (child < 2)
            write_register(pgm_read_byte(&child_pipe[child]),
                           reinterpret_cast<uint8_t *>(&address), 5);
        else
            write_register(pgm_read_byte(&child_pipe[child]),
                           reinterpret_cast<uint8_t *>(&address), 1);

        write_register(pgm_read_byte(&child_payload_size[child]), payload_size);

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        uint8_t en_rxaddr = read_register(nRF24L01_MemoryMap_t::EN_RXADDR);

        setBit_r(en_rxaddr, pgm_read_byte(&child_pipe_enable[child]));
        write_register(nRF24L01_MemoryMap_t::EN_RXADDR, en_rxaddr);
    }
}

// TODO: Find in data sheet
void nRF24L01::toggle_features(void) {
    // uint8_t command = RF24_Command_ACTIVATE;
    // uint8_t foo     = 0x73;
    // uint8_t result;
    // uint8_t status = transmit(command, &foo, NULL, 1);
}

// XXX
void nRF24L01::enableDynamicPayloads(void) {
    // Enable dynamic payload throughout the system
    uint8_t feature = read_register(nRF24L01_MemoryMap_t::FEATURE);
    setBit_r(feature, EN_DPL);
    write_register(nRF24L01_MemoryMap_t::FEATURE, feature);

    // If it didn't work, the features are not enabled
    if (!read_register(nRF24L01_MemoryMap_t::FEATURE)) {
        // So enable them and try again
        toggle_features();
        read_register(nRF24L01_MemoryMap_t::FEATURE);
        setBit_r(feature, EN_DPL);
        write_register(nRF24L01_MemoryMap_t::FEATURE, feature);
    }

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    uint8_t dynpd = read_register(nRF24L01_MemoryMap_t::DYNPD);
    setBit_r(dynpd, DPL_P0);
    setBit_r(dynpd, DPL_P1);
    setBit_r(dynpd, DPL_P2);
    setBit_r(dynpd, DPL_P3);
    setBit_r(dynpd, DPL_P4);
    setBit_r(dynpd, DPL_P5);
    write_register(nRF24L01_MemoryMap_t::DYNPD, dynpd);

    dynamic_payloads_enabled = true;
}

// XXX
void nRF24L01::enableAckPayload(void) {
    //
    // enable ack payload and dynamic payload features
    //
    uint8_t feature = read_register(nRF24L01_MemoryMap_t::FEATURE);
    setBit_r(feature, EN_ACK_PAY);
    setBit_r(feature, EN_DPL);
    write_register(nRF24L01_MemoryMap_t::FEATURE, feature);

    // If it didn't work, the features are not enabled
    if (!read_register(nRF24L01_MemoryMap_t::FEATURE)) {
        // So enable them and try again
        toggle_features();
        read_register(nRF24L01_MemoryMap_t::FEATURE);
        setBit_r(feature, EN_ACK_PAY);
        setBit_r(feature, EN_DPL);
        write_register(nRF24L01_MemoryMap_t::FEATURE, feature);
    }

    LOG("FEATURE=%i\r\n", read_register(nRF24L01_MemoryMap_t::FEATURE));

    //
    // Enable dynamic payload on pipes 0 & 1
    //
    uint8_t dynpd = read_register(nRF24L01_MemoryMap_t::DYNPD);
    setBit_r(dynpd, DPL_P0);
    setBit_r(dynpd, DPL_P1);
    write_register(nRF24L01_MemoryMap_t::DYNPD, dynpd);
}

// XXX
void nRF24L01::writeAckPayload(uint8_t pipe, const uint8_t *buf, uint8_t len) {
    uint8_t data_len = min(len, maxPayloadSize);
    uint8_t command =
        __castCMD(nRF24L01_Command_t::W_ACK_PAYLOAD) | (pipe & 0b111);
    uint8_t status = transmit(command, buf, NULL, data_len);
}

// XXX
bool nRF24L01::isAckPayloadAvailable(void) {
    bool result           = ack_payload_available;
    ack_payload_available = false;
    return (result);
}

// TODO: Split into two functions: enable and disable.
void nRF24L01::setAutoAck(bool enable) {
    if (enable)
        write_register(nRF24L01_MemoryMap_t::EN_AA, 0b111111);
    else
        write_register(nRF24L01_MemoryMap_t::EN_AA, 0);
}

/*
 * TODO: Use enum for pipes, split into two functions
 */
void nRF24L01::setAutoAck(uint8_t pipe, bool enable) {
    uint8_t en_aa = read_register(nRF24L01_MemoryMap_t::EN_AA);

    if (pipe >= 6) {
        return;
    }

    if (enable) {
        en_aa |= _BV(pipe);
    } else {
        en_aa &= ~_BV(pipe);
    }

    write_register(nRF24L01_MemoryMap_t::EN_AA, en_aa);

    assert(en_aa == read_register(nRF24L01_MemoryMap_t::EN_AA));
}

// XXX
bool nRF24L01::testCarrier(void) {
    uint8_t rpd = read_register(nRF24L01_MemoryMap_t::CD);

    return ((bool)rpd);
}

// XXX
bool nRF24L01::testRPD(void) {
    uint8_t rpd = read_register(nRF24L01_MemoryMap_t::RPD);

    return ((bool)rpd);
}

void nRF24L01::setPowerLevel(RF24_PowerLevel_t level) {
    uint8_t rf_setup = read_register(nRF24L01_MemoryMap_t::RF_SETUP);

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

    write_register(nRF24L01_MemoryMap_t::RF_SETUP, rf_setup);

    assert(rf_setup == read_register(nRF24L01_MemoryMap_t::RF_SETUP));
}

RF24_PowerLevel_t nRF24L01::getPowerLevel(void) {
    uint8_t rfSetup = read_register(nRF24L01_MemoryMap_t::RF_SETUP);

    bitwiseAND_r(rfSetup, RF_PWR_MASK);
    shiftRight_r(rfSetup, 1);

    return ((RF24_PowerLevel_t)rfSetup);
}

void nRF24L01::setDataRate(nRF24L01_DataRate_t dataRate) {
    uint8_t rfSetup = read_register(nRF24L01_MemoryMap_t::RF_SETUP);

    switch (dataRate) {
        case (nRF24L01_DataRate_t::SPEED_1MBPS): {
            clearBit_r(rfSetup, RF_DR_LOW);
            clearBit_r(rfSetup, RF_DR_HIGH);
        } break;
        case nRF24L01_DataRate_t::SPEED_2MBPS: {
            clearBit_r(rfSetup, RF_DR_LOW);
            setBit_r(rfSetup, RF_DR_HIGH);
        } break;
        case nRF24L01_DataRate_t::SPEED_250KBPS: {
            setBit_r(rfSetup, RF_DR_LOW);
            clearBit_r(rfSetup, RF_DR_HIGH);
        } break;
    }

    write_register(nRF24L01_MemoryMap_t::RF_SETUP, rfSetup);

    assert(dataRate == getDataRate());
}

nRF24L01_DataRate_t nRF24L01::getDataRate(void) {
    uint8_t rfSetup = read_register(nRF24L01_MemoryMap_t::RF_SETUP);

    if (readBit(rfSetup, RF_DR_LOW)) {
        return (nRF24L01_DataRate_t::SPEED_250KBPS);
    }

    if (readBit(rfSetup, RF_DR_HIGH)) {
        return (nRF24L01_DataRate_t::SPEED_2MBPS);
    }

    return (nRF24L01_DataRate_t::SPEED_1MBPS);
}

/*
 * TODO: This function also enables CRC!
 */
void nRF24L01::setCRCLength(RF24_CRC_t crc) {
    uint8_t config = read_register(nRF24L01_MemoryMap_t::CONFIG);

    switch (crc) {
        case RF24_CRC_DISABLED: {
            clearBit_r(config, RF24_Config_EN_CRC);
        } break;
        case RF24_CRC_8: {
            setBit_r(config, RF24_Config_EN_CRC);
            clearBit_r(config, RF24_Config_CRCO);
        } break;
        case RF24_CRC_16: {
            setBit_r(config, RF24_Config_EN_CRC);
            setBit_r(config, RF24_Config_CRCO);
        } break;
        default: break;
    }

    write_register(nRF24L01_MemoryMap_t::CONFIG, config);

    assert(config == read_register(nRF24L01_MemoryMap_t::CONFIG));
}

/*
 * TODO: This function also checks whether CRC is enabled or not.
 */
RF24_CRC_t nRF24L01::getCRCLength(void) {
    uint8_t config = read_register(nRF24L01_MemoryMap_t::CONFIG);

    if (!readBit(config, RF24_Config_EN_CRC)) {
        return (RF24_CRC_DISABLED);
    }

    if (readBit(config, RF24_Config_CRCO)) {
        return (RF24_CRC_16);
    } else {
        return (RF24_CRC_8);
    }
}

void nRF24L01::setRetries(uint8_t delay, uint8_t count) {
    uint8_t setup_retr;

    bitwiseAND_r(delay, 0xf);
    shiftLeft_r(delay, RF24_SETUP_RETR_ARD);

    bitwiseAND_r(count, 0xf);
    shiftLeft_r(count, RF24_SETUP_RETR_ARC);

    setup_retr = bitwiseOR(delay, count);

    write_register(nRF24L01_MemoryMap_t::SETUP_RETR, setup_retr);

    assert(setup_retr == read_register(nRF24L01_MemoryMap_t::SETUP_RETR));
}
