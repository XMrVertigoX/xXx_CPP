#include <assert.h>

#include <FreeRTOS.h>

#ifdef __UNUSED
#undef __UNUSED
#endif

#define __UNUSED(x) ((void)x)

void *operator new(size_t s) {
    void *p = pvPortMalloc(s);

#ifdef NDEBUG
#warning "'new' operator may return NULL!"
#else
    assert(p not_eq NULL);
#endif

    return (p);
}

void *operator new[](size_t s) {
    void *p = pvPortMalloc(s);

#ifdef NDEBUG
#warning "'new[]' operator may return NULL!"
#else
    assert(p not_eq NULL);
#endif

    return (p);
}

void operator delete(void *p) {
    vPortFree(p);
}

void operator delete(void *p, size_t s) {
    __UNUSED(s);

    operator delete(p);
}

void operator delete[](void *p) {
    vPortFree(p);
}

void operator delete[](void *p, size_t s) {
    __UNUSED(s);

    operator delete[](p);
}
