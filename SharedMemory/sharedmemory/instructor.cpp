#include "sharedmemory.h"
#include <iostream>

bool CSharedMemoryInstructor::WaitForResponse(DWORD dwMaxWaitMilliseconds , DWORD dwCheckIntervalMilliseconds) {
	
	EMessageID CurrentMessageID = MESSAGE_RECEIVED;
	DWORD dwWait = 0;
	size_t sizeBytes = 0;

	while (CurrentMessageID != MESSAGE_NONE && dwWait <= dwMaxWaitMilliseconds)
	{
		if (!ReadProcessMemory(hProcessHandle, (PVOID)((uintptr_t)BlockAddress + FIELD_OFFSET(TSharedMemoryBlock, MessageID)), &CurrentMessageID, sizeof(EMessageID), &sizeBytes))
			return false;

		if (CurrentMessageID == MESSAGE_NONE)
			return true;

		Sleep(dwCheckIntervalMilliseconds);
		dwWait += dwCheckIntervalMilliseconds;
	}

	return false;
}

bool CSharedMemoryInstructor::SendRmtMessage(PVOID Input, size_t sizeInput, PVOID Output, size_t sizeOutput, DWORD dwMaxWaitMilliseconds , DWORD dwCheckIntervalMilliseconds) {

	if (!Input || !sizeInput)
		return false;

	std::unique_lock<std::shared_mutex> LockGuard(Mutex);
	
	size_t sizeBytes = 0;
	PVOID NullPointer = nullptr;

	if (!WriteProcessMemory(hProcessHandle, InputBufferAddress, Input, sizeInput, &sizeBytes))
		return false;

	EMessageID MessageSend = MESSAGE_RECEIVED;

	if (!WriteProcessMemory(hProcessHandle,(PVOID)((uintptr_t)BlockAddress + FIELD_OFFSET(TSharedMemoryBlock, MessageID)), &MessageSend, sizeof(EMessageID), &sizeBytes))
		return false;

	if (!WaitForResponse(dwMaxWaitMilliseconds, dwCheckIntervalMilliseconds))
		return false;

	if (!Output || !sizeOutput)
		return true;

	if (!ReadProcessMemory(hProcessHandle, OutputBufferAddress, Output, sizeOutput, &sizeBytes))
		return false;
	

	return true;
}

bool CSharedMemoryInstructor::Connect(DWORD dwProcessID, uintptr_t uMagic)
{
	FnNtQueryVirtualMemory NtQueryVirtualMemory = (FnNtQueryVirtualMemory)GetProcAddress(GetModuleHandleA("ntdll"), "NtQueryVirtualMemory");

	if (!NtQueryVirtualMemory)
		return false;

	dwConnectedProcessID = dwProcessID;
	hProcessHandle = OpenProcess(PROCESS_ALL_ACCESS,false,dwConnectedProcessID);

	if (hProcessHandle == INVALID_HANDLE_VALUE)
		return false;

	MEMORY_BASIC_INFORMATION BasicInformation = { 0 };

	NTSTATUS Status = 0;
	size_t sizeBytes = 0;

	for (PVOID pCurrentBase = 0x0; pCurrentBase >= (PVOID)0x0; pCurrentBase = (PVOID)((uintptr_t)BasicInformation.BaseAddress + BasicInformation.RegionSize))
	{
		Status = NtQueryVirtualMemory(hProcessHandle, pCurrentBase, MemoryBasicInformation, &BasicInformation, sizeof(BasicInformation), &sizeBytes);

		if (Status < 0)
			break;

		if (BasicInformation.RegionSize < sizeof(TSignature) )
			continue;

		TSignature Signature;
		size_t BytesRead;

		if (!ReadProcessMemory(hProcessHandle, BasicInformation.AllocationBase, &Signature, sizeof(TSignature),&BytesRead)) 
			continue;

		if (Signature.uSignature != uMagic)
			continue;

		if (!ReadProcessMemory(hProcessHandle, (PVOID)((uintptr_t)Signature.pSharedMemoryBlock + FIELD_OFFSET(TSharedMemoryBlock, InputBuffer)), &InputBufferAddress, sizeof(PVOID), &BytesRead))
			continue;

		if (!ReadProcessMemory(hProcessHandle, (PVOID)((uintptr_t)Signature.pSharedMemoryBlock + FIELD_OFFSET(TSharedMemoryBlock, OutputBuffer)), &OutputBufferAddress, sizeof(PVOID), &BytesRead))
			continue;

		if (!ReadProcessMemory(hProcessHandle, (PVOID)((uintptr_t)Signature.pSharedMemoryBlock + FIELD_OFFSET(TSharedMemoryBlock, sizeOfBuffer)), &sizeMaxBuffer, sizeof(size_t), &BytesRead))
			continue;

		BlockAddress = Signature.pSharedMemoryBlock;
		return true;
	}

	Destroy();
	return false;
}

void CSharedMemoryInstructor::Destroy()
{
	if (hProcessHandle == INVALID_HANDLE_VALUE)
		return;

	CloseHandle(hProcessHandle);
	dwConnectedProcessID = 0;
	BlockAddress = 0;
	InputBufferAddress = 0;
	OutputBufferAddress = 0;
	sizeMaxBuffer = 0;
	hProcessHandle = INVALID_HANDLE_VALUE;
}