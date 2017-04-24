#if not defined(NRF24L01P_DEFINITIONS_HPP_)
#define NRF24L01P_DEFINITIONS_HPP_

#include <stdint.h>

static const uint8_t txFifoSize = 32;
static const uint8_t rxFifoSize = 32;
static const uint8_t txSettling = 130;
static const uint8_t rxSettling = 130;

static const uint8_t RX_ADDR_P0_LENGTH = 5;
static const uint8_t RX_ADDR_P1_LENGTH = 5;
static const uint8_t RX_ADDR_P2_LENGTH = 1;
static const uint8_t RX_ADDR_P3_LENGTH = 1;
static const uint8_t RX_ADDR_P4_LENGTH = 1;
static const uint8_t RX_ADDR_P5_LENGTH = 1;
static const uint8_t TX_ADDR_LENGTH    = 5;

enum Command_t : uint8_t {
    Command_R_REGISTER         = 0b00000000,
    Command_W_REGISTER         = 0b00100000,
    Command_R_RX_PAYLOAD       = 0b01100001,
    Command_W_TX_PAYLOAD       = 0b10100000,
    Command_FLUSH_TX           = 0b11100001,
    Command_FLUSH_RX           = 0b11100010,
    Command_REUSE_TX_PL        = 0b11100011,
    Command_R_RX_PL_WID        = 0b01100000,
    Command_W_ACK_PAYLOAD      = 0b10101000,
    Command_W_TX_PAYLOAD_NOACK = 0b10110000,
    Command_NOP                = 0b11111111,
};

enum Register_t : uint8_t {
    Register_CONFIG      = 0x00,
    Register_EN_AA       = 0x01,
    Register_EN_RXADDR   = 0x02,
    Register_SETUP_AW    = 0x03,
    Register_SETUP_RETR  = 0x04,
    Register_RF_CH       = 0x05,
    Register_RF_SETUP    = 0x06,
    Register_STATUS      = 0x07,
    Register_OBSERVE_TX  = 0x08,
    Register_RPD         = 0x09,
    Register_RX_ADDR_P0  = 0x0A,
    Register_RX_ADDR_P1  = 0x0B,
    Register_RX_ADDR_P2  = 0x0C,
    Register_RX_ADDR_P3  = 0x0D,
    Register_RX_ADDR_P4  = 0x0E,
    Register_RX_ADDR_P5  = 0x0F,
    Register_TX_ADDR     = 0x10,
    Register_RX_PW_P0    = 0x11,
    Register_RX_PW_P1    = 0x12,
    Register_RX_PW_P2    = 0x13,
    Register_RX_PW_P3    = 0x14,
    Register_RX_PW_P4    = 0x15,
    Register_RX_PW_P5    = 0x16,
    Register_FIFO_STATUS = 0x17,
    Register_DYNPD       = 0x1C,
    Register_FEATURE     = 0x1D
};

enum CONFIG_t : uint8_t {
    CONFIG_PRIM_RX     = 0,
    CONFIG_PWR_UP      = 1,
    CONFIG_CRCO        = 2,
    CONFIG_EN_CRC      = 3,
    CONFIG_MASK_MAX_RT = 4,
    CONFIG_MASK_TX_DS  = 5,
    CONFIG_MASK_RX_DR  = 6,
};

enum SETUP_AW_t : uint8_t { SETUP_AW = 0, SETUP_AW_MASK = 0b00000011 };

enum SETUP_RETR_t : uint8_t {
    SETUP_RETR_ARC      = 0,
    SETUP_RETR_ARC_MASK = 0b00001111,
    SETUP_RETR_ARD      = 4,
    SETUP_RETR_ARD_MASK = 0b11110000
};

enum RF_CH_t : uint8_t { RF_CH = 0, RF_CH_MASK = 0b01111111 };

enum RF_SETUP_t : uint8_t {
    RF_SETUP_RF_PWR          = 1,
    RF_SETUP_RF_PWR_MASK     = 0b00000110,
    RF_SETUP_RF_DR_HIGH      = 3,
    RF_SETUP_RF_DR_HIGH_MASK = 0b00001000,
    RF_SETUP_PLL_LOCK        = 4,
    RF_SETUP_PLL_LOCK_MASK   = 0b00010000,
    RF_SETUP_RF_DR_LOW       = 5,
    RF_SETUP_RF_DR_LOW_MASK  = 0b00100000,
    RF_SETUP_CONT_WAVE       = 7,
    RF_SETUP_CONT_WAVE_MASK  = 0b10000000
};

enum STATUS_t : uint8_t {
    STATUS_TX_FULL      = 0,
    STATUS_TX_FULL_MASK = 0b00000001,
    STATUS_RX_P_NO      = 1,
    STATUS_RX_P_NO_MASK = 0b00001110,
    STATUS_MAX_RT       = 4,
    STATUS_MAX_RT_MASK  = 0b00010000,
    STATUS_TX_DS        = 5,
    STATUS_TX_DS_MASK   = 0b00100000,
    STATUS_RX_DR        = 6,
    STATUS_RX_DR_MASK   = 0b01000000
};

enum OBSERVE_TX_t : uint8_t {
    OBSERVE_TX_ARC_CNT       = 0,
    OBSERVE_TX_ARC_CNT_MASK  = 0b00001111,
    OBSERVE_TX_PLOS_CNT      = 4,
    OBSERVE_TX_PLOS_CNT_MASK = 0b11110000
};

enum RPD_t : uint8_t { RPD_RPD = 0, RPD_RPD_MASK = 0b00000001 };

enum FIFO_STATUS_t : uint8_t {
    FIFO_STATUS_RX_EMPTY = 0,
    FIFO_STATUS_RX_FULL  = 1,
    FIFO_STATUS_TX_EMPTY = 4,
    FIFO_STATUS_TX_FULL  = 5,
    FIFO_STATUS_TX_REUSE = 6
};

enum FEATURE_t : uint8_t {
    FEATURE_EN_DYN_ACK      = 0,
    FEATURE_EN_DYN_ACK_MASK = 0b00000001,
    FEATURE_EN_ACK_PAY      = 1,
    FEATURE_EN_ACK_PAY_MASK = 0b00000010,
    FEATURE_EN_DPL          = 2,
    FEATURE_EN_DPL_MASK     = 0b00000100
};

#endif  // NRF24L01P_DEFINITIONS_HPP_
