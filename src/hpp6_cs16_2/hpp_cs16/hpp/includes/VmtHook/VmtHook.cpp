#include "framework.h"

bool VHookTable::HookTable(DWORD dwTablePtrPtr)
{
	dwPtrPtrTable = dwTablePtrPtr;
	dwPtrOldTable = *(PDWORD)dwPtrPtrTable;

	DWORD dwIndexFunction;

	for (dwIndexFunction = 0; ((PDWORD) * (PDWORD)dwTablePtrPtr)[dwIndexFunction]; ++dwIndexFunction)
	{
		if (IsBadCodePtr((FARPROC)((PDWORD) * (PDWORD)dwTablePtrPtr)[dwIndexFunction]))
			break;
	}

	dwSizeTable = sizeof(DWORD) * dwIndexFunction;

	if (dwIndexFunction && dwSizeTable)
	{
		dwPtrNewTable = (DWORD)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSizeTable);
		RtlCopyMemory((PVOID)dwPtrNewTable, (PVOID) * (PDWORD)dwTablePtrPtr, dwSizeTable);

		*(PDWORD)dwTablePtrPtr = dwPtrNewTable;

		return true;
	}

	return false;
}

void VHookTable::HookIndex(DWORD dwIndex, PVOID pAddress)
{
	((PDWORD)dwPtrNewTable)[dwIndex] = (DWORD)pAddress;
}

DWORD VHookTable::RetHookIndex(DWORD dwIndex, PVOID pAddress)
{
	DWORD dwOld = ((PDWORD)dwPtrNewTable)[dwIndex];
	((PDWORD)dwPtrNewTable)[dwIndex] = (DWORD)pAddress;
	return dwOld;
}

void VHookTable::UnHook()
{
	if (dwPtrPtrTable)
		*(PDWORD)dwPtrPtrTable = dwPtrOldTable;
}

void VHookTable::ReHook()
{
	if (dwPtrPtrTable)
		*(PDWORD)dwPtrPtrTable = dwPtrNewTable;
}