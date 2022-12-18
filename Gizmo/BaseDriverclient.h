// BaseDriverClient.h : This file is the base class for all driver interactions. Used by Gizmo.h.
//
#pragma once
#include <Windows.h>
#include "ErrorHandler.h"

class BaseDriverClient
{
public:
	BaseDriverClient();
	~BaseDriverClient();

protected:
	/// <summary>
	/// This method is a wrapper around CreateFileW.
	/// </summary>
	/// <param name="testing"></param>
	/// <returns>TRUE. If this method fails, reference GetLastError().</returns>
	BOOL init(const wchar_t* testing);

	/// <summary>
	/// This method is a wrapper for DeviceIoControlFile.
	/// </summary>
	/// <param name="dwIoControlCode"></param>
	/// <param name="lpInBuffer"></param>
	/// <param name="dwInBufferSize"></param>
	/// <param name="lpOutBuffer"></param>
	/// <param name="dwOutBufferSize"></param>
	/// <param name="lpBytesReturned"></param>
	/// <returns>true. If this method fails, reference GetLastError().</returns>
	BOOL SendIoControlRequest(
		_In_ DWORD dwIoControlCode,
		_In_reads_bytes_opt_(dwInBufferSize) LPVOID lpInBuffer = nullptr,
		_In_ DWORD dwInBufferSize = NULL,
		_Out_writes_bytes_to_opt_(dwOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer = nullptr,
		_In_ DWORD dwOutBufferSize = NULL,
		_Out_opt_ LPDWORD lpBytesReturned = nullptr
	);

private:
	HANDLE m_hFileHandle;
	std::unique_ptr<ErrorHandler> m_pErro;

};


/// EOF