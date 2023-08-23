#pragma once
#include <Windows.h>
#include "../dependencies/react.h"
#include <shared_mutex>

#define SIGNATURE_MAGIC 0xcde

class CSharedMemory;

using FnOnMessage = void(*)(CSharedMemory*);

enum EMessageID {
	MESSAGE_NONE = 0,
	MESSAGE_RECEIVED,
	MESSAGE_MAX
};

struct TSharedMemoryBlock {
	EMessageID MessageID = MESSAGE_NONE;
	BYTE* InputBuffer = 0;
	BYTE* OutputBuffer = 0;
	size_t sizeOfBuffer = 0;
};

struct TSignature {
	uintptr_t uSignature = 0;
	PVOID pSharedMemoryBlock = 0;
	PVOID pSelfBlock = 0;
};

class CSharedMemory {
public:

	~CSharedMemory() {
		Destroy();
	}

	bool Setup(size_t sizeMaxBuffer, FnOnMessage OnMessage, uintptr_t uMagic = SIGNATURE_MAGIC, DWORD dwMessageProcessDelay = 1);
	void Destroy();

private:
	TSignature* SignatureVirtual = 0;
	HANDLE hThreadHandle = INVALID_HANDLE_VALUE;
	DWORD dwThreadID = 0;
public:
	FnOnMessage fOnMessage = 0;
	DWORD dwMessageProcessDelay = 1;
public:
	TSharedMemoryBlock SharedMemoryBlock;
};

class CSharedMemoryInstructor 
{
public:

	~CSharedMemoryInstructor() {
		Destroy();
	}

	bool SendRmtMessage(PVOID Input, size_t sizeInput, PVOID Output=0,size_t sizeOutput=0, DWORD dwMaxWaitMilliseconds = 1000,DWORD dwCheckIntervalMilliseconds = 10);
	bool Connect(DWORD dwProcessID , uintptr_t uMagic = SIGNATURE_MAGIC);
	void Destroy();
private:
	bool WaitForResponse(DWORD dwMaxWaitMilliseconds = 1000, DWORD dwCheckIntervalMilliseconds = 10);
private:
	DWORD dwConnectedProcessID = 0;
	HANDLE hProcessHandle = INVALID_HANDLE_VALUE;
	std::shared_mutex Mutex;
private:
	PVOID BlockAddress = 0;
	PVOID InputBufferAddress = 0;
	PVOID OutputBufferAddress = 0;
	size_t sizeMaxBuffer = 0;
};