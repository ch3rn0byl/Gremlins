#pragma once
#include <Windows.h>
#include "ErrorHandler.h"

class BaseDriverClient
{
public:
	BaseDriverClient();
	~BaseDriverClient();

protected:
	BOOL init(const wchar_t* testing);

	BOOL SendIoControlRequest(
		_In_ DWORD dwIoControlCode,
		_In_reads_bytes_opt_(dwInBufferSize) LPVOID lpInBuffer = NULL,
		_In_ DWORD dwInBufferSize = NULL,
		_Out_writes_bytes_to_opt_(dwOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer = NULL,
		_In_ DWORD dwOutBufferSize = NULL,
		_Out_opt_ LPDWORD lpBytesReturned = NULL
	);

private:
	HANDLE m_hFileHandle;
	std::unique_ptr<ErrorHandler> m_pErro;

};


/// EOF