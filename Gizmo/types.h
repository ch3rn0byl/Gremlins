#pragma once
#include <windef.h>
#include <cstdint>

///-------------------------------------------------------------------------------------------------
/// Macaroo's
///-------------------------------------------------------------------------------------------------
#define ENCODE_CTL(Function) ( \
    ((0x8000) << 16) |                  \
    ((FILE_READ_ACCESS) << 14) |              \
    ((Function) << 2) |                     \
    (METHOD_BUFFERED)                       \
)

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

///-------------------------------------------------------------------------------------------------
/// Definitions
///-------------------------------------------------------------------------------------------------
#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

// 
// Syscall signature for ntdll. 
// 
static UINT8 SyscallSig[] = {
    0x4c, 0x8b, 0xd1,	// mov r10, rcx
    0xb8				// mov eax, ??
};

//====================================================
// Types: IOCTLs
//====================================================
enum IOCTL_FUNCTION : INT32
{
    IsInitialized = ENCODE_CTL(0x800),
    Initialize = ENCODE_CTL(0x801),
    IsHooked = ENCODE_CTL(0x802),
    Hook = ENCODE_CTL(0x803),
    Unhook = ENCODE_CTL(0x804),
    ExcludeDriver = ENCODE_CTL(0x805),
    ImageAnalysisIoctl = ENCODE_CTL(0x806)
};

typedef struct _INPUT_BUFFER
{
    UINT16 syscall;
    bool status;
} INPUT_BUFFER, * PINPUT_BUFFER;


/// EOF