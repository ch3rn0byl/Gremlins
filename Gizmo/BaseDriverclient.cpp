#include "BaseDriverClient.h"

BaseDriverClient::BaseDriverClient() :
	m_hFileHandle(INVALID_HANDLE_VALUE),
	m_pErro(nullptr)
{
}

BaseDriverClient::~BaseDriverClient()
{
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
}

BOOL BaseDriverClient::init(const wchar_t* testing)
{
	m_hFileHandle = CreateFile(
		testing,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	return TRUE;
}

_Use_decl_annotations_
BOOL
BaseDriverClient::SendIoControlRequest(
	DWORD dwIoControlCode,
	LPVOID lpInBuffer,
	DWORD dwInBufferSize,
	LPVOID lpOutBuffer,
	DWORD dwOutBufferSize,
	LPDWORD lpBytesReturned
)
{
	BOOL bStatus = DeviceIoControl(
		m_hFileHandle,
		dwIoControlCode,
		lpInBuffer,
		dwInBufferSize,
		lpOutBuffer,
		dwOutBufferSize,
		lpBytesReturned,
		NULL
	);
	if (!bStatus)
	{
		m_pErro = std::make_unique<ErrorHandler>(GetLastError());
	}

	return bStatus;
}


/// EOF