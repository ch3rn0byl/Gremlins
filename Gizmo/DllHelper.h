// DllHelper.h : This file is used in NtUndoc.h.
//
#pragma once
#include <type_traits>
#include <Windows.h>

//
// I was just a mere peasant before discovering RAII for GetProcAddress. Big
// thanks to the following blogpost:
// https://blog.benoitblanchon.fr/getprocaddress-like-a-boss/
//
class ProcPtr
{
public:
	explicit ProcPtr(FARPROC ptr) :
		m_ptr(ptr)
	{}

	template <typename T, typename = std::enable_if_t<std::is_function_v<T>>>
	operator T* () const
	{
		return reinterpret_cast<T*>(m_ptr);
	}

private:
	FARPROC m_ptr;
};

class DllHelper
{
public:
	explicit DllHelper(LPCTSTR filename) :
		m_hModule(LoadLibrary(filename))
	{}

	~DllHelper()
	{
		FreeLibrary(m_hModule);
	}

	ProcPtr operator[](LPCSTR proc_name) const {
		return ProcPtr(GetProcAddress(m_hModule, proc_name));
	}

private:
	HMODULE m_hModule;
};


/// EOF