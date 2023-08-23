#include "sharedmemory.h"

DWORD WINAPI ListenerThread(CSharedMemory* SharedMemory)
{
	while (true)
	{
		if (SharedMemory->SharedMemoryBlock.MessageID != MESSAGE_NONE)
		{
			RtlZeroMemory(SharedMemory->SharedMemoryBlock.OutputBuffer, SharedMemory->SharedMemoryBlock.sizeOfBuffer);
			SharedMemory->fOnMessage(SharedMemory);
			RtlZeroMemory(SharedMemory->SharedMemoryBlock.InputBuffer, SharedMemory->SharedMemoryBlock.sizeOfBuffer);

			SharedMemory->SharedMemoryBlock.MessageID = MESSAGE_NONE;
		}

		Sleep(SharedMemory->dwMessageProcessDelay);
	}
}

bool CSharedMemory::Setup(size_t sizeMaxBuffer, FnOnMessage OnMessage ,uintptr_t uMagic, DWORD _dwMessageProcessDelay)
{
	fOnMessage = OnMessage;
	dwMessageProcessDelay = _dwMessageProcessDelay;

	SharedMemoryBlock.InputBuffer = (BYTE*)malloc(sizeMaxBuffer);
	SharedMemoryBlock.OutputBuffer = (BYTE*)malloc(sizeMaxBuffer);
	SharedMemoryBlock.sizeOfBuffer = sizeMaxBuffer;
	RtlZeroMemory(SharedMemoryBlock.InputBuffer, sizeMaxBuffer);
	RtlZeroMemory(SharedMemoryBlock.OutputBuffer, sizeMaxBuffer);

	SignatureVirtual = (TSignature*)VirtualAlloc(NULL, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	if (!SignatureVirtual) {
		Destroy();
		return false;
	}

	SignatureVirtual->uSignature = uMagic;
	SignatureVirtual->pSharedMemoryBlock = &SharedMemoryBlock;
	SignatureVirtual->pSelfBlock = &SignatureVirtual;

	hThreadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&ListenerThread, this, NULL, &dwThreadID);

	if (hThreadHandle == INVALID_HANDLE_VALUE)
	{
		Destroy();
		return false;
	}

	return true;
}


void CSharedMemory::Destroy() {

	if (hThreadHandle != INVALID_HANDLE_VALUE)
	{
		TerminateThread(hThreadHandle,EXIT_SUCCESS);
		hThreadHandle = INVALID_HANDLE_VALUE;
	}

	if (SharedMemoryBlock.InputBuffer) {
		free(SharedMemoryBlock.InputBuffer);
		SharedMemoryBlock.InputBuffer = nullptr;
	}

	if (SharedMemoryBlock.OutputBuffer) {
		free(SharedMemoryBlock.OutputBuffer);
		SharedMemoryBlock.OutputBuffer = nullptr;
	}

	if (SignatureVirtual) {
		VirtualFree(SignatureVirtual, 0, MEM_RELEASE);
		SignatureVirtual = nullptr;
	}
}

