#ifndef NRF24L01P_DEFINITIONS_HPP_
#define NRF24L01P_DEFINITIONS_HPP_

#include <stdint.h>

enum class Command_t : uint8_t {
    R_REGISTER         = 0b00000000,
    W_REGISTER         = 0b00100000,
    R_RX_PAYLOAD       = 0b01100001,
    W_TX_PAYLOAD       = 0b10100000,
    FLUSH_TX           = 0b11100001,
    FLUSH_RX           = 0b11100010,
    REUSE_TX_PL        = 0b11100011,
    R_RX_PL_WID        = 0b01100000,
    W_ACK_PAYLOAD      = 0b10101000,
    W_TX_PAYLOAD_NOACK = 0b10110000,
    NOP                = 0b11111111,
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

enum CONFIG_t : uint8_t {
    CONFIG_PRIM_RX             = 0,
    CONFIG_PRIM_RX_DEFAULT     = 0,
    CONFIG_PRIM_RX_MASK        = 0b00000001,
    CONFIG_PWR_UP              = 1,
    CONFIG_PWR_UP_DEFAULT      = 0,
    CONFIG_PWR_UP_MASK         = 0b00000010,
    CONFIG_CRCO                = 2,
    CONFIG_CRCO_DEFAULT        = 0,
    CONFIG_CRCO_MASK           = 0b00000100,
    CONFIG_EN_CRC              = 3,
    CONFIG_EN_CRC_DEFAULT      = 1,
    CONFIG_EN_CRC_MASK         = 0b00001000,
    CONFIG_MASK_MAX_RT         = 4,
    CONFIG_MASK_MAX_RT_DEFAULT = 0,
    CONFIG_MASK_MAX_RT_MASK    = 0b00010000,
    CONFIG_MASK_TX_DS          = 5,
    CONFIG_MASK_TX_DS_DEFAULT  = 0,
    CONFIG_MASK_TX_DS_MASK     = 0b00100000,
    CONFIG_MASK_RX_DR          = 6,
    CONFIG_MASK_RX_DR_DEFAULT  = 0,
    CONFIG_MASK_RX_DR_MASK     = 0b01000000
};

enum EN_AA_t : uint8_t {
    ENAA_P0 = 0,
    ENAA_P1 = 1,
    ENAA_P2 = 2,
    ENAA_P3 = 3,
    ENAA_P4 = 4,
    ENAA_P5 = 5
};

enum EN_RXADDR_t : uint8_t {
    ERX_P0 = 0,
    ERX_P1 = 1,
    ERX_P2 = 2,
    ERX_P3 = 3,
    ERX_P4 = 4,
    ERX_P5 = 5
};

enum SETUP_AW_t : uint8_t { SETUP_AW = 0, SETUP_AW_DEFAULT = 0b11, SETUP_AW_MASK = 0b00000011 };

enum SETUP_RETR_t : uint8_t {
    SETUP_RETR_ARC         = 0,
    SETUP_RETR_ARC_MASK    = 0b00001111,
    SETUP_RETR_ARD         = 4,
    SETUP_RETR_ARD_MASK    = 0b11110000
};

enum RF_CH_t : uint8_t { RF_CH = 0, RF_CH_DEFAULT = 2, RF_CH_MASK = 0b01111111 };

enum RF_SETUP_t : uint8_t {
    RF_SETUP_RF_PWR             = 1,
    RF_SETUP_RF_PWR_DEFAULT     = 0b11,
    RF_SETUP_RF_PWR_MASK        = 0b00000110,
    RF_SETUP_RF_DR_HIGH         = 3,
    RF_SETUP_RF_DR_HIGH_DEFAULT = 0b1,
    RF_SETUP_RF_DR_HIGH_MASK    = 0b00001000,
    RF_SETUP_PLL_LOCK           = 4,
    RF_SETUP_PLL_LOCK_DEFAULT   = 0b0,
    RF_SETUP_PLL_LOCK_MASK      = 0b00010000,
    RF_SETUP_RF_DR_LOW          = 5,
    RF_SETUP_RF_DR_LOW_DEFAULT  = 0b0,
    RF_SETUP_RF_DR_LOW_MASK     = 0b00100000,
    RF_SETUP_CONT_WAVE          = 7,
    RF_SETUP_CONT_WAVE_DEFAULT  = 0b0,
    RF_SETUP_CONT_WAVE_MASK     = 0b10000000
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

enum RPD_t : uint8_t { RPD_RPD = 0, RPD_RPD_DEFAULT = 0b0, RPD_RPD_MASK = 0b00000001 };

enum RX_ADDR_P0_t : uint64_t {
    RX_ADDR_P0_DEFAULT = 0x000000E7E7E7E7E7,
    RX_ADDR_P0_MASK    = 0x000000FFFFFFFFFF,
    RX_ADDR_P0_LENGTH  = 5
};

enum RX_ADDR_P1_t : uint64_t {
    RX_ADDR_P1_DEFAULT = 0x000000C2C2C2C2C2,
    RX_ADDR_P1_MASK    = 0x000000FFFFFFFFFF,
    RX_ADDR_P1_LENGTH  = 5
};

enum RX_ADDR_P2_t : uint64_t {
    RX_ADDR_P2_DEFAULT = 0x00000000000000C3,
    RX_ADDR_P2_MASK    = 0x00000000000000FF,
    RX_ADDR_P2_LENGTH  = 1
};

enum RX_ADDR_P3_t : uint64_t {
    RX_ADDR_P3_DEFAULT = 0x00000000000000C4,
    RX_ADDR_P3_MASK    = 0x00000000000000FF,
    RX_ADDR_P3_LENGTH  = 1
};

enum RX_ADDR_P4_t : uint64_t {
    RX_ADDR_P4_DEFAULT = 0x00000000000000C5,
    RX_ADDR_P4_MASK    = 0x00000000000000FF,
    RX_ADDR_P4_LENGTH  = 1
};

enum RX_ADDR_P5_t : uint64_t {
    RX_ADDR_P5_DEFAULT = 0x00000000000000C6,
    RX_ADDR_P5_MASK    = 0x00000000000000FF,
    RX_ADDR_P5_LENGTH  = 1
};

enum TX_ADDR_t : uint64_t {
    TX_ADDR_DEFAULT = 0x000000E7E7E7E7E7,
    TX_ADDR_MASK    = 0x000000FFFFFFFFFF,
    TX_ADDR_LENGTH  = 5
};

enum RX_PW_P0_t : uint8_t { RX_PW_P0_DEFAULT = 0, RX_PW_P0_MASK = 0b00011111 };

enum RX_PW_P1_t : uint8_t { RX_PW_P1_DEFAULT = 0, RX_PW_P1_MASK = 0b00011111 };

enum RX_PW_P2_t : uint8_t { RX_PW_P2_DEFAULT = 0, RX_PW_P2_MASK = 0b00011111 };

enum RX_PW_P3_t : uint8_t { RX_PW_P3_DEFAULT = 0, RX_PW_P3_MASK = 0b00011111 };

enum RX_PW_P4_t : uint8_t { RX_PW_P4_DEFAULT = 0, RX_PW_P4_MASK = 0b00011111 };

enum RX_PW_P5_t : uint8_t { RX_PW_P5_DEFAULT = 0, RX_PW_P5_MASK = 0b00011111 };

enum FIFO_STATUS_t : uint8_t {
    FIFO_STATUS_RX_EMPTY = 0,
    FIFO_STATUS_RX_FULL  = 1,
    FIFO_STATUS_TX_EMPTY = 4,
    FIFO_STATUS_TX_FULL  = 5,
    FIFO_STATUS_TX_REUSE = 6
};

enum DYNPD_t : uint8_t {
    DYNPD_DPL_P0 = 0,
    DYNPD_DPL_P1 = 1,
    DYNPD_DPL_P2 = 2,
    DYNPD_DPL_P3 = 3,
    DYNPD_DPL_P4 = 4,
    DYNPD_DPL_P5 = 5
};

enum FEATURE_t : uint8_t {
    FEATURE_EN_DYN_ACK         = 0,
    FEATURE_EN_DYN_ACK_DEFAULT = 0,
    FEATURE_EN_DYN_ACK_MASK    = 0b00000001,
    FEATURE_EN_ACK_PAY         = 1,
    FEATURE_EN_ACK_PAY_DEFAULT = 0,
    FEATURE_EN_ACK_PAY_MASK    = 0b00000010,
    FEATURE_EN_DPL             = 2,
    FEATURE_EN_DPL_DEFAULT     = 0,
    FEATURE_EN_DPL_MASK        = 0b00000100
};

#endif // NRF24L01P_DEFINITIONS_HPP_
