#ifndef RF24_TYPES_HPP
#define RF24_TYPES_HPP

#include <stdint.h>

#define txFifoSize (32)
#define rxFifoSize (32)
#define txSettling (130)
#define rxSettling (130)

struct RF24_DataPackage_t {
    uint8_t bytes[32];
    uint8_t numBytes;
    uint8_t pipe;
};

// typedef void (*RF24_TxCallback_t)(void *user);
typedef void (*RF24_RxCallback_t)(RF24_DataPackage_t data, void *user);

enum class RF24_DataRate : uint8_t
{
    DR_250KBPS,
    DR_1MBPS,
    DR_2MBPS
};

enum class RF24_CRCConfig : uint8_t
{
    CRC_DISABLED,
    CRC_1Byte,
    CRC_2Bytes
};

enum class RF24_OutputPower : uint8_t
{
    PWR_18dBm,
    PWR_12dBm,
    PWR_6dBm,
    PWR_0dBm
};

enum class RF24_Status : uint8_t
{
    Success,
    Failure,
    UnknownPipe,
    UnknownChannel,
    VerificationFailed
};

enum class RF24_Command : uint8_t
{
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
    NOP                = 0b11111111
};

enum class RF24_Register : uint8_t
{
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

enum CONFIG : uint8_t
{
    CONFIG_PRIM_RX     = 0,
    CONFIG_PWR_UP      = 1,
    CONFIG_CRCO        = 2,
    CONFIG_EN_CRC      = 3,
    CONFIG_MASK_MAX_RT = 4,
    CONFIG_MASK_TX_DS  = 5,
    CONFIG_MASK_RX_DR  = 6
};

enum SETUP_AW : uint8_t
{
    SETUP_AW      = 0,
    SETUP_AW_MASK = 0b00000011
};

enum SETUP_RETR : uint8_t
{
    SETUP_RETR_ARC      = 0,
    SETUP_RETR_ARC_MASK = 0b00001111,
    SETUP_RETR_ARD      = 4,
    SETUP_RETR_ARD_MASK = 0b11110000
};

enum RF_CH : uint8_t
{
    RF_CH      = 0,
    RF_CH_MASK = 0b01111111
};

enum RF_SETUP : uint8_t
{
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

enum STATUS : uint8_t
{
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

enum OBSERVE_TX : uint8_t
{
    OBSERVE_TX_ARC_CNT       = 0,
    OBSERVE_TX_ARC_CNT_MASK  = 0b00001111,
    OBSERVE_TX_PLOS_CNT      = 4,
    OBSERVE_TX_PLOS_CNT_MASK = 0b11110000
};

enum RPD : uint8_t
{
    RPD_RPD      = 0,
    RPD_RPD_MASK = 0b00000001
};

enum FIFO_STATUS : uint8_t
{
    FIFO_STATUS_RX_EMPTY = 0,
    FIFO_STATUS_RX_FULL  = 1,
    FIFO_STATUS_TX_EMPTY = 4,
    FIFO_STATUS_TX_FULL  = 5,
    FIFO_STATUS_TX_REUSE = 6
};

enum FEATURE : uint8_t
{
    FEATURE_EN_DYN_ACK      = 0,
    FEATURE_EN_DYN_ACK_MASK = 0b00000001,
    FEATURE_EN_ACK_PAY      = 1,
    FEATURE_EN_ACK_PAY_MASK = 0b00000010,
    FEATURE_EN_DPL          = 2,
    FEATURE_EN_DPL_MASK     = 0b00000100
};

#endif  // RF24_TYPES_HPP
