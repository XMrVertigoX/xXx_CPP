/*
 * Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <string.h>

#include <xXx/components/wireless/RF24/RF24.hpp>
#include <xXx/components/wireless/RF24/RF24_config.h>
#include <xXx/components/wireless/RF24/nRF24L01_definitions.h>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/utils/logging.hpp>

static const uint8_t dummy = 0xFF;

static const uint8_t child_pipe[] = {RX_ADDR_P0, RX_ADDR_P1, RX_ADDR_P2,
                                     RX_ADDR_P3, RX_ADDR_P4, RX_ADDR_P5};
static const uint8_t child_payload_size[] = {RX_PW_P0, RX_PW_P1, RX_PW_P2,
                                             RX_PW_P3, RX_PW_P4, RX_PW_P5};
static const uint8_t child_pipe_enable[] = {ERX_P0, ERX_P1, ERX_P2,
                                            ERX_P3, ERX_P4, ERX_P5};

static uint8_t transmit(ISpi &spi, uint8_t command, uint8_t const txBytes[],
                        uint8_t rxBytes[], size_t numBytes) {
    uint8_t status;

    size_t transmitNumBytes = numBytes + 1;
    uint8_t mosiBytes[transmitNumBytes];
    uint8_t misoBytes[transmitNumBytes];

    mosiBytes[0] = command;

    if (txBytes != NULL) {
        memcpy(&mosiBytes[1], txBytes, numBytes);
    } else {
        memset(&mosiBytes[1], dummy, numBytes);
    }

    spi.transmit(mosiBytes, misoBytes, transmitNumBytes);

    if (rxBytes != NULL) {
        memcpy(&misoBytes[1], rxBytes, numBytes);
    }

    status = misoBytes[0];

    return (status);
}

static inline void clearBit(uint8_t &byte, uint8_t bit) {
    byte &= ~(1 << bit);
}

static inline bool readBit(uint8_t byte, uint8_t bit) {
    return (byte & (1 << bit));
}

static inline void setBit(uint8_t &byte, uint8_t bit) {
    byte |= (1 << bit);
}

uint8_t RF24::read_register(uint8_t reg, uint8_t bytes[], uint8_t numBytes) {
    uint8_t command = R_REGISTER | (REGISTER_MASK & reg);
    uint8_t status  = transmit(_spi, command, NULL, bytes, numBytes);

    return (status);
}

uint8_t RF24::read_register(uint8_t reg) {
    uint8_t result;
    uint8_t status = read_register(reg, &result, 1);

    return (result);
}

uint8_t RF24::write_register(uint8_t reg, uint8_t const bytes[],
                             uint8_t numBytes) {
    uint8_t command = W_REGISTER | (REGISTER_MASK & reg);
    uint8_t status  = transmit(_spi, command, bytes, NULL, numBytes);

    return (status);
}

uint8_t RF24::write_register(uint8_t reg, uint8_t value) {
    uint8_t status = write_register(reg, &value, 1);

    return (status);
}

uint8_t RF24::write_payload(const uint8_t *buf, uint8_t len) {
    uint8_t data_len = min(len, payload_size);

    uint8_t tempBuffer[payload_size] = {};
    memcpy(tempBuffer, buf, data_len);

    uint8_t command = W_TX_PAYLOAD;
    uint8_t status  = transmit(_spi, command, tempBuffer, NULL, payload_size);

    return (status);
}

uint8_t RF24::read_payload(uint8_t *buf, uint8_t len) {
    uint8_t command = R_RX_PAYLOAD;
    uint8_t status = transmit(_spi, command, NULL, buf, min(len, payload_size));

    return (status);
}

uint8_t RF24::flush_rx(void) {
    uint8_t command = FLUSH_RX;
    uint8_t status  = transmit(_spi, command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24::flush_tx(void) {
    uint8_t command = FLUSH_TX;
    uint8_t status  = transmit(_spi, command, NULL, NULL, 0);

    return (status);
}

uint8_t RF24::get_status(void) {
    uint8_t command = NOP;
    uint8_t status  = transmit(_spi, command, NULL, NULL, 0);

    return (status);
}

RF24::RF24(ISpi &spi, IGpio &ce, IGpio &irq)
    : _spi(spi), _ce(ce), _irq(irq), wide_band(true), p_variant(true),
      payload_size(32), ack_payload_available(false),
      dynamic_payloads_enabled(false), pipe0_reading_address(0),
      ack_payload_length(0) {}

void RF24::setChannel(uint8_t channel) {
    /*
     * TODO: This method could take advantage of the 'wide_band' calculation
     * done in setChannel() to require certain channel spacing.
     */
    const uint8_t max_channel = 127;
    write_register(RF_CH, min(channel, max_channel));
}

void RF24::setPayloadSize(uint8_t size) {
    const uint8_t max_payload_size = 32;
    payload_size                   = min(size, max_payload_size);
}

