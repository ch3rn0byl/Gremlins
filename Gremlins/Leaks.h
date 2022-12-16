#pragma once
#include "AuxWrapper.h"
#include "LeaksTypes.h"

class Leaks :
    public AuxWrapper
{
public:
    Leaks(
        _In_reads_bytes_(BufferLen) PUINT8 Buffer,
        _In_ ULONG BufferLen
    );

    ~Leaks();

    _IRQL_requires_min_(PASSIVE_LEVEL)
        _Must_inspect_result_
        UINT64
        DidKernelAddressLeak();

private:
    const PUINT8 m_Buffer;
    const ULONG m_BufferLen;
};


/// EOF