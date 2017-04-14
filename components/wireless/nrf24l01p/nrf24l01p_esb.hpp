#if not defined(NRF24L01P_ESB_HPP_)
#define NRF24L01P_ESB_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p_base.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/os/simpletask.hpp>
#include <xXx/templates/queue.hpp>

static const uint8_t txFifoSize = 32;
static const uint8_t rxFifoSize = 32;

enum DataRate_t : uint8_t { DataRate_1MBPS, DataRate_2MBPS, DataRate_250KBPS };

enum CRCConfig_t : uint8_t { CRCConfig_DISABLED, CRCConfig_1Byte, CrcConfig_2Bytes };

enum OutputPower_t : uint8_t {
    OutputPower_18dBm,
    OutputPower_12dBm,
    OutputPower_6dBm,
    OutputPower_0dBm
};

enum OperatingMode_t : uint8_t {
    OperatingMode_Shutdown,
    OperatingMode_Standby,
    OperatingMode_Rx,
    OperatingMode_Tx
};

struct Package_t {
    uint8_t bytes[rxFifoSize];
    uint8_t numBytes;
};

namespace xXx {

typedef void (*txCallback_t)(int8_t status, void *user);

class nRF24L01P_ESB : public nRF24L01P_BASE, public SimpleTask {
   private:
    IGpio &_ce;
    IGpio &_irq;

    Queue_Handle_t<Package_t> _rxQueue[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

    uint8_t *_txBytes        = NULL;
    size_t _txBytesStart     = 0;
    size_t _txBytesEnd       = 0;
    txCallback_t _txCallback = NULL;
    void *_txUser            = NULL;

    uint8_t readShortRegister(Register_t reg);
    void writeShortRegister(Register_t reg, uint8_t regValue);
    void clearSingleBit(Register_t reg, uint8_t bitIndex);
    void setSingleBit(Register_t reg, uint8_t bitIndex);

    void setup();
    void loop();

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    void handle_MAX_RT();
    void handle_RX_DR();
    void handle_TX_DS();

    int8_t readRxFifo();
    int8_t writeTxFifo();

    void enableDataPipe(uint8_t pipeIndex);
    void disableDataPipe(uint8_t pipeIndex);

   public:
    nRF24L01P_ESB(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ESB();

    void configureTxPipe(uint64_t address);
    void configureRxPipe(uint8_t pipe, Queue<Package_t> &rxQueue, uint64_t address);
    void switchOperatingMode(OperatingMode_t mode);

    int8_t send(uint8_t bytes[], size_t numBytes, txCallback_t callback, void *user);

    int8_t getChannel();
    void setChannel(int8_t channel);
    CRCConfig_t getCrcConfig();
    void setCrcConfig(CRCConfig_t crc);
    void setDataRate(DataRate_t dataRate);
    DataRate_t getDataRate();
    OutputPower_t getOutputPower();
    void setOutputPower(OutputPower_t level);
    uint8_t getRetryCount();
    void setRetryCount(uint8_t count);
    uint8_t getRetryDelay();
    void setRetryDelay(uint8_t delay);
    int64_t getRxAddress(uint8_t pipe);
    void setRxAddress(uint8_t pipe, int64_t address);
    int64_t getTxAddress();
    void setTxAddress(int64_t address);
};

} /* namespace xXx */

#endif  // NRF24L01P_ESB_HPP_
