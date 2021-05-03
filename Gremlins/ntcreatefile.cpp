#include "ntcreatefile.h"

NTSTATUS NtCF(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
)
{
    KPROCESSOR_MODE kmPreviousMode = ExGetPreviousMode();

    if (kmPreviousMode == UserMode)
    {
        PEPROCESS_WINDBG Process = reinterpret_cast<PEPROCESS_WINDBG>(PsGetCurrentProcess());

        ExAcquireFastMutex(&g_Globals->fMutex);
        DbgPrint("=============================================================================\n");
        DbgPrint("[%ws::%ws::%d] %wZ is reaching to %wZ\n", __TITLE__, __FUNCTIONW__, __LINE__, Process->Peb->ProcessParameters->ImagePathName, ObjectAttributes->ObjectName);
        DbgPrint("[%ws::%ws::%d] EaBuffer: %p\n", __TITLE__, __FUNCTIONW__, __LINE__, EaBuffer);
        DbgPrint("[%ws::%ws::%d] EaLength: %d\n", __TITLE__, __FUNCTIONW__, __LINE__, EaLength);

        if (EaBuffer && EaLength > 0)
        {
            for (unsigned int i = 0; i < EaLength; i++)
            {
                DbgPrint("%02x ", *(PUINT8)((PUCHAR)EaBuffer + i));
            }
            DbgPrint("\n");
        }
       
        DbgPrint("=============================================================================\n");
        ExReleaseFastMutex(&g_Globals->fMutex);
    }

    return g_Globals->sub.IopCreateFile(
        FileHandle,
        DesiredAccess,
        ObjectAttributes,
        IoStatusBlock,
        AllocationSize,
        FileAttributes,
        ShareAccess,
        CreateDisposition,
        CreateOptions,
        EaBuffer,
        EaLength,
        CreateFileTypeNone,
        NULL,
        NULL,
        0x20,
        NULL
    );
}


/// EOF