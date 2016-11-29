#ifndef SI1145_HPP_
#define SI1145_HPP_

#include <stdint.h>

#include <xXx/interfaces/itwi.hpp>

namespace xXx {

class SI1145 {
  private:
    ITwi &_twi;

  public:
    SI1145(ITwi &twi);
    ~SI1145();
};

} /* namespace xXx */

#endif /* SI1145_HPP_ */
