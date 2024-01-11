#define MAX_COROUTINE_NUM 5
#define MAX_LOCK 16

#include "FiberFlow/FiberFlow.h"
#include <stdint.h>

typedef struct CoroutineLock* mutex_t;
typedef int32_t co_handle;

struct stack_mem {
    void* memory;
    uint64_t size;
};

int32_t init(struct stack_mem* stack, uint32_t num, uint16_t priority);

void wake(co_handle index);

// Giving control to other coroutine
void yield();

void block();

// Declare this coroutine is finished
void kill();

// Push a fiber into the coroutine pool
// Return < 0 if there is no space
void push(void (* func)(void), void* args, uint16_t priority, co_handle* handle);

mutex_t create_mutex();

// Does not need a return value
void acquire_mutex(mutex_t);

void release_mutex(mutex_t);

void destroy_mutex(mutex_t);