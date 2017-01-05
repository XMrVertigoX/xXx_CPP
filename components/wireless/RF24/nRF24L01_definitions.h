/*
 * Copyright (c) 2007 Stefan Engelke <mbox@stefanengelke.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef NRF24L01_DEFINITIONS_H_
#define NRF24L01_DEFINITIONS_H_

enum RF24_Command_t {
    RF24_Command_R_REGISTER    = 0x00,
    RF24_Command_W_REGISTER    = 0x20,
    RF24_Command_REGISTER_MASK = 0x1F,
    RF24_Command_ACTIVATE      = 0x50,
    RF24_Command_R_RX_PL_WID   = 0x60,
    RF24_Command_R_RX_PAYLOAD  = 0x61,
    RF24_Command_W_TX_PAYLOAD  = 0xA0,
    RF24_Command_W_ACK_PAYLOAD = 0xA8,
    RF24_Command_FLUSH_TX      = 0xE1,
    RF24_Command_FLUSH_RX      = 0xE2,
    RF24_Command_REUSE_TX_PL   = 0xE3,
    RF24_Command_NOP           = 0xFF
};

enum RF24_MemoryMap_t {
    RF24_MM_CONFIG      = 0x00,
    RF24_MM_EN_AA       = 0x01,
    RF24_MM_EN_RXADDR   = 0x02,
    RF24_MM_SETUP_AW    = 0x03,
    RF24_MM_SETUP_RETR  = 0x04,
    RF24_MM_RF_CH       = 0x05,
    RF24_MM_RF_SETUP    = 0x06,
    RF24_MM_STATUS      = 0x07,
    RF24_MM_OBSERVE_TX  = 0x08,
    RF24_MM_CD          = 0x09,
    RF24_MM_RX_ADDR_P0  = 0x0A,
    RF24_MM_RX_ADDR_P1  = 0x0B,
    RF24_MM_RX_ADDR_P2  = 0x0C,
    RF24_MM_RX_ADDR_P3  = 0x0D,
    RF24_MM_RX_ADDR_P4  = 0x0E,
    RF24_MM_RX_ADDR_P5  = 0x0F,
    RF24_MM_TX_ADDR     = 0x10,
    RF24_MM_RX_PW_P0    = 0x11,
    RF24_MM_RX_PW_P1    = 0x12,
    RF24_MM_RX_PW_P2    = 0x13,
    RF24_MM_RX_PW_P3    = 0x14,
    RF24_MM_RX_PW_P4    = 0x15,
    RF24_MM_RX_PW_P5    = 0x16,
    RF24_MM_FIFO_STATUS = 0x17,
    RF24_MM_DYNPD       = 0x1C,
    RF24_MM_FEATURE     = 0x1D
};

enum RF24_Config_t {
    RF24_Config_PRIM_RX,
    RF24_Config_PWR_UP,
    RF24_Config_CRCO,
    RF24_Config_EN_CRC,
    RF24_Config_MASK_MAX_RT,
    RF24_Config_MASK_TX_DS,
    RF24_Config_MASK_RX_DR
};

enum RF24_EN_AA_t {
    RF24_ENAA_P0,
    RF24_ENAA_P1,
    RF24_ENAA_P2,
    RF24_ENAA_P3,
    RF24_ENAA_P4,
    RF24_ENAA_P5
};

enum RF24_EN_RXADDR_t {
    RF24_ERX_P0,
    RF24_ERX_P1,
    RF24_ERX_P2,
    RF24_ERX_P3,
    RF24_ERX_P4,
    RF24_ERX_P5
};

// clang-format off

/* Bit Mnemonics */
#define AW            (0)
#define ARD           (4)
#define ARC           (0)
#define PLL_LOCK      (4)
#define RF_DR         (3)
#define RF_PWR        (6)
#define RX_DR         (6)
#define TX_DS         (5)
#define MAX_RT        (4)
#define RX_P_NO       (1)
#define TX_FULL       (0)
#define PLOS_CNT      (4)
#define ARC_CNT       (0)
#define TX_REUSE      (6)
#define FIFO_FULL     (5)
#define TX_EMPTY      (4)
#define RX_FULL       (1)
#define RX_EMPTY      (0)
#define DPL_P5        (5)
#define DPL_P4        (4)
#define DPL_P3        (3)
#define DPL_P2        (2)
#define DPL_P1        (1)
#define DPL_P0        (0)
#define EN_DPL        (2)
#define EN_ACK_PAY    (1)
#define EN_DYN_ACK    (0)

/* Non-P omissions */
#define LNA_HCURR     (0)

/* P model memory Map */
#define RPD           (0x09)

/* P model bit Mnemonics */
#define RF_DR_LOW     (5)
#define RF_DR_HIGH    (3)
#define RF_PWR_LOW    (1)
#define RF_PWR_HIGH   (2)

// clang-format on

#endif // NRF24L01_DEFINITIONS_H_
