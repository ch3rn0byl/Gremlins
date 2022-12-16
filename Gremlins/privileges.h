#pragma once
#include <ntddk.h>
#include "kutypes.h"

class privileges
{
public:
	privileges();
	~privileges();

	privileges(
		_In_ PSECURITY_DESCRIPTOR SecurityDescriptor
	);

	_IRQL_requires_max_(APC_LEVEL)
		NTSTATUS
		IsLowIntegrityFriendly(
			_Out_ PBOOLEAN IsFriendly
		);

protected:
private:
	PSECURITY_DESCRIPTOR m_SecurityDescriptor;
	PSID m_EveryoneSid;
	ULONG m_EveryoneSidLength;
};

