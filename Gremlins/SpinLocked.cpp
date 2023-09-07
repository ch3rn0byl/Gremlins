#include "SpinLocked.h"

SpinLocked::SpinLocked(KSPIN_LOCK SpinLock) :
	m_kSpinLock(SpinLock)
{
	KeAcquireSpinLock(&m_kSpinLock, &m_OldIrql);
}

SpinLocked::SpinLocked()
{
	KeInitializeSpinLock(&m_kSpinLock);

	KeAcquireSpinLock(&m_kSpinLock, &m_OldIrql);
}

SpinLocked::~SpinLocked()
{
	KeReleaseSpinLock(&m_kSpinLock, m_OldIrql);
}


/// EOF