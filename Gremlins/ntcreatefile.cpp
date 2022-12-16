#include "ntcreatefile.h"

_Use_decl_annotations_
NTSTATUS fn_hNtCreateFile(
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
        //
        // Implement logic to suit your needs.
        //
    }
    
    return g_Globals->internal.IopCreateFile(
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