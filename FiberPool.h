#define MAX_COROUTINE_NUM 5
#define MAX_LOCK 16

#include <stdint.h>

typedef struct CoroutineLock* mutex_t;
typedef int32_t co_handle;

struct stack_mem {
    void* memory;
    uint64_t size;
};

int32_t FiberPool_init(struct stack_mem* stack, uint32_t num, uint16_t priority);

int32_t Fiber_wake(co_handle handle);

// Giving control to other coroutine
void Fiber_yield();

void Fiber_block();

// Declare this coroutine is finished
void Fiber_kill();

void* Fiber_GetArgs();

// Push a fiber into the coroutine pool
// handle would be able to used to wake the blocked coroutine
// When a coroutine ends, the handle would be set to a negative value
void FiberPool_push(void (* func)(void), void* args, uint16_t priority, co_handle* handle);

// Mutex not implemented yet
mutex_t create_mutex();

// Does not need a return value
void acquire_mutex(mutex_t);

void release_mutex(mutex_t);

void destroy_mutex(mutex_t);