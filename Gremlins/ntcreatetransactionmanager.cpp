#include "ntcreatetransactionmanager.h"

_Use_decl_annotations_
NTSTATUS fn_hNtCreateTransactionManager(
    PHANDLE TmHandle, 
    ACCESS_MASK DesiredAccess, 
    POBJECT_ATTRIBUTES ObjectAttributes, 
    PUNICODE_STRING LogFileName, 
    ULONG CreateOptions, 
    ULONG CommitStrength
)
{
    UNREFERENCED_PARAMETER(TmHandle);
    UNREFERENCED_PARAMETER(DesiredAccess);
    UNREFERENCED_PARAMETER(ObjectAttributes);
    UNREFERENCED_PARAMETER(LogFileName);
    UNREFERENCED_PARAMETER(CreateOptions);
    UNREFERENCED_PARAMETER(CommitStrength);

    return STATUS_SUCCESS;
}