uint8_t RF24::getPayloadSize(void) {
    return (payload_size);
}

void RF24::begin(void) {
    _ce.clear();

    // Must allow the radio time to settle else configuration bits will not necessarily stick.
    // This is actually only required following power up but some settling time also appears to
    // be required after resets too. For full coverage, we'll always assume the worst.
    // Enabling 16b CRC is by far the most obvious case if the wrong timing is used - or skipped.
    // Technically we require 4.5ms + 14us as a worst case. We'll just call it 5ms for good measure.
    // WARNING: Delay is based on P-variant whereby non-P *may* require different timing.
    delayMs(5);

    // Set 1500uS (minimum for 32B payload in ESB@250KBPS) timeouts, to make testing a little easier
    // WARNING: If this is ever lowered, either 250KBS mode with AA is broken or maximum packet
    // sizes must never be used. See documentation for a more complete explanation.
    write_register(SETUP_RETR, (0b0100 << ARD) | (0b1111 << ARC));

    // Restore our default PA level
    setPALevel(RF24_PA_MAX);

    // Determine if this is a p or non-p RF24 module and then
    // reset our data rate back to default value. This works
    // because a non-P variant won't allow the data rate to
    // be set to 250Kbps.
    if (setDataRate(RF24_250KBPS)) {
        p_variant = true;
    }

    // Then set the data rate to the slowest (and most reliable) speed supported by all
    // hardware.
    setDataRate(RF24_1MBPS);

    // Initialize CRC and request 2-byte (16bit) CRC
    setCRCLength(RF24_CRC_16);

    // Disable dynamic payloads, to match dynamic_payloads_enabled setting
    write_register(DYNPD, 0);

    // Reset current status
    // Notice reset and flush is the last thing we do
    write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Set up default configuration.  Callers can always change it later.
    // This channel should be universally safe and not bleed over into adjacent
    // spectrum.
    setChannel(76);

    // Flush buffers
    flush_rx();
    flush_tx();
}

void RF24::startListening(void) {
    write_register(CONFIG, read_register(CONFIG) | _BV(PWR_UP) | _BV(PRIM_RX));
    write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Restore the pipe0 adddress, if exists
    if (pipe0_reading_address)
        write_register(
            RX_ADDR_P0,
            reinterpret_cast<const uint8_t *>(&pipe0_reading_address), 5);

    // Flush buffers
    flush_rx();
    flush_tx();

    // Go!
    _ce.set();

    // wait for the radio to come up (130us actually only needed)
    delayUs(130);
}

void RF24::stopListening(void) {
    _ce.clear();
    flush_tx();
    flush_rx();
}

void RF24::powerDown(void) {
    write_register(CONFIG, read_register(CONFIG) & ~_BV(PWR_UP));
}

void RF24::powerUp(void) {
    write_register(CONFIG, read_register(CONFIG) | _BV(PWR_UP));
}

/******************************************************************/

bool RF24::write(const uint8_t *buf, uint8_t len) {
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
    uint8_t observe_tx;
    uint8_t status;
    uint32_t sent_at       = getMillis();
    const uint32_t timeout = 500; //ms to wait for timeout
    do {
        status = read_register(OBSERVE_TX, &observe_tx, 1);
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

    // Power down
    powerDown();

    // Flush buffers (Is this a relic of past experimentation, and not needed anymore??)
    flush_tx();

    return (result);
}

void RF24::startWrite(const uint8_t *buf, uint8_t len) {
    // Transmitter power-up
    write_register(CONFIG,
                   (read_register(CONFIG) | _BV(PWR_UP)) & ~_BV(PRIM_RX));
    delayUs(150);

    // Send the payload
    write_payload(buf, len);

    // Allons!
    _ce.set();
    delayUs(15);
    _ce.clear();
}

uint8_t RF24::getDynamicPayloadSize(void) {

    uint8_t command = R_RX_PL_WID;
    uint8_t result;
    uint8_t status = transmit(_spi, command, NULL, &result, 1);

    return (result);
}

bool RF24::available(void) {
    return (available(NULL));
}

bool RF24::available(uint8_t *pipe_num) {
    uint8_t status = get_status();

    // Too noisy, enable if you really want lots o data!!
    //IF_SERIAL_DEBUG(print_status(status));

    bool result = (status & _BV(RX_DR));

    if (result) {
        // If the caller wants the pipe number, include that
        if (pipe_num) *pipe_num = (status >> RX_P_NO) & 0b111;

        // Clear the status bit

        // ??? Should this REALLY be cleared now?  Or wait until we
        // actually READ the payload?

        write_register(STATUS, _BV(RX_DR));

        // Handle ack payload receipt
        if (status & _BV(TX_DS)) {
            write_register(STATUS, _BV(TX_DS));
        }
    }

    return (result);
}

bool RF24::read(uint8_t *buf, uint8_t len) {
    // Fetch the payload
    read_payload(buf, len);

    // was this the last of the data available?
    return (read_register(FIFO_STATUS) & _BV(RX_EMPTY));
}

void RF24::whatHappened(bool &tx_ok, bool &tx_fail, bool &rx_ready) {
    // Read the status & reset the status in one easy call
    // Or is that such a good idea?
    uint8_t status =
        write_register(STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));

    // Report to the user what happened
    tx_ok    = status & _BV(TX_DS);
    tx_fail  = status & _BV(MAX_RT);
    rx_ready = status & _BV(RX_DR);
}

