#pragma once
#include <ntddk.h>
#include "ProcessHandleTypes.h"

class ProcessHandle
{
public:
	ProcessHandle();
	~ProcessHandle();

	ProcessHandle(
		_In_ HANDLE hFileHandle
	);

	NTSTATUS AnalyzeHandle();
	PSECURITY_DESCRIPTOR GetFileObjectSD();

protected:
	PFILE_OBJECT m_FileObject;

private:
	HANDLE m_FileHandle;
	PUNICODE_STRING m_NameInformation;
	ULONG m_ReturnLength;
};


/// EOF