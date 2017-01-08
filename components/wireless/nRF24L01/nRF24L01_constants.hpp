#ifndef NRF24L01_CONSTANTS_HPP_
#define NRF24L01_CONSTANTS_HPP_

static const uint8_t maxPayloadSize   = 32;
static const uint8_t maxAddressLength = 5;
static const uint8_t txSettling       = 130;
static const uint8_t rxSettling       = 130;

// TODO: Search in data sheet
// #define RF24_Command_ACTIVATE  0x50

enum class nRF24L01_Command_t : uint8_t {
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

enum class nRF24L01_RegisterMap_t : uint8_t {
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

enum class nRF24L01_DataRate_t : uint8_t {
    SPEED_1MBPS,
    SPEED_2MBPS,
    SPEED_250KBPS
};

enum RF24_CRC_t { RF24_CRC_DISABLED, RF24_CRC_8, RF24_CRC_16 };

enum RF24_PowerLevel_t {
    RF24_PA_18dBm,
    RF24_PA_12dBm,
    RF24_PA_6dBm,
    RF24_PA_0dBm
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

enum RF24_SETUP_AW_t {
    RF24_SETUP_AW,
};

enum RF24_SETUP_RETR_t {
    RF24_SETUP_RETR_ARC = 0,
    RF24_SETUP_RETR_ARD = 4,
};

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

/* Non-P omissions */
// #define LNA_HCURR     (0)

/* P model bit Mnemonics */
#define RF_DR_LOW     (5)
#define RF_DR_HIGH    (3)
#define RF_PWR_LOW    (1)
#define RF_PWR_HIGH   (2)

// clang-format on

enum class nRF24L01_CONFIG_t : uint8_t {
    PRIM_RX     = 0,
    PWR_UP      = 1,
    CRCO        = 2,
    EN_CRC      = 3,
    MASK_MAX_RT = 4,
    MASK_TX_DS  = 5,
    MASK_RX_DR  = 6
};

enum class nRF24L01_EN_AA_t : uint8_t {
    ENAA_P0 = 0,
    ENAA_P1 = 1,
    ENAA_P2 = 2,
    ENAA_P3 = 3,
    ENAA_P4 = 4,
    ENAA_P5 = 5
};

enum class nRF24L01_EN_RXADDR_t : uint8_t {
    ERX_P0 = 0,
    ERX_P1 = 1,
    ERX_P2 = 2,
    ERX_P3 = 3,
    ERX_P4 = 4,
    ERX_P5 = 5
};

// enum class nRF24L01_SETUP_AW_t : uint8_t { AW = 0 };
//
// enum class nRF24L01_SETUP_RETR_t : uint8_t { ARC = 0, ARD = 4 };
//
// enum class nRF24L01_RF_CH_t : uint8_t { RF_CH = 0 };
//
// enum class nRF24L01_RF_SETUP_t : uint8_t {
//     RF_PWR     = 1,
//     RF_DR_HIGH = 3,
//     PLL_LOCK   = 4,
//     RF_DR_LOW  = 5,
//     CONT_WAVE  = 7
// };
//
// enum class nRF24L01_STATUS_t : uint8_t {
//     TX_FULL = 0,
//     RX_P_NO = 1,
//     MAX_RT  = 4,
//     TX_DS   = 5,
//     RX_DR   = 6
// };
//
// enum class nRF24L01_OBSERVE_TX__t : uint8_t { ARC_CNT = 0, PLOS_CNT = 4 };
//
// enum class nRF24L01_RPD_t : uint8_t { RPD = 0 };
//
// enum class nRF24L01_RX_PW_P0_t : uint8_t { RX_PW_P0 = 0 };
//
// enum class nRF24L01_RX_PW_P1_t : uint8_t { RX_PW_P1 = 0 };
//
// enum class nRF24L01_RX_PW_P2_t : uint8_t { RX_PW_P2 = 0 };
//
// enum class nRF24L01_RX_PW_P3_t : uint8_t { RX_PW_P3 = 0 };
//
// enum class nRF24L01_RX_PW_P4_t : uint8_t { RX_PW_P4 = 0 };
//
// enum class nRF24L01_RX_PW_P5_t : uint8_t { RX_PW_P5 = 0 };
//
// enum class nRF24L01_FIFO_STATUS_t : uint8_t {
//     RX_EMPTY = 0,
//     RX_FULL  = 1,
//     TX_EMPTY = 4,
//     TX_FULL  = 5,
//     TX_REUSE = 6
// };
//
// enum class nRF24L01_DYNPD_t : uint8_t {
//     DPL_P0 = 0,
//     DPL_P1 = 1,
//     DPL_P2 = 2,
//     DPL_P3 = 3,
//     DPL_P4 = 4,
//     DPL_P5 = 5
// };
//
// enum class nRF24L01_FEATURE_t : uint8_t {
//     EN_DYN_ACK = 0,
//     EN_ACK_PAY = 1,
//     EN_DPL     = 2
// };

#endif // NRF24L01_CONSTANTS_HPP_
