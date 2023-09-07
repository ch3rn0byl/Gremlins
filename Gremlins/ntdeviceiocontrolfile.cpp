#include "ntdeviceiocontrolfile.h"

_Use_decl_annotations_
NTSTATUS
fn_hNtDeviceIoControlFile(
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
    NTSTATUS hStatus = STATUS_UNSUCCESSFUL;

    //BOOLEAN bIsFriendly = FALSE;

   // UINT64 LeakedAddress = 0;

    KPROCESSOR_MODE PreviousMode = ExGetPreviousMode();
    // internals of afl
    // create basic one for kernel
    // 
    //
    // Only interested in calls coming from within usermode.
    // 
    if (PreviousMode == UserMode)
    {
        ProcessHandle ph = ProcessHandle(FileHandle);

        hStatus = ph.AnalyzeHandle();
        if (NT_SUCCESS(hStatus))
        {
            if (exclusions::IsKernelImageExcluded(ph.GetDriverName()))
            {
                goto FinishUp;
            }

            if (exclusions::IsKernelImageForAnalysis(ph.GetFileObjectName()))
            {
                ExAcquireFastMutex(&g_Globals->fMutex);

                LOG_TRACE("[%ws::%d] \n\n", __FUNCTIONW__, __LINE__);
                LOG_TRACE("[%ws::%d] Device Name: %wZ\n", __FUNCTIONW__, __LINE__, ph.GetFileObjectName());
                LOG_TRACE("[%ws::%d] Ioctl: %04x\n", __FUNCTIONW__, __LINE__, IoControlCode);
                LOG_TRACE("[%ws::%d] InputBuffer: %p\n", __FUNCTIONW__, __LINE__, InputBuffer);
                LOG_TRACE("[%ws::%d] InputBufferLength: %d\n", __FUNCTIONW__, __LINE__, InputBufferLength);
                LOG_TRACE("[%ws::%d] OutputBuffer: %p\n", __FUNCTIONW__, __LINE__, OutputBuffer);
                LOG_TRACE("[%ws::%d] OutputBufferLength: %d\n", __FUNCTIONW__, __LINE__, OutputBufferLength);

                LOG_TRACE("[%ws::%d] Dumping input buffer: ", __FUNCTIONW__, __LINE__);
                for (ULONG i = 0; i < InputBufferLength; i++)
                {
                    LOG_TRACE("%02x ", static_cast<PUINT8>(InputBuffer)[i]);
                }
                KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "\n"));

                LOG_TRACE("[%ws::%d] \n\n", __FUNCTIONW__, __LINE__);

                ExReleaseFastMutex(&g_Globals->fMutex);
                __debugbreak();
			}

            if (METHOD_FROM_CTL_CODE(IoControlCode) == METHOD_NEITHER)
            {
                ExAcquireFastMutex(&g_Globals->fMutex);

                LOG_TRACE("[%ws::%d] \n\n", __FUNCTIONW__, __LINE__);
                LOG_TRACE("[%ws::%d] Device Name: %wZ\n", __FUNCTIONW__, __LINE__, ph.GetFileObjectName());
                LOG_TRACE("[%ws::%d] Ioctl: %04x\n", __FUNCTIONW__, __LINE__, IoControlCode);
                LOG_TRACE("[%ws::%d] InputBuffer: %p\n", __FUNCTIONW__, __LINE__, InputBuffer);
                LOG_TRACE("[%ws::%d] InputBufferLength: %d\n", __FUNCTIONW__, __LINE__, InputBufferLength);
                LOG_TRACE("[%ws::%d] OutputBuffer: %p\n", __FUNCTIONW__, __LINE__, OutputBuffer);
                LOG_TRACE("[%ws::%d] OutputBufferLength: %d\n", __FUNCTIONW__, __LINE__, OutputBufferLength);

                LOG_TRACE("[%ws::%d] Dumping input buffer: ", __FUNCTIONW__, __LINE__);
                for (ULONG i = 0; i < InputBufferLength; i++)
                {
                    LOG_TRACE("%02x ", static_cast<PUINT8>(InputBuffer)[i]);
                }
                KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "\n"));

                LOG_TRACE("[%ws::%d] \n\n", __FUNCTIONW__, __LINE__);

                ExReleaseFastMutex(&g_Globals->fMutex);
            }

            /*
            ExAcquireFastMutex(&g_Globals->fMutex);

            LOG_TRACE("[%ws::%d] \n\n", __FUNCTIONW__, __LINE__);
            LOG_TRACE("[%ws::%d] Device Name: %wZ\n", __FUNCTIONW__, __LINE__, ph.GetFileObjectName());
            LOG_TRACE("[%ws::%d] Ioctl: %04x\n", __FUNCTIONW__, __LINE__, IoControlCode);
            LOG_TRACE("[%ws::%d] InputBuffer: %p\n", __FUNCTIONW__, __LINE__, InputBuffer);
            LOG_TRACE("[%ws::%d] InputBufferLength: %d\n", __FUNCTIONW__, __LINE__, InputBufferLength);
            LOG_TRACE("[%ws::%d] OutputBuffer: %p\n", __FUNCTIONW__, __LINE__, OutputBuffer);
            LOG_TRACE("[%ws::%d] OutputBufferLength: %d\n", __FUNCTIONW__, __LINE__, OutputBufferLength);

            LOG_TRACE("[%ws::%d] Dumping input buffer: ", __FUNCTIONW__, __LINE__);
            for (ULONG i = 0; i < InputBufferLength; i++)
            {
                LOG_TRACE("%02x ", static_cast<PUINT8>(InputBuffer)[i]);
            }
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_TRACE_LEVEL, "\n"));

            LOG_TRACE("[%ws::%d] \n\n", __FUNCTIONW__, __LINE__);

            ExReleaseFastMutex(&g_Globals->fMutex);
            */

        }
        /*
        //
        // The following tests are not taxing, therefore, these tests can be done with no performance
        // impact. If fuzz logic will be introduced, threads would need to be fired off otherwise the
        // system would come to a crawl.
        //
        ProcessHandle ph = ProcessHandle(FileHandle);

        //
        // Analyze the handle of the file object.
        //
        hStatus = ph.AnalyzeHandle();
        if (NT_SUCCESS(hStatus))
        {
            //
            // Do a quick leak test to see if there are any kernel address leaks.
            //
            Leaks LeakTesting = Leaks(
                static_cast<PUINT8>(OutputBuffer),
                OutputBufferLength
            );

            LeakedAddress = LeakTesting.DidKernelAddressLeak();
            if (LeakedAddress != NULL)
            {
                //
                // TODO: extend the AUX wrapper to find out what module address was leaked.
                //
                LOG_INFO("[%ws::%d] Kernel address leaked: %p.\n", __FUNCTIONW__, __LINE__, LeakedAddress);
            }

            //
            // The privileges method is used to check if the device object in question is low
            // integrity friendly. Would be a potential SBX/AppContainer escape!
            //
            privileges p = privileges(ph.GetFileObjectSD());

            hStatus = p.IsLowIntegrityFriendly(&bIsFriendly);
            if (NT_SUCCESS(hStatus))
            {
                if (bIsFriendly)
                {
                    LOG_INFO("[%ws::%d] SBX friendly.\n", __FUNCTIONW__, __LINE__);
                }
            }
            else
            {
                LOG_ERR("[%ws::%d] Failed with status 0x%08x.\n", __FUNCTIONW__, __LINE__, hStatus);
            }
        }
        else
        {
            LOG_ERR("[%ws::%d] Failed with status 0x%08x.\n", __FUNCTIONW__, __LINE__, hStatus);
        }
        */
    }


    Status = g_Globals->internal.IopXxxControlFile(
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

FinishUp:
    return Status;
}


/// EOF