void RF24::openWritingPipe(uint64_t value) {
    write_register(RX_ADDR_P0, reinterpret_cast<uint8_t *>(&value), 5);
    write_register(TX_ADDR, reinterpret_cast<uint8_t *>(&value), 5);

    const uint8_t max_payload_size = 32;
    write_register(RX_PW_P0, min(payload_size, max_payload_size));
}

void RF24::openReadingPipe(uint8_t child, uint64_t address) {
    // If this is pipe 0, cache the address.  This is needed because
    // openWritingPipe() will overwrite the pipe 0 address, so
    // startListening() will have to restore it.
    if (child == 0) pipe0_reading_address = address;

    if (child <= 6) {
        // For pipes 2-5, only write the LSB
        if (child < 2)
            write_register(pgm_read_byte(&child_pipe[child]),
                           reinterpret_cast<const uint8_t *>(&address), 5);
        else
            write_register(pgm_read_byte(&child_pipe[child]),
                           reinterpret_cast<const uint8_t *>(&address), 1);

        write_register(pgm_read_byte(&child_payload_size[child]), payload_size);

        // Note it would be more efficient to set all of the bits for all open
        // pipes at once.  However, I thought it would make the calling code
        // more simple to do it this way.
        write_register(EN_RXADDR,
                       read_register(EN_RXADDR) |
                           _BV(pgm_read_byte(&child_pipe_enable[child])));
    }
}

void RF24::toggle_features(void) {
    uint8_t command = ACTIVATE;
    uint8_t foo     = 0x73;
    uint8_t result;
    uint8_t status = transmit(_spi, command, &foo, NULL, 1);
}

void RF24::enableDynamicPayloads(void) {
    // Enable dynamic payload throughout the system
    write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));

    // If it didn't work, the features are not enabled
    if (!read_register(FEATURE)) {
        // So enable them and try again
        toggle_features();
        write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));
    }

    LOG("FEATURE=%i\r\n", read_register(FEATURE));

    // Enable dynamic payload on all pipes
    //
    // Not sure the use case of only having dynamic payload on certain
    // pipes, so the library does not support it.
    write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) |
                              _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) |
                              _BV(DPL_P0));

    dynamic_payloads_enabled = true;
}

void RF24::enableAckPayload(void) {
    //
    // enable ack payload and dynamic payload features
    //

    write_register(FEATURE,
                   read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));

    // If it didn't work, the features are not enabled
    if (!read_register(FEATURE)) {
        // So enable them and try again
        toggle_features();
        write_register(FEATURE,
                       read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));
    }

    LOG("FEATURE=%i\r\n", read_register(FEATURE));

    //
    // Enable dynamic payload on pipes 0 & 1
    //

    write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
}

void RF24::writeAckPayload(uint8_t pipe, const uint8_t *buf, uint8_t len) {
    const uint8_t max_payload_size = 32;
    uint8_t data_len               = min(len, max_payload_size);
    uint8_t command                = W_ACK_PAYLOAD | (pipe & 0b111);
    uint8_t status = transmit(_spi, command, buf, NULL, data_len);
}

bool RF24::isAckPayloadAvailable(void) {
    bool result           = ack_payload_available;
    ack_payload_available = false;
    return (result);
}

bool RF24::isPVariant(void) {
    return (p_variant);
}

void RF24::setAutoAck(bool enable) {
    if (enable)
        write_register(EN_AA, 0b111111);
    else
        write_register(EN_AA, 0);
}

void RF24::setAutoAck(uint8_t pipe, bool enable) {
    if (pipe <= 6) {
        uint8_t en_aa = read_register(EN_AA);
        if (enable) {
            en_aa |= _BV(pipe);
        } else {
            en_aa &= ~_BV(pipe);
        }
        write_register(EN_AA, en_aa);
    }
}

bool RF24::testCarrier(void) {
    return (read_register(CD) & 1);
}

bool RF24::testRPD(void) {
    return (read_register(RPD) & 1);
}

void RF24::setPALevel(RF24_PALevel_t level) {
    uint8_t setup = read_register(RF_SETUP);
    setup &= ~(_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));

    // switch uses RAM (evil!)
    if (level == RF24_PA_MAX) {
        setup |= (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));
    } else if (level == RF24_PA_HIGH) {
        setup |= _BV(RF_PWR_HIGH);
    } else if (level == RF24_PA_LOW) {
        setup |= _BV(RF_PWR_LOW);
    } else if (level == RF24_PA_MIN) {
        // nothing
    } else if (level == RF24_PA_ERROR) {
        // On error, go to maximum PA
        setup |= (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));
    }

    write_register(RF_SETUP, setup);
}

RF24_PALevel_t RF24::getPALevel(void) {
    RF24_PALevel_t result = RF24_PA_ERROR;
    uint8_t power =
        read_register(RF_SETUP) & (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH));

    // switch uses RAM (evil!)
    if (power == (_BV(RF_PWR_LOW) | _BV(RF_PWR_HIGH))) {
        result = RF24_PA_MAX;
    } else if (power == _BV(RF_PWR_HIGH)) {
        result = RF24_PA_HIGH;
    } else if (power == _BV(RF_PWR_LOW)) {
        result = RF24_PA_LOW;
    } else {
        result = RF24_PA_MIN;
    }

    return (result);
}

bool RF24::setDataRate(RF24_DataRate_t speed) {
    bool result   = false;
    uint8_t setup = read_register(RF_SETUP);

    // // HIGH and LOW '00' is 1Mbs - our default
    // wide_band = false;
    // setup &= ~(_BV(RF_DR_LOW) | _BV(RF_DR_HIGH));
    // if (speed == RF24_250KBPS) {
    //     // Must set the RF_DR_LOW to 1; RF_DR_HIGH (used to be RF_DR) is already 0
    //     // Making it '10'.
    //     wide_band = false;
    //     setup |= _BV(RF_DR_LOW);
    // } else {
    //     // Set 2Mbs, RF_DR (RF_DR_HIGH) is set 1
    //     // Making it '01'
    //     if (speed == RF24_2MBPS) {
    //         wide_band = true;
    //         setup |= _BV(RF_DR_HIGH);
    //     } else {
    //         // 1Mbs
    //         wide_band = false;
    //     }
    // }

    // TODO: Set masks at once
    switch (speed) {
        case RF24_1MBPS: {
            clearBit(setup, RF_DR_LOW);
            clearBit(setup, RF_DR_HIGH);
            wide_band = false;
        } break;
        case RF24_2MBPS: {
            //setup |= _BV(RF_DR_HIGH);
            clearBit(setup, RF_DR_LOW);
            setBit(setup, RF_DR_HIGH);
            wide_band = true;
        } break;
        case RF24_250KBPS: {
            //setup |= _BV(RF_DR_LOW);
            setBit(setup, RF_DR_LOW);
            clearBit(setup, RF_DR_HIGH);
            wide_band = false;
        } break;
    }

    write_register(RF_SETUP, setup);

    /*
     * Verify our result
     *
     * TODO: Necessary?
     */
    if (read_register(RF_SETUP) == setup) {
        result = true;
    } else {
        wide_band = false;
    }

    return (result);
}

RF24_DataRate_t RF24::getDataRate(void) {
    uint8_t rfSetup = read_register(RF_SETUP);

    if (readBit(rfSetup, RF_DR_LOW)) {
        return (RF24_250KBPS);
    }

    if (readBit(rfSetup, RF_DR_HIGH)) {
        return (RF24_2MBPS);
    }

    return (RF24_1MBPS);
}

void RF24::setCRCLength(RF24_CRCLength_t length) {
    uint8_t config = read_register(CONFIG) & ~(_BV(CRCO) | _BV(EN_CRC));

    // switch uses RAM (evil!)
    if (length == RF24_CRC_DISABLED) {
        // Do nothing, we turned it off above.
    } else if (length == RF24_CRC_8) {
        config |= _BV(EN_CRC);
    } else {
        config |= _BV(EN_CRC);
        config |= _BV(CRCO);
    }

    write_register(CONFIG, config);
}

RF24_CRCLength_t RF24::getCRCLength(void) {
    RF24_CRCLength_t result = RF24_CRC_DISABLED;
    uint8_t config          = read_register(CONFIG) & (_BV(CRCO) | _BV(EN_CRC));

    if (config & _BV(EN_CRC)) {
        if (config & _BV(CRCO))
            result = RF24_CRC_16;
        else
            result = RF24_CRC_8;
    }

    return (result);
}

void RF24::disableCRC(void) {
    uint8_t disable = read_register(CONFIG) & ~_BV(EN_CRC);
    write_register(CONFIG, disable);
}

void RF24::setRetries(uint8_t delay, uint8_t count) {
    write_register(SETUP_RETR, (delay & 0xf) << ARD | (count & 0xf) << ARC);
}
