#include "ntdeviceiocontrolfile.h"

NTSTATUS NtDICF(
    HANDLE FileHandle, 
    HANDLE Event, 
    PIO_APC_ROUTINE ApcRoutine, 
    PVOID ApcContext, 
    PIO_STATUS_BLOCK IoStatusBlock, 
    ULONG IoControlCode, 
    PVOID InputBuffer, 
    ULONG InputBufferLength, 
    PVOID OutputBuffer, 
    ULONG OutputBufferLength
)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    KPROCESSOR_MODE kmPreviousMode = ExGetPreviousMode();
    PFILE_OBJECT pFileObject = nullptr;

    ///
    /// Only interested in calls made from Usermode
    /// 
    if (kmPreviousMode == UserMode)
    {
        Status = ObReferenceObjectByHandle(
            FileHandle,
            0,
            0,
            KernelMode,
            reinterpret_cast<PVOID*>(&pFileObject),
            NULL
        );
        if (NT_SUCCESS(Status))
        {
            ///
            /// If we catch our driver, skip it. Not interested in fuzzing our driver
            /// 
            UNICODE_STRING usDriverName = RTL_CONSTANT_STRING(L"\\Driver\\Gremlins");
            UNICODE_STRING usTargetName = pFileObject->DeviceObject->DriverObject->DriverName;

            if (RtlCompareUnicodeString(&usTargetName, &usDriverName, false) != 0)
            {
                PEPROCESS_WINDBG Process = reinterpret_cast<PEPROCESS_WINDBG>(PsGetCurrentProcess());

                ///
                /// Same with our binary. Skip it, not interested in fuzzing Gizmo
                /// 
                if (RtlCompareUnicodeString(&Process->Peb->ProcessParameters->ImagePathName, &g_Globals->BinaryName, false) != 0)
                {
                    ///
                    /// Skip MsMpEng.exe. It's too much and makes the vm die; plus, you can't interact with it as a low level user
                    /// 
                    UNICODE_STRING usMsMpEng = RTL_CONSTANT_STRING(L"C:\\ProgramData\\Microsoft\\Windows Defender\\Platform\\4.18.2103.7-0\\MsMpEng.exe");
                    
                    if (RtlCompareUnicodeString(&Process->Peb->ProcessParameters->ImagePathName, &usMsMpEng, false) != 0)
                    {
                        ULONG ReturnLength = 0;
                        PVOID NameInformationObject = nullptr;

                        Status = ZwQueryObject(
                            FileHandle,
                            static_cast<OBJECT_INFORMATION_CLASS>(ObjectNameInformation),
                            NULL,
                            ReturnLength,
                            &ReturnLength
                        );
                        if (!NT_SUCCESS(Status))
                        {
                            NameInformationObject = ExAllocatePoolWithTag(NonPagedPoolNx, ReturnLength, POOLTAG);
                            if (NameInformationObject != NULL)
                            {
                                RtlSecureZeroMemory(NameInformationObject, ReturnLength);

                                Status = ZwQueryObject(
                                    FileHandle,
                                    static_cast<OBJECT_INFORMATION_CLASS>(ObjectNameInformation),
                                    NameInformationObject,
                                    ReturnLength,
                                    &ReturnLength
                                );


                                if (!NT_SUCCESS(Status))
                                {
                                    ///DbgPrint("What. The. Eff.\n");
                                    DbgPrint("[%ws::%ws::%d] ZwQueryObject failed.\n", __TITLE__, __FUNCTIONW__, __LINE__);
                                }
                            }
                            else
                            {
                                ///DbgPrint("Damn wtheck is going on???\n");
                                DbgPrint("[%ws::%ws::%d] ExAllocatePoolWithTag failed.\n", __TITLE__, __FUNCTIONW__, __LINE__);
                            }
                        }
                        else
                        {
                            DbgPrint("[%ws::%ws::%d] ZwQueryObject failed.\n", __TITLE__, __FUNCTIONW__, __LINE__);
                        }

                        ExAcquireFastMutex(&g_Globals->fMutex);

                        DbgPrint("=============================================================================\n");
                        DbgPrint("[%ws::%ws::%d] Image Name: %wZ\n", __TITLE__, __FUNCTIONW__, __LINE__, Process->Peb->ProcessParameters->ImagePathName);
                        DbgPrint("[%ws::%ws::%d] Device Name: %wZ\n", __TITLE__, __FUNCTIONW__, __LINE__, reinterpret_cast<PUNICODE_STRING>(NameInformationObject));
                        DbgPrint("[%ws::%ws::%d] Current Process: %d\n", __TITLE__, __FUNCTIONW__, __LINE__, PsGetCurrentProcessId());
                        DbgPrint("[%ws::%ws::%d] Current Commandline: %wZ\n", __TITLE__, __FUNCTIONW__, __LINE__, Process->Peb->ProcessParameters->CommandLine);

                        if (pFileObject->FileName.Length != 0)
                        {
                            DbgPrint("[%ws::%ws::%d] Filename: %wZ\n", __FUNCTIONW__, __LINE__, pFileObject->FileName);
                        }

                        DbgPrint("[%ws::%ws::%d] IOCTL: %04x\n", __TITLE__, __FUNCTIONW__, __LINE__, IoControlCode);
                        DbgPrint("[%ws::%ws::%d] InputBuffer: %p\n", __TITLE__, __FUNCTIONW__, __LINE__, InputBuffer);
                        DbgPrint("[%ws::%ws::%d] InputBufferLength: %d\n", __TITLE__, __FUNCTIONW__, __LINE__, InputBufferLength);
                        if (InputBuffer && InputBufferLength > 0)
                        {
                            for (unsigned int i = 0; i < InputBufferLength; i++)
                            {
                                UINT8 value = *(PUINT8)((PUCHAR)InputBuffer + i);

                                if (value > 0x19 && value < 0x7f)
                                {
                                    DbgPrint("%c ", value);
                                }
                                else
                                {
                                    DbgPrint("%02x ", value);
                                }
                            }
                            DbgPrint("\n");
                        }

                        DbgPrint("[%ws::%ws::%d] OutputBuffer: %p\n", __TITLE__, __FUNCTIONW__, __LINE__, OutputBuffer);
                        DbgPrint("[%ws::%ws::%d] OutputBufferLength: %d\n", __TITLE__, __FUNCTIONW__, __LINE__, OutputBufferLength);

                        if (OutputBuffer && OutputBufferLength > 0)
                        {
                            for (unsigned int i = 0; i < OutputBufferLength; i++)
                            {
                                UINT8 value = *(PUINT8)((PUCHAR)OutputBuffer + i);

                                if (value > 0x19 && value < 0x7f)
                                {
                                    DbgPrint("%c ", value);
                                }
                                else
                                {
                                    DbgPrint("%02x ", value);
                                }
                            }
                            DbgPrint("\n");
                        }
                        DbgPrint("=============================================================================\n");

                        ExReleaseFastMutex(&g_Globals->fMutex);
                    }
                }
            }
        }
        else
        {
            DbgPrint("[%ws::%ws::%d] ObReferenceObjectByHandle failed.\n", __TITLE__, __FUNCTIONW__, __LINE__);
        }
    }

    if (pFileObject != nullptr)
    {
        ObDereferenceObject(pFileObject);
        pFileObject = nullptr;
    }

    return g_Globals->sub.IopXxxControlFile(
        FileHandle,
        Event,
        ApcRoutine,
        ApcContext,
        IoStatusBlock,
        IoControlCode,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        TRUE
    );
}


/// EOF