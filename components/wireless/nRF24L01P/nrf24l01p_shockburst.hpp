#ifndef NRF24L01P_SHOCKBURST_HPP_
#define NRF24L01P_SHOCKBURST_HPP_

#include <stdint.h>

#include <xXx/components/wireless/nrf24l01p/nrf24l01p.hpp>
#include <xXx/interfaces/igpio.hpp>
#include <xXx/interfaces/ispi.hpp>
#include <xXx/templates/queue.hpp>
#define LAMBDA []

namespace xXx {

enum class DataRate_t : uint8_t {
    DataRate_1MBPS,
    DataRate_2MBPS,
    DataRate_250KBPS
};

enum class Crc_t : uint8_t { DISABLED, CRC8, CRC16 };

enum class OutputPower_t : uint8_t {
    PowerLevel_18dBm,
    PowerLevel_12dBm,
    PowerLevel_6dBm,
    PowerLevel_0dBm
};

enum class OperatingMode_t : uint8_t { Shutdown, Standby, Rx, Tx };

class nRF24L01P_ShockBurst : public nRF24L01P {
  private:
    IGpio &_ce;
    IGpio &_irq;

    Queue_Handle_t<uint8_t> _rxQueue[6];
    Queue_Handle_t<uint8_t> _txQueue;

    OperatingMode_t _operatingMode;

    uint8_t readShortRegister(Register_t reg);
    void writeShortRegister(Register_t reg, uint8_t regValue);
    void clearSingleBit(Register_t address, uint8_t bitIndex);
    void setSingleBit(Register_t address, uint8_t bitIndex);

    void enterRxMode();
    void enterShutdownMode();
    void enterStandbyMode();
    void enterTxMode();

    uint8_t getPayloadLength();
    void enableDataPipe(uint8_t pipe, bool enable = true);
    void clearInterruptFlags();

    uint8_t getChannel();
    Crc_t getCrcConfig();
    DataRate_t getDataRate();
    // TODO: OutputPower_t getOutputPower();
    // TODO: ??? getRetries();
    uint64_t getRxAddress(uint8_t pipe);
    uint64_t getTxAddress();

  public:
    nRF24L01P_ShockBurst(ISpi &spi, IGpio &ce, IGpio &irq);
    ~nRF24L01P_ShockBurst();

    void configureRxPipe(uint8_t pipe, Queue<uint8_t> &queue,
                         uint64_t address = 0);
    void configureTxPipe(Queue<uint8_t> &queue, uint64_t address = 0);
    void init();
    void switchOperatingMode(OperatingMode_t mode);
    void update();

    void setChannel(uint8_t channel);
    void setCrcConfig(Crc_t crc);
    void setDataRate(DataRate_t dataRate);
    void setOutputPower(OutputPower_t level);
    void setRetries(uint8_t delay, uint8_t count);
    void setRxAddress(uint8_t pipe, uint64_t address);
    void setTxAddress(uint64_t address);
};

} /* namespace xXx */

#endif // NRF24L01P_SHOCKBURST_HPP_
