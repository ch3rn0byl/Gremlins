#pragma once
#include <ntddk.h>
#include "ProcessHandleTypes.h"

class ProcessHandle
{
public:
	ProcessHandle(
		_In_ HANDLE hFileHandle
	);

	ProcessHandle();
	~ProcessHandle();

	

	NTSTATUS AnalyzeHandle();
	PUNICODE_STRING GetDriverName();
	PUNICODE_STRING GetDeviceObjectName();
	PUNICODE_STRING GetFileObjectName();
	PSECURITY_DESCRIPTOR GetFileObjectSD();

protected:
	PFILE_OBJECT m_FileObject;

private:
	HANDLE m_FileHandle;
	PUNICODE_STRING m_NameInformation;
	ULONG m_ReturnLength;

};


/// EOF