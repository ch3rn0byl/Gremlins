#include "ProcessCallbacks.h"

_Use_decl_annotations_
void PCreateProcessNotifyRoutine(
	HANDLE ParentId, 
	HANDLE ProcessId,
	BOOLEAN Create
)
{
	UNREFERENCED_PARAMETER(ParentId);
	UNREFERENCED_PARAMETER(ProcessId);

	if (Create)
	{
		
	}
	else
	{

	}
}


/// EOF