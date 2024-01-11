#include "FiberPool.h"
#include "FiberFlow/FiberFlow.h"

#define INVALID_COHANDLE -1

co_handle SelectNextCoroutine(int32_t index);
int32_t FindFromPool(Fiber_t Handle);

// This store the register part for the thread that init the coroutine pool
static uint8_t Registerpart[AArch64_RegsterPart];
static co_handle active_co = 0;

/* State code for Coroutines */
typedef enum state_code {
    NOT_INIT = 0,
    FREE,
    ACTIVE,
    BlOCKED,
    READY,
} state_t;

/* State code for Locks */
typedef enum Lock_state {
    EMPTY = 0,
    CREATED,
    AQUIRED,
} lock_state_t;

typedef struct Coroutine {
    Fiber_t handle;
    uint16_t priority;
    state_t state;
    // Used to implement lock
    uint32_t index;
    // Stack management
    struct stack_mem stack;
    // Handle for state
    co_handle *co_handle;
} Coroutine_t;

// The implementation of lock for coroutines does not involves atomics
struct CoroutineLock {
    lock_state_t state;
    uint32_t index : 28;
};

// Well, we do not have malloc so I use static memory
static Coroutine_t pool[MAX_COROUTINE_NUM];

static struct CoroutineLock Lock[MAX_LOCK];

int32_t FindFromPool(Fiber_t Handle) {
    for (uint32_t i = 0; i < MAX_COROUTINE_NUM; i++) {
        if (pool[i].handle == Handle) {
            return i;
        }
    }
    return -1;
}

co_handle SelectNextCoroutine(int32_t index) {
    uint32_t i = 1;
    co_handle ret = 0;
    Coroutine_t next;
    next.priority = 0;
    while (i < MAX_COROUTINE_NUM) {
        if (pool[(index + i) % MAX_COROUTINE_NUM].state == READY && 
              pool[(index + i) % MAX_COROUTINE_NUM].priority > next.priority) {
            next = pool[index + i];
            ret = (index + i) % MAX_COROUTINE_NUM;
        }
        i++;
    }
    return ret;
}

void Fiber_yield() {
    pool[active_co].state = READY;
    co_handle ret = SelectNextCoroutine(active_co);
    pool[ret].state = ACTIVE;
    active_co = ret;
    Fiber_switch(pool[ret].handle);
}

void Fiber_block() {
    pool[active_co].state = BlOCKED;
    co_handle ret = SelectNextCoroutine(active_co);
    pool[ret].state = ACTIVE;
    active_co = ret;
    Fiber_switch(pool[ret].handle);
}

void Fiber_kill() {
    pool[active_co].state = FREE;
    co_handle ret = SelectNextCoroutine(active_co);
    // Invalidate the co handle
    *(pool[active_co].co_handle) = INVALID_COHANDLE;
    pool[ret].state = ACTIVE;
    active_co = ret;
    Fiber_switch(pool[ret].handle);
}

int32_t Fiber_wake(co_handle handle) {
    if (handle == INVALID_COHANDLE) {
        return INVALID_COHANDLE;
    }
    pool[handle].state = READY;
    return 0;
}

int32_t FiberPool_init(struct stack_mem* stack, uint32_t num, uint16_t priority) {
    if (num > MAX_COROUTINE_NUM - 1) {
        return -1;
    }
    Fiber_init((Fiber_t)Registerpart);
    pool[0].handle = (Fiber_t)Registerpart;
    pool[0].priority = priority;
    pool[0].state = ACTIVE;
    for (int i = 1; i < num + 1; i++) {
        pool[i].stack = stack[i - 1];
        pool[i].state = FREE;
    }
    return 0;
}

void FiberPool_push(void (* func)(void), void* args, uint16_t priority, co_handle* handle) {
    int32_t i;
    for (i = 0; i < MAX_COROUTINE_NUM; i++) {
        if (pool[i].state == FREE) {
            break;
        }
    }
    if (i == MAX_COROUTINE_NUM) {
        *handle = INVALID_COHANDLE;
        return;
    }
    Fiber_t fiber = Fiber_create(pool[i].stack.memory, pool[i].stack.size, func);
    Fiber_setargs(fiber, args);
    *handle = i;
    pool[i].handle = fiber;
    pool[i].priority = priority;
    pool[i].state = READY;
    pool[i].co_handle = handle;
    return;
}

void* Fiber_GetArgs() {
    Fiber_t temp = Fiber_active();
    return Fiber_getargs(temp);
}