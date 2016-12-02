#ifndef XXX_IGPIO_HPP_
#define XXX_IGPIO_HPP_

namespace xXx {

typedef void (*IGpio_Callback_t)(void *user);

class IGpio {
  public:
    virtual ~IGpio() = default;

    /* General functionality */
    virtual bool read()            = 0;
    virtual void toggle()          = 0;
    virtual void write(bool state) = 0;

    /* Interrupt control */
    virtual void disableInterrupt() = 0;
    virtual void enableInterrupt(IGpio_Callback_t cb, void *user) = 0;
};

} /* namespace xXx */

#endif /* XXX_IGPIO_HPP_ */
