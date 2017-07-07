#ifndef SI1145_HPP_
#define SI1145_HPP_

#include <stdint.h>

#include <xXx/interfaces/itwi.hpp>

namespace xXx {

class SI1145 {
   private:
    ITwi &_twi;
    uint8_t _address;

   public:
    SI1145(ITwi &twi, uint8_t address);
    ~SI1145();
    void init();
};

} /* namespace xXx */

#endif /* SI1145_HPP_ */
