#pragma once
#include <ntdef.h>

//================================================================================
// Definitions
//================================================================================
constexpr ULONG ObReferenceTag = 'OgbD';
constexpr ULONG phPoolTagDbg = 'bDhP';
constexpr ULONG phPoolTag = 'yThP';

//================================================================================
// Types
//================================================================================
typedef enum _OBJECT_INFORMATION_CLASS
{
    ObjectBasicInformation = 0,
    ObjectNameInformation = 1,
    ObjectTypeInformation = 2,
    ObjectTypesInformation = 3,
    ObjectHandleFlagInformation = 4,
    ObjectSessionInformation = 5,
    ObjectSessionObjectInformation = 6,
    MaxObjectInfoClass = 7
} OBJECT_INFORMATION_CLASS;

//================================================================================
// Exported Functions
//================================================================================
EXTERN_C
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryObject(
    _In_opt_ HANDLE Handle,
    _In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
    _Out_writes_bytes_opt_(ObjectInformationLength) PVOID ObjectInformation,
    _In_ ULONG ObjectInformationLength,
    _Out_opt_ PULONG ReturnLength
);


/// EOF