#include "privileges.h"

privileges::privileges() :
    m_SecurityDescriptor(nullptr),
    m_EveryoneSid(nullptr),
    m_EveryoneSidLength(NULL)
{
}

privileges::~privileges()
{
    if (m_EveryoneSid != nullptr && m_EveryoneSidLength > 0)
    {
        RtlSecureZeroMemory(m_EveryoneSid, m_EveryoneSidLength);

#ifdef DBG
        ExFreePoolWithTag(m_EveryoneSid, kuPoolTagDbg);
#else
        ExFreePoolWithTag(m_EveryoneSid, kuPoolTag);
#endif // DBG
        m_EveryoneSid = nullptr;
    }

    m_EveryoneSidLength = NULL;
}

_Use_decl_annotations_
privileges::privileges(PSECURITY_DESCRIPTOR SecurityDescriptor) :
    m_SecurityDescriptor(SecurityDescriptor),
    m_EveryoneSid(nullptr),
    m_EveryoneSidLength(NULL)
{
}

_Use_decl_annotations_
NTSTATUS privileges::IsLowIntegrityFriendly(PBOOLEAN IsFriendly)
{
    SID_IDENTIFIER_AUTHORITY EveryoneSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

    PACL pDacl = nullptr;
    PACCESS_ALLOWED_ACE pAllowedAce = nullptr;

    BOOLEAN bDaclPresent = NULL;
    BOOLEAN bDaclDefaulted = NULL;

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    *IsFriendly = FALSE;

    //
    // Get the DACL of the driver's security descriptor. If a Dacl is 
    // not present, bail. 
    //
    Status = RtlGetDaclSecurityDescriptor(
        m_SecurityDescriptor,
        &bDaclPresent,
        &pDacl,
        &bDaclDefaulted
    );
    if (!NT_SUCCESS(Status) || !bDaclPresent)
    {
        return Status;
    }

    //
    // According to the MSDN, if the dacl is present and the dacl
    // is NULL, this means that all access is allowed to this object. Place
    // a breakpoint here to see what kind of goodies could lie ahead. I've
    // never seen this before so it should be interesting >:)
    // 
    if (bDaclPresent && pDacl == NULL)
    {
        DbgPrint("This object allows all access!\n");
        DbgBreakPoint();
    }

    //
    // Get the size of the buffer needed to store the SID. 
    //
    m_EveryoneSidLength = RtlLengthRequiredSid(1);

    m_EveryoneSid = static_cast<PSID>(
#ifdef DBG
        ExAllocatePool2(POOL_FLAG_NON_PAGED | POOL_FLAG_SPECIAL_POOL, m_EveryoneSidLength, kuPoolTagDbg)
#else
        ExAllocatePool2(POOL_FLAG_NON_PAGED, m_EveryoneSidLength, kuPoolTag)
#endif // DBG
        );
    if (m_EveryoneSid == NULL)
    {
        return STATUS_MEMORY_NOT_ALLOCATED;
    }

    //
    // Initialize the SID
    //
    Status = RtlInitializeSid(m_EveryoneSid, &EveryoneSidAuthority, 1);
    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    //
    // Now iterate through all the entries and check to see if we have
    // one that contains the everyone sid. If so, this object is low integrity
    // friendly. 
    //
    for (USHORT i = 0; i < pDacl->AceCount; i++)
    {
        Status = RtlGetAce(pDacl, i, reinterpret_cast<PVOID*>(&pAllowedAce));
        if (NT_SUCCESS(Status) && pAllowedAce != nullptr)
        {
            if (RtlEqualSid(m_EveryoneSid, &pAllowedAce->SidStart))
            {
                *IsFriendly = TRUE;
                break;
            }
        }
    }

    return Status;
}
