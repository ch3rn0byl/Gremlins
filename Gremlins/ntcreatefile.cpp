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
        if (PsGetCurrentProcessId() == (HANDLE)5300)
        {
            ExAcquireFastMutex(&g_Globals->fMutex);

            LOG_TRACE("[%ws::%d] NtCreateFile called from %d\n", __FUNCTIONW__, __LINE__, HandleToULong(PsGetCurrentProcessId()));
            LOG_TRACE("[%ws::%d] Access mask: %04x\n", __FUNCTIONW__, __LINE__, DesiredAccess);

            if (ObjectAttributes != nullptr)
            {
                LOG_TRACE("[%ws::%d] Object attribute length: %d\n", __FUNCTIONW__, __LINE__, ObjectAttributes->Length);
                LOG_TRACE("[%ws::%d] Object attribute root directory: %08x\n", __FUNCTIONW__, __LINE__, ObjectAttributes->RootDirectory);
                LOG_TRACE("[%ws::%d] Object attribute name: %wZ\n", __FUNCTIONW__, __LINE__, ObjectAttributes->ObjectName);
                LOG_TRACE("[%ws::%d] Object attribute attributes: %08x\n", __FUNCTIONW__, __LINE__, ObjectAttributes->Attributes);
                LOG_TRACE("[%ws::%d] Object attribute security descriptor: %p\n", __FUNCTIONW__, __LINE__, ObjectAttributes->SecurityDescriptor);
                LOG_TRACE("[%ws::%d] Object attribute security quality of service: %p\n", __FUNCTIONW__, __LINE__, ObjectAttributes->SecurityQualityOfService);
            }

            LOG_TRACE("[%ws::%d] Allocation size: %p\n", __FUNCTIONW__, __LINE__, AllocationSize);
            LOG_TRACE("[%ws::%d] File attributes: %08x\n", __FUNCTIONW__, __LINE__, FileAttributes);
            LOG_TRACE("[%ws::%d] Share access: %08x\n", __FUNCTIONW__, __LINE__, ShareAccess);
            LOG_TRACE("[%ws::%d] Create disposition: %08x\n", __FUNCTIONW__, __LINE__, CreateDisposition);
            LOG_TRACE("[%ws::%d] Create options: %08x\n", __FUNCTIONW__, __LINE__, CreateOptions);
            
            if (EaBuffer && EaLength != 0)
            {
                LOG_TRACE("[%ws::%d] Ea buffer: %p\n", __FUNCTIONW__, __LINE__, EaBuffer);
                LOG_TRACE("[%ws::%d] Ea length: %08x\n", __FUNCTIONW__, __LINE__, EaLength);
                LOG_TRACE("[%ws::%d] Dumping ea buffer: \n", __FUNCTIONW__, __LINE__);

                PUINT8 EaBufferTemp = static_cast<PUINT8>(EaBuffer);

                for (ULONG i = 0; i < EaLength; i++)
                {
                    LOG_TRACE("%02x ", EaBufferTemp[i]);
                }
            }
            LOG_TRACE("[%ws::%d]========================================================================\n", __FUNCTIONW__, __LINE__);
            ExReleaseFastMutex(&g_Globals->fMutex);
        }
		else
		{
			goto FinishUp;
		}
    }
    
FinishUp:
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