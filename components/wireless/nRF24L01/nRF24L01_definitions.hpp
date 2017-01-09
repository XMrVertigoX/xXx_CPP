#ifndef NRF24L01_CONSTANTS_HPP_
#define NRF24L01_CONSTANTS_HPP_

#define STATIC_CAST(x) static_cast<uint8_t>(x)

static const uint8_t maxPayloadSize   = 32;
static const uint8_t maxAddressLength = 5;
static const uint8_t txSettling       = 130;
static const uint8_t rxSettling       = 130;
static const uint8_t dummy            = 0xFF;

// TODO: Search in data sheet
// #define RF24_Command_ACTIVATE  0x50

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

enum Register_t : uint8_t {
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

// TODO
enum class SETUP_AW_t : uint8_t {};

enum class SETUP_RETR_t : uint8_t {
    ARC = 0,
    ARD = 4,
};

// TODO
enum class RF_CH_t : uint8_t {};

// TODO
enum class RF_SETUP_t : uint8_t {};

// TODO
enum class STATUS_t : uint8_t {};

// TODO
enum class OBSERVE_TX_t : uint8_t {};

// TODO
enum class RPD_t : uint8_t {};

// clang-format off

/* Bit Mnemonics */
#define PLL_LOCK      (4)
#define RF_DR         (3)
#define RF_PWR_MASK   (6)
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

/* P model bit Mnemonics */
#define RF_DR_LOW     (5)
#define RF_DR_HIGH    (3)
#define RF_PWR_LOW    (1)
#define RF_PWR_HIGH   (2)

// clang-format on

#endif // NRF24L01_CONSTANTS_HPP_
