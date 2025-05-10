#ifndef __JKR_DISPOSER_H__
#define __JKR_DISPOSER_H__

#include <JSUList.h>

class JKRHeap;

class JKRDisposer
{
    friend class JKRHeap;

    constructor JKRDisposer ();
    destructor ~JKRDisposer();      ///< 0x08v

private:
    JKRHeap*             pHeapObj;  ///< 0x00 // Pointer to inheriting heap (?)
    JSULink<JKRDisposer> mHeapLink; ///< 0x04 // Link to disposer, for heap (?)
};

//
SASSERT_SIZE (JKRDisposer, 0x18);

#endif
