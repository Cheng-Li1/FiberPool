#include "FiberPool.h"
#include <stdint.h>

#define Invalid_Handle -1

uint32_t SelectNextCoroutine(int32_t index);
int32_t FindFromPool(Fiber_t Handle);

// This store the register part for the thread that init the coroutine pool
static uint8_t Registerpart[AArch64_RegsterPart];
static uint32_t active_co = 0;

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

uint32_t SelectNextCoroutine(int32_t index) {
    uint32_t i = 1, ret = 0;
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

void yield() {
    pool[active_co].state = READY;
    uint32_t ret = SelectNextCoroutine(active_co);
    pool[ret].state = ACTIVE;
    active_co = ret;
    Fiber_switch(pool[ret].handle);
}

void block() {
    pool[active_co].state = BlOCKED;
    uint32_t ret = SelectNextCoroutine(active_co);
    pool[ret].state = ACTIVE;
    active_co = ret;
    Fiber_switch(pool[ret].handle);
}

void kill() {
    pool[active_co].state = FREE;
    uint32_t ret = SelectNextCoroutine(active_co);
    pool[ret].state = ACTIVE;
    active_co = ret;
    // Invalidate the co handler
    *(pool[active_co].co_handle) = Invalid_Handle;
    Fiber_switch(pool[ret].handle);
}

int32_t wake(co_handle handle) {
    if (handle == Invalid_Handle) {
        return Invalid_Handle;
    }
    pool[handle].state = READY;
    return 0;
}

int32_t init(struct stack_mem* stack, uint32_t num, uint16_t priority) {
    if (num > MAX_COROUTINE_NUM) {
        return -1;
    }
    Fiber_init((Fiber_t)Registerpart);
    pool[0].handle = (Fiber_t)Registerpart;
    pool[0].priority = priority;
    pool[0].state = READY;
    for (int i = 1; i < num + 1; i++) {
        pool[i].stack = stack[i - 1];
        pool[i].state = FREE;
    }
    return 0;
}

void push(void (* func)(void), void* args, uint16_t priority, co_handle* handle) {
    int32_t i;
    for (i = 0; i < MAX_COROUTINE_NUM; i++) {
        if (pool[i].state == FREE) {
            break;
        }
    }
    if (i == MAX_COROUTINE_NUM) {
        *handle = Invalid_Handle;
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