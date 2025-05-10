#include <dolphin/os.h>

#include <stdlib.h>

#include "OSPrivate.h"

#ifdef __cplusplus
extern "C"
{
#endif

DECL_SECT (".ctors") extern void (*_ctors[])(); // size: 0x0, address: 0x0
DECL_SECT (".dtors") extern void (*_dtors[])(); // size: 0x0, address: 0x0

DECL_SECT (".init")

asm void
__init_hardware (void)
{
#ifdef __MWERKS__
    nofralloc;
    mfmsr r0;
    ori   r0, r0, MSR_FP;
    mtmsr r0;
    mflr  r31;
    bl    __OSPSInit;
    bl    __OSCacheInit;
    mtlr  r31;
    blr;
#endif
}
DECL_SECT (".init")

asm void
__flush_cache (void* address, u32 size)
{
#ifdef __MWERKS__
    nofralloc;
    lis r5, 0xffff;
    ori r5, r5, 0xfff1;
    and r5, r5, r3;
    subf r3, r5, r3;
    add  r4, r4, r3;

rept:
    dcbst 0, r5;
    sync;
    icbi 0, r5;
    addic r5, r5, 0x8;
    subic.r4, r4, 0x8;
    bge rept;
    isync;
    blr;
#endif
}

void
__init_user (void)
{
    __init_cpp();
}

void
__init_cpp (void)
{
#ifdef __MWERKS__
    void (**constructor)();

    /*
     *	call static initializers
     */
    for (constructor = _ctors; *constructor; constructor++) { (*constructor)(); }
#endif
}

void
__fini_cpp (void)
{
#ifdef __MWERKS__
    void (**destructor)();

    /*
     *	call destructors
     */
    for (destructor = _dtors; *destructor; destructor++) { (*destructor)(); }
#endif
}

WEAKFUNC
void
abort (void)
{
    _ExitProcess();
}

WEAKFUNC
void
exit (int status)
{
    __fini_cpp();
    _ExitProcess();
}

void
_ExitProcess (void)
{
    PPCHalt();
}
#ifdef __cplusplus
}
#endif
