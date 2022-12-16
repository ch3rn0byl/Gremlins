#pragma once
#include <ntdef.h>

typedef union _VIRTUAL_ADDR
{
    struct
    {
        UINT64 Reserved : 63; // Reserved: I don't care about these bits
        UINT64 MSB : 1; // Most significant bit: used to check if kernel address
    };
    ULARGE_INTEGER Address;
    UINT64 value;
} VIRTUAL_ADDR, * PVIRTUAL_ADDR;


/// EOF