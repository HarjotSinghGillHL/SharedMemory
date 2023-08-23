#pragma once

typedef enum _MEMORY_INFORMATION_CLASS {
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName,
	MemoryBasicVlmInformation,
	MemoryWorkingSetExList,
} MEMORY_INFORMATION_CLASS;


using FnNtQueryVirtualMemory = NTSTATUS(NTAPI*)(HANDLE, PVOID, MEMORY_INFORMATION_CLASS, PVOID, SIZE_T, PSIZE_T);