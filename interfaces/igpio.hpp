#ifndef IGPIO_HPP
#define IGPIO_HPP

namespace xXx {

typedef void (*IGpio_Callback_t)(void *user);

class IGpio {
   public:
    /* General functionality */
    virtual void clear()  = 0;
    virtual bool get()    = 0;
    virtual void set()    = 0;
    virtual void toggle() = 0;

    /* Interrupt control */
    virtual void disableInterrupt()                               = 0;
    virtual void enableInterrupt(IGpio_Callback_t cb, void *user) = 0;
};

} /* namespace xXx */

#endif /* IGPIO_HPP */
