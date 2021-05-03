#pragma once
#include <ntifs.h>
#include <ntddk.h>

///-------------------------------------------------------------------------------------------------
/// Definitions
///-------------------------------------------------------------------------------------------------
#ifndef __TITLE__
#define __TITLE__ L"Gremlins"
#endif // !__TITLE__

///-------------------------------------------------------------------------------------------------
/// Types
///-------------------------------------------------------------------------------------------------
typedef enum class _SYSCALL_INDEX : INT16
{
    NtDeviceIoControlFileIndex = 0x0007,
    NtCreateFileIndex = 0x0055
} SYSCALL_INDEX;

typedef enum class _IOCTL_FUNCTION : INT32
{
    IsInitialized = CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA),
    Initialize = CTL_CODE(0x8000, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA),
    IsHooked = CTL_CODE(0x8000, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA),
    Hook = CTL_CODE(0x8000, 0x803, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA),
    Unhook = CTL_CODE(0x8000, 0x804, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
} IOCTL_FUNCTION;

///
/// 1: kd> dt nt!_OBJECT_INFORMATION_CLASS
/// 
typedef enum _OBJECT_INFORMATION_CLASS2
{
    ///ObjectBasicInformation = 0,
    ObjectNameInformation = 1,
    ///ObjectTypeInformation = 2,
    ObjectTypesInformation = 3,
    ObjectHandleFlagInformation = 4,
    ObjectSessionInformation = 5,
    ObjectSessionObjectInformation = 6,
    MaxObjectInfoClass = 7
} OBJECT_INFORMATION_CLASS2;

///-------------------------------------------------------------------------------------------------
/// Undocumented functions
///-------------------------------------------------------------------------------------------------
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

///-------------------------------------------------------------------------------------------------
/// Objects
///-------------------------------------------------------------------------------------------------
typedef struct _INPUT_BUFFER
{
    UINT16 padding[1] = { 0 };
    UINT16 syscall = 0;
    bool status = false;
} INPUT_BUFFER, * PINPUT_BUFFER;

typedef struct _KSERVICE_DESCRIPTOR_TABLE
{
    PULONG ServiceTableBase = nullptr;
    PULONG ServiceCounterTableBase = nullptr;
    ULONG NumberOfServices = 0;
    PULONG ParamTableBase = nullptr;
}KSERVICE_DESCRIPTOR_TABLE, * PKSERVICE_DESCRIPTOR_TABLE;

typedef struct _GLOBALS_HEADER
{
    _Guarded_by_(kInterlockedSpinLock) LIST_ENTRY ListHead;

    ///
    /// I am using the ExInterlockedXXXList routines and the MSDN specifically
    /// states that the lock used on those API's should only be used for them and to
    /// not use their lock for any other purpose. You can use the same lock for 
    /// muliptle lists, but this behavior increases lock contention. I ain't got 
    /// time fo dat
    /// 
    KSPIN_LOCK kSpinLock;
    KSPIN_LOCK kInterlockedSpinLock;
    FAST_MUTEX fMutex;
    bool isInitialized = false;
    UNICODE_STRING BinaryName = { 0 };
} GLOBALS_HEADER, * PGLOBALS_HEADER;

typedef struct _NT_SUB_FUNCTIONS
{
    _IopCreateFile IopCreateFile = nullptr;
    _IopXxxControlFile IopXxxControlFile = nullptr;
} NT_SUB_FUNCTIONS, * PNT_SUB_FUNCTIONS;

typedef struct _HOOKED_NT_FUNCTION
{
    bool isHooked = false;
    UINT16 index = 0;
    LIST_ENTRY entry;
    PVOID address = nullptr;
    UINT8 original[16] = { 0 };
} HOOKED_NT_FUNCTION, * PHOOKED_NT_FUNCTION;

typedef struct _GLOBALS : GLOBALS_HEADER
{
    PKSERVICE_DESCRIPTOR_TABLE ssdt;
    PHOOKED_NT_FUNCTION hooked;
    NT_SUB_FUNCTIONS sub;
} GLOBALS, * PGLOBALS;

///-------------------------------------------------------------------------------------------------
/// Globals
///-------------------------------------------------------------------------------------------------
///
/// The pool tag is yours truly: ChRn
/// 
constexpr int POOLTAG = 'nRhC';
constexpr UINT32 IA32_LSTAR_MSR = 0xc0000082;

///
/// The stub for the hook. The instructions are:
///     mov rax, 1122334455667788h
///     jmp rax
/// 
static UINT8 hookedcode[] = {
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0xe0
};

extern PGLOBALS g_Globals;


/// EOF