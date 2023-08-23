#include "sharedmemory/sharedmemory.h"
#include <iostream>

void OnMessage(CSharedMemory* Memory) {   //Handle messages here
	std::cout << "Message Received : " << (char*)Memory->SharedMemoryBlock.InputBuffer  << "\n";
	*(bool*)Memory->SharedMemoryBlock.OutputBuffer = true;
	std::cout << "Output Sent : " << *(bool*)Memory->SharedMemoryBlock.OutputBuffer << "\n";
}

// In this demonstration i have used the local process / current process as target process and instructor at the same time. 
int main() {

	std::cout << "Start\n";
	CSharedMemory SharedMemory;  // CSharedMemory should only be used in remote process as for allocation purpose
	bool bReturn = SharedMemory.Setup(0x1000, OnMessage);  // Create shared memory object with 0x1000 bytes input / output buffers in remote process
	;
	if (bReturn) {
		CSharedMemoryInstructor Instructor;                // Instructor should only be used in local process to communicate with remote process 
		bReturn = Instructor.Connect(GetCurrentProcessId());  // Put remote process ID

		if (bReturn) {

			std::cout << "Connect : " << bReturn << "\n";

			char Message[] = "Test string";                 // Test message to send
			bool bReceiveMessage = false;

			bReturn = Instructor.SendRmtMessage(&Message, sizeof(Message), &bReceiveMessage, sizeof(bReceiveMessage));    // Send message to remote process

			std::cout << "Output : " << bReceiveMessage << "\n";           // Output received
			std::cout << "Message Success : " << bReturn << "\n";           // Message status

			Instructor.Destroy();                              // Destroy instance
		}
		else
			std::cout << "Failed to connect\n";
	}
	else
		std::cout << "Failed to setup\n";


	system("pause");
}