#ifndef __OSTHREAD_H__
#define __OSTHREAD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <dolphin/os/OSContext.h>

typedef struct OSThread      OSThread;
typedef struct OSThreadQueue OSThreadQueue;
typedef struct OSThreadLink  OSThreadLink;
typedef s32                  OSPriority; //  0 highest, 31 lowest

typedef struct OSMutex      OSMutex;
typedef struct OSMutexQueue OSMutexQueue;
typedef struct OSMutexLink  OSMutexLink;
typedef struct OSCond       OSCond;

typedef void (*OSIdleFunction) (void* param);

struct OSThreadQueue
{
    OSThread* head;                      ///< 0x00
    OSThread* tail;                      ///< 0x04
};

struct OSThreadLink
{
    OSThread* next;                      ///< 0x00
    OSThread* prev;                      ///< 0x04
};

struct OSMutexQueue
{
    OSMutex* head;                       ///< 0x00
    OSMutex* tail;                       ///< 0x04
};

struct OSMutexLink
{
    OSMutex* next;                       ///< 0x00
    OSMutex* prev;                       ///< 0x04
};

struct OSThread
{
    OSContext context;                   ///< 0x00 register context

    u16        state;                    ///< 0x2C8 OS_THREAD_STATE_*
    u16        attr;                     ///< 0x2CA OS_THREAD_ATTR_*
    s32        suspend;                  ///< 0x2CC suspended if the count is greater than zero
    OSPriority priority;                 ///< 0x2D0 effective scheduling priority
    OSPriority base;                     ///< 0x2D4 base scheduling priority
    void*      val;                      ///< 0x2D8 exit value

    OSThreadQueue* queue;                ///< 0x2DC queue thread is on
    OSThreadLink   link;                 ///< 0x2E0 queue link

    OSThreadQueue queueJoin; ///< 0x2E8 list of threads waiting for termination (join)

    OSMutex*     mutex;      ///< 0x2F0 mutex trying to lock
    OSMutexQueue queueMutex; ///< 0x2F4 list of mutexes owned

    OSThreadLink linkActive; ///< 0x2FC link of all threads for debugging

    u8*  stackBase;          ///< 0x304 the thread's designated stack (high address)
    u32* stackEnd;           ///< 0x308 last word of stack (low address)
};

// Thread states
enum OS_THREAD_STATE
{
    OS_THREAD_STATE_READY    = 1,
    OS_THREAD_STATE_RUNNING  = 2,
    OS_THREAD_STATE_WAITING  = 4,
    OS_THREAD_STATE_MORIBUND = 8
};

// Thread priorities
#define OS_PRIORITY_MIN       0  // highest
#define OS_PRIORITY_MAX       31 // lowest
#define OS_PRIORITY_IDLE      OS_PRIORITY_MAX

// Thread attributes
#define OS_THREAD_ATTR_DETACH 0x0001u // detached

// Stack magic value
#define OS_THREAD_STACK_MAGIC 0xDEADBABE

void       OSInitThreadQueue (OSThreadQueue* queue);
OSThread*  OSGetCurrentThread (void);
BOOL       OSIsThreadSuspended (OSThread* thread);
BOOL       OSIsThreadTerminated (OSThread* thread);
s32        OSDisableScheduler (void);
s32        OSEnableScheduler (void);
void       OSYieldThread (void);
BOOL       OSCreateThread (OSThread*  thread,
                           void*      (*func) (void*),
                           void*      param,
                           void*      stack,
                           u32        stackSize,
                           OSPriority priority,
                           u16        attr);
void       OSExitThread (void* val);
void       OSCancelThread (OSThread* thread);
BOOL       OSJoinThread (OSThread* thread, void** val);
void       OSDetachThread (OSThread* thread);
s32        OSResumeThread (OSThread* thread);
s32        OSSuspendThread (OSThread* thread);
BOOL       OSSetThreadPriority (OSThread* thread, OSPriority priority);
OSPriority OSGetThreadPriority (OSThread* thread);
void       OSSleepThread (OSThreadQueue* queue);
void       OSWakeupThread (OSThreadQueue* queue);

OSThread* OSSetIdleFunction (OSIdleFunction idleFunction,
                             void*          param,
                             void*          stack,
                             u32            stackSize);
OSThread* OSGetIdleFunction (void);

s32 OSCheckActiveThreads (void);

#ifdef __cplusplus
}
#endif

#endif // __OSTHREAD_H__
