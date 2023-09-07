#pragma once
#include <ntddk.h>

class SpinLocked
{
public:
	SpinLocked(
		_In_ KSPIN_LOCK SpinLock
	);

	SpinLocked();
	~SpinLocked();

	
private:
	KSPIN_LOCK m_kSpinLock;
	KIRQL m_OldIrql;

};


/// EOF