#include <assert.h>

#include <FreeRTOS.h>

void *operator new(size_t s) {
    void *p = pvPortMalloc(s);

#if defined(NDEBUG)
#error "'new' operator may return NULL!"
#else
    assert(p != NULL);
#endif

    return (p);
}

void *operator new[](size_t s) {
    void *p = pvPortMalloc(s);

#if defined(NDEBUG)
#error "'new' operator may return NULL!"
#else
    assert(p != NULL);
#endif

    return (p);
}

void operator delete(void *p) {
    vPortFree(p);
}

void operator delete(void *p, size_t s) {
    operator delete(p);
}

void operator delete[](void *p) {
    vPortFree(p);
}

void operator delete[](void *p, size_t s) {
    operator delete[](p);
}
