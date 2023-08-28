#include "BaseDriverClient.h"

_Use_decl_annotations_
BaseDriverClient::BaseDriverClient(LPCWSTR lpFileName) :
	m_hFileHandle(INVALID_HANDLE_VALUE),
	m_pErro(nullptr)
{
	m_hFileHandle = CreateFile(
		lpFileName,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (m_hFileHandle == INVALID_HANDLE_VALUE)
	{
		m_pErro = std::make_unique<ErrorHandler>(GetLastError());

		throw std::runtime_error(m_pErro->GetLastErrorAsStringA().c_str());
	}
}

BaseDriverClient::BaseDriverClient() :
	m_hFileHandle(INVALID_HANDLE_VALUE),
	m_pErro(nullptr)
{
	throw std::runtime_error("A device name was not provided.");
}

BaseDriverClient::~BaseDriverClient()
{
	if (m_hFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileHandle);
		m_hFileHandle = INVALID_HANDLE_VALUE;
	}
}

_Use_decl_annotations_
BOOL BaseDriverClient::SendIoControlRequest(
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
	return bStatus;
}


/// EOF