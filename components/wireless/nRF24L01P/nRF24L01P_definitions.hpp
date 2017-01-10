#ifndef NRF24L01P_DEFINITIONS_HPP_
#define NRF24L01P_DEFINITIONS_HPP_

#include <stdint.h>

#define VALUE(x) static_cast<uint8_t>(x)
#define LAMBDA []

static const uint8_t maxPayloadSize    = 32;
static const uint8_t txSettling        = 130;
static const uint8_t rxSettling        = 130;
static const uint8_t dummy             = 0xFF;
static const uint64_t longAddressMask  = 0xFFFFFFFFFF;
static const uint64_t shortAddressMask = 0xFF;
static const uint8_t maxChannel        = 0x7F;

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
    PRIM_RX,
    PWR_UP,
    CRCO,
    EN_CRC,
    MASK_MAX_RT,
    MASK_TX_DS,
    MASK_RX_DR
};

enum class EN_AA_t : uint8_t {
    ENAA_P0,
    ENAA_P1,
    ENAA_P2,
    ENAA_P3,
    ENAA_P4,
    ENAA_P5
};

enum class EN_RXADDR_t : uint8_t {
    ERX_P0,
    ERX_P1,
    ERX_P2,
    ERX_P3,
    ERX_P4,
    ERX_P5
};

enum class SETUP_AW_t : uint8_t { AW };

enum class SETUP_RETR_t : uint8_t { ARC = 0, ARD = 4 };

enum class RF_CH_t : uint8_t { RF_CH };

enum class RF_SETUP_t : uint8_t {
    RF_PWR     = 1,
    RF_DR_HIGH = 3,
    PLL_LOCK   = 4,
    RF_DR_LOW  = 5,
    CONT_WAVE  = 7
};

enum class STATUS_t : uint8_t {
    TX_FULL = 0,
    RX_P_NO = 1,
    MAX_RT  = 4,
    TX_DS   = 5,
    RX_DR   = 6
};

enum class OBSERVE_TX_t : uint8_t { ARC_CNT = 0, PLOS_CNT = 4 };

enum class RPD_t : uint8_t { RPD };

enum class RX_PW_P0_t : uint8_t { RX_PW_P0 };

enum class RX_PW_P1_t : uint8_t { RX_PW_P1 };

enum class RX_PW_P2_t : uint8_t { RX_PW_P2 };

enum class RX_PW_P3_t : uint8_t { RX_PW_P3 };

enum class RX_PW_P4_t : uint8_t { RX_PW_P4 };

enum class RX_PW_P5_t : uint8_t { RX_PW_P5 };

enum class FIFO_STATUS : uint8_t {
    RX_EMPTY = 0,
    RX_FULL  = 1,
    TX_EMPTY = 4,
    TX_FULL  = 5,
    TX_REUSE = 6
};

enum class DYNPD : uint8_t { DPL_P0, DPL_P1, DPL_P2, DPL_P3, DPL_P4, DPL_P5 };

enum class FEATURE : uint8_t { EN_DYN_ACK, EN_ACK_PAY, EN_DPL };

#endif // NRF24L01P_DEFINITIONS_HPP_
