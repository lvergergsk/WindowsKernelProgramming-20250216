#include <Windows.h>
#include <stdio.h>
#include "..\WindowsKernelProgramming004\BoosterCommon.h"

int main(int argc ,const char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <tid> <priority>\n", argv[0]);
		return 1;
	}
	int tid = atoi(argv[1]);
	int priority = atoi(argv[2]);

	HANDLE hDevice = CreateFile(L"\\\\.\\WindowsKernelProgramming004", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE) {
		printf("Error opening device (%u)\n", GetLastError());
		return 1;
	}

	ThreadData data;
	data.Priority = priority;
	data.ThreadId = tid;
	DWORD bytes;
	BOOL ok = DeviceIoControl(hDevice, IOCTL_SET_PRIORITY, &data, sizeof(data), nullptr, 0, &bytes, nullptr);

	if (!ok) {
		printf("Error in DeviceIoControl (%u)\n", GetLastError());
		return 1;
	}
	
	printf("Success!\n");

	CloseHandle(hDevice);
}
