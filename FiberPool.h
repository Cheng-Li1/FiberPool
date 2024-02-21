#define MAX_COROUTINE_NUM 5
#define MAX_LOCK 16
#define INVALID_COHANDLE -1

#include <stdint.h>

typedef struct CoroutineLock* mutex_t;
typedef int32_t co_handle;

struct stack_mem {
    void* memory;
    uint64_t size;
};

int32_t FiberPool_init(struct stack_mem* stack, uint32_t num, uint16_t priority);

int32_t Fiber_wake(co_handle handle);

co_handle Get_Cohandle();

// Giving control to other coroutine
void Fiber_yield();

void Fiber_block();

// Declare this coroutine is finished
/* Kill the main thread may cause undefined bahavior */
void Fiber_kill();

co_handle Fiber_next();

void* Fiber_GetArgs();

// Push a fiber into the coroutine pool
// handle would be able to used to wake the blocked coroutine
// When a coroutine ends, the handle would be set to a negative value
int32_t FiberPool_push(void (* func)(void), void* args, uint16_t priority, co_handle* handle);

// Return the index of the next free coroutine in the pool
int32_t FiberPool_FindFree();

// Function to set args for a coroutine
void FiberPool_SetArgs(co_handle handle, void* data);

// Mutex not implemented yet
mutex_t create_mutex();

// Does not need a return value
void acquire_mutex(mutex_t);

void release_mutex(mutex_t);

void destroy_mutex(mutex_t);

/* simple usage example
#include "FiberPool/FiberPool.h"
#include <stdlib.h>
#include <stdio.h>

char* string = "I am a running coroutine!\n";

void print_stuff1() {
    printf("Now I yield\n");
    Fiber_yield();
    printf("Well I am back again, gonna kill myself\n");
    Fiber_kill();
}

void print_stuff2() {
    printf("Get my args first!\n");
    char* p = Fiber_GetArgs();
    printf("My args is %s\n", p);
    printf("Try to block myself\n");
    Fiber_block();
    printf("I am waken\n");
    Fiber_kill();
}

int main() {
    void* stack1 = malloc(4096);
    void* stack2 = malloc(4096);
    void* stack3 = malloc(4096);
    void* stack4 = malloc(4096);
    struct stack_mem stackarray[4];
    stackarray[0].memory = stack1;
    stackarray[0].size = 4096;
    stackarray[1].memory = stack2;
    stackarray[1].size = 4096;
    stackarray[2].memory = stack3;
    stackarray[2].size = 4096;
    stackarray[3].memory = stack4;
    stackarray[3].size = 4096;
    FiberPool_init(stackarray, 4, 1);
    co_handle f, f2;
    FiberPool_push(print_stuff1, NULL, 2, &f);
    FiberPool_push(print_stuff2, string, 2, &f2);
    Fiber_yield();
    Fiber_wake(f2);
    Fiber_yield();
    printf("Well, get control again!\n");
    return 0;
}

*/