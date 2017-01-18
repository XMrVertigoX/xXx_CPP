#ifndef NRF24L01P_REGISTER_MAP_HPP_
#define NRF24L01P_REGISTER_MAP_HPP_

#include <stdint.h>

enum class Command_t : uint8_t {
    R_REGISTER         = 0x00,
    W_REGISTER         = 0x20,
    R_RX_PAYLOAD       = 0x61,
    W_TX_PAYLOAD       = 0xA0,
    FLUSH_TX           = 0xE1,
    FLUSH_RX           = 0xE2,
    REUSE_TX_PL        = 0xE3,
    R_RX_PL_WID        = 0x60,
    W_ACK_PAYLOAD      = 0xA8,
    W_TX_PAYLOAD_NOACK = 0xB0,
    NOP                = 0xFF,
};

enum class Register_t : uint8_t {
    CONFIG      = 0x00,
    EN_AA       = 0x01,
    EN_RXADDR   = 0x02,
    SETUP_AW    = 0x03,
    SETUP_RETR  = 0x04,
    RF_CH       = 0x05,
    RF_SETUP    = 0x06,
    STATUS      = 0x07,
    OBSERVE_TX  = 0x08,
    RPD         = 0x09,
    RX_ADDR_P0  = 0x0A,
    RX_ADDR_P1  = 0x0B,
    RX_ADDR_P2  = 0x0C,
    RX_ADDR_P3  = 0x0D,
    RX_ADDR_P4  = 0x0E,
    RX_ADDR_P5  = 0x0F,
    TX_ADDR     = 0x10,
    RX_PW_P0    = 0x11,
    RX_PW_P1    = 0x12,
    RX_PW_P2    = 0x13,
    RX_PW_P3    = 0x14,
    RX_PW_P4    = 0x15,
    RX_PW_P5    = 0x16,
    FIFO_STATUS = 0x17,
    DYNPD       = 0x1C,
    FEATURE     = 0x1D
};

enum class CONFIG_t : uint8_t {
    PRIM_RX             = 0,
    PRIM_RX_DEFAULT     = 0,
    PRIM_RX_MASK        = 0b00000001,
    PWR_UP              = 1,
    PWR_UP_DEFAULT      = 0,
    PWR_UP_MASK         = 0b00000010,
    CRCO                = 2,
    CRCO_DEFAULT        = 0,
    CRCO_MASK           = 0b00000100,
    EN_CRC              = 3,
    EN_CRC_DEFAULT      = 1,
    EN_CRC_MASK         = 0b00001000,
    MASK_MAX_RT         = 4,
    MASK_MAX_RT_DEFAULT = 0,
    MASK_MAX_RT_MASK    = 0b00010000,
    MASK_TX_DS          = 5,
    MASK_TX_DS_DEFAULT  = 0,
    MASK_TX_DS_MASK     = 0b00100000,
    MASK_RX_DR          = 6,
    MASK_RX_DR_DEFAULT  = 0,
    MASK_RX_DR_MASK     = 0b01000000
};

enum class EN_AA_t : uint8_t {
    ENAA_P0 = 0,
    ENAA_P1 = 1,
    ENAA_P2 = 2,
    ENAA_P3 = 3,
    ENAA_P4 = 4,
    ENAA_P5 = 5
};

enum class EN_RXADDR_t : uint8_t {
    ERX_P0 = 0,
    ERX_P1 = 1,
    ERX_P2 = 2,
    ERX_P3 = 3,
    ERX_P4 = 4,
    ERX_P5 = 5
};

enum class SETUP_AW_t : uint8_t {
    AW         = 0,
    AW_DEFAULT = 0b11,
    AW_MASK    = 0b00000011
};

enum class SETUP_RETR_t : uint8_t {
    ARC         = 0,
    ARC_DEFAULT = 3,
    ARC_MASK    = 0b00001111,
    ARD         = 4,
    ARD_DEFAULT = 0,
    ARD_MASK    = 0b11110000
};

enum class RF_CH_t : uint8_t {
    RF_CH         = 0,
    RF_CH_DEFAULT = 2,
    RF_CH_MASK    = 0b01111111
};

enum class RF_SETUP_t : uint8_t {
    RF_PWR             = 1,
    RF_PWR_DEFAULT     = 0b11,
    RF_PWR_MASK        = 0b00000110,
    RF_DR_HIGH         = 3,
    RF_DR_HIGH_DEFAULT = 0b1,
    RF_DR_HIGH_MASK    = 0b00001000,
    PLL_LOCK           = 4,
    PLL_LOCK_DEFAULT   = 0b0,
    PLL_LOCK_MASK      = 0b00010000,
    RF_DR_LOW          = 5,
    RF_DR_LOW_DEFAULT  = 0b0,
    RF_DR_LOW_MASK     = 0b00100000,
    CONT_WAVE          = 7,
    CONT_WAVE_DEFAULT  = 0b0,
    CONT_WAVE_MASK     = 0b10000000
};

enum class STATUS_t : uint8_t {
    TX_FULL      = 0,
    TX_FULL_MASK = 0b00000001,
    RX_P_NO      = 1,
    RX_P_NO_MASK = 0b00001110,
    MAX_RT       = 4,
    MAX_RT_MASK  = 0b00010000,
    TX_DS        = 5,
    TX_DS_MASK   = 0b00100000,
    RX_DR        = 6,
    RX_DR_MASK   = 0b01000000
};

enum class OBSERVE_TX_t : uint8_t {
    ARC_CNT       = 0,
    ARC_CNT_MASK  = 0b00001111,
    PLOS_CNT      = 4,
    PLOS_CNT_MASK = 0b11110000
};

enum class RPD_t : uint8_t {
    RPD         = 0,
    RPD_DEFAULT = 0b0,
    RPD_MASK    = 0b00000001
};

enum class RX_ADDR_P0_t : uint64_t { DEFAULT = 0xE7E7E7E7E7, LENGTH = 5 };

enum class RX_ADDR_P1_t : uint64_t { DEFAULT = 0xC2C2C2C2C2, LENGTH = 5 };

enum class RX_ADDR_P2_t : uint64_t { DEFAULT = 0xC3, LENGTH = 1 };

enum class RX_ADDR_P3_t : uint64_t { DEFAULT = 0xC4, LENGTH = 1 };

enum class RX_ADDR_P4_t : uint64_t { DEFAULT = 0xC5, LENGTH = 1 };

enum class RX_ADDR_P5_t : uint64_t { DEFAULT = 0xC6, LENGTH = 1 };

enum class TX_ADDR_t : uint64_t { DEFAULT = 0xE7E7E7E7E7, LENGTH = 5 };

enum class RX_PW_P0_t : uint8_t { DEFAULT = 0, MASK = 0b0011111 };

enum class RX_PW_P1_t : uint8_t { DEFAULT = 0, MASK = 0b0011111 };

enum class RX_PW_P2_t : uint8_t { DEFAULT = 0, MASK = 0b0011111 };

enum class RX_PW_P3_t : uint8_t { DEFAULT = 0, MASK = 0b0011111 };

enum class RX_PW_P4_t : uint8_t { DEFAULT = 0, MASK = 0b0011111 };

enum class RX_PW_P5_t : uint8_t { DEFAULT = 0, MASK = 0b0011111 };

enum class FIFO_STATUS_t : uint8_t {
    RX_EMPTY = 0,
    RX_FULL  = 1,
    TX_EMPTY = 4,
    TX_FULL  = 5,
    TX_REUSE = 6
};

enum class DYNPD_t : uint8_t {
    DPL_P0 = 0,
    DPL_P1 = 1,
    DPL_P2 = 2,
    DPL_P3 = 3,
    DPL_P4 = 4,
    DPL_P5 = 5
};

enum class FEATURE_t : uint8_t {
    EN_DYN_ACK         = 1,
    EN_DYN_ACK_DEFAULT = 0b0,
    EN_DYN_ACK_MASK    = 0b00000001,
    EN_ACK_PAY         = 2,
    EN_ACK_PAY_DEFAULT = 0b0,
    EN_ACK_PAY_MASK    = 0b00000010,
    EN_DPL             = 3,
    EN_DPL_DEFAULT     = 0b0,
    EN_DPL_MASK        = 0b00000100
};

#endif // NRF24L01P_REGISTER_MAP_HPP_
