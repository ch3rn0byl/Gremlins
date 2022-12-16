#pragma once
#include <ntddk.h>

//====================================================
// Macros
//====================================================
#ifdef DBG
#define LOG_ERR(format, ...) KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__))
#define LOG_WARN(format, ...) KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, format, __VA_ARGS__))
#define LOG_TRACE(format, ...) KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, format, __VA_ARGS__))
#define LOG_INFO(format, ...) KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, __VA_ARGS__))
#else
#define LOG_ERR(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, format, __VA_ARGS__)
#define LOG_WARN(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, format, __VA_ARGS__)
#define LOG_TRACE(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, format, __VA_ARGS__)
#define LOG_INFO(format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, __VA_ARGS__)
#endif // DBG

#define ENCODE_CTL(Function) ( \
    ((0x8000) << 16) |                  \
    ((FILE_READ_ACCESS) << 14) |              \
    ((Function) << 2) |                     \
    (METHOD_BUFFERED)                       \
)

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

//====================================================
// Types: Syscall Indexes
//====================================================
enum SYSCALL_INDEX : INT16
{
    NtDeviceIoControlFile = 7,
    NtCreateFile = 0x55
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
    Unhook = ENCODE_CTL(0x804)
};

//====================================================
// Types: Undocumented Functions
//====================================================
typedef NTSTATUS(NTAPI* _IopXxxControlFile)(
    _In_ HANDLE FileHandle,
    _In_opt_ HANDLE Event,
    _In_opt_ PIO_APC_ROUTINE ApcRoutine,
    _In_opt_ PVOID ApcContext,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_ ULONG IoControlCode,
    _In_opt_ PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_opt_ PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _In_ BOOLEAN DeviceIoControl
    );

typedef NTSTATUS(NTAPI* _IopCreateFile)(
    _Out_ PHANDLE FileHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes,
    _Out_ PIO_STATUS_BLOCK IoStatusBlock,
    _In_opt_ PLARGE_INTEGER AllocationSize,
    _In_ ULONG FileAttributes,
    _In_ ULONG ShareAccess,
    _In_ ULONG CreateDisposition,
    _In_ ULONG CreateOptions,
    _In_opt_ PVOID EaBuffer,
    _In_ ULONG EaLength,
    _In_ CREATE_FILE_TYPE CreateFileType,
    _In_opt_ PVOID ExtraCreateParameters,
    _In_ ULONG Options,
    _In_ ULONG Flags,
    _In_opt_ PDEVICE_OBJECT DeviceObject
    );

//====================================================
// Definitions
//====================================================
typedef struct _INPUT_BUFFER
{
    UINT16 syscall;
    bool status;
    //NTSTATUS Status;
} INPUT_BUFFER, * PINPUT_BUFFER;

struct KSERVICE_DESCRIPTOR_TABLE
{
    PULONG ServicetableBase;
    PULONG ServiceCounterTableBase;
    ULONG NumberOfServices;
    PULONG ParamTablebase;
};

struct GLOBALS_HEADER
{
    LIST_ENTRY ListHead;

    //
    // I am using the ExInterlockedXXXList routines and the MSDN specifically 
    // states that the lock used on those API's should only be used for them and to
    // not use their lock for any other purpose. You can use the same lock for multiple
    // lists, but this behavior increases lock contention. I ain't got time
    // fo dat.
    //
    KSPIN_LOCK kSpinLock;
    KSPIN_LOCK kInterlockedSpinLock;
    FAST_MUTEX fMutex;
    bool IsInitialized;
    UNICODE_STRING BinaryName;
};

//
// If there are any internal functions that get called in the syscall, throw
// dem hoes up in here. 
//
struct INTERNAL_NT
{
    _IopCreateFile IopCreateFile;
    _IopXxxControlFile IopXxxControlFile;
};

struct HOOKED_SYSCALLS
{
    bool IsHooked;
    UINT16 Index;
    LIST_ENTRY entry;
    PVOID address;
    UINT8 original[16];
};

typedef HOOKED_SYSCALLS* PHOOKED_SYSCALLS;
typedef KSERVICE_DESCRIPTOR_TABLE* PKSERVICE_DESCRIPTOR_TABLE;

struct GLOBALS : GLOBALS_HEADER
{
    PKSERVICE_DESCRIPTOR_TABLE ssdt;
    PHOOKED_SYSCALLS hooked;
    INTERNAL_NT internal;
};


//====================================================
// Globals
//====================================================
constexpr auto POOLTAG = 'nRhC';
constexpr auto POOLTAG_DBG = 'gBdC';
constexpr auto IA32_LSTAR_MSR = 0xc0000082;

typedef GLOBALS* PGLOBALS;

//
// The stub for the hook encoded as follows:
//   mov rax, 0
//   jmp rax
static UINT8 HookingStub[] = {
    0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xe0
};

extern PGLOBALS g_Globals;


/// EOF