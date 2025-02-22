#include <Windows.h>
#include <stdio.h>
int main()
{
	// HANDLE hDevice = CreateFile(L"\\\\.\\HelloWorld", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	HANDLE hDevice = CreateFile(L"\\\\.\\procexp152", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("CreateFile failed with error code %d\n", GetLastError());
		return 1;
	}
	else {
		printf("CreateFile success\n");
		CloseHandle(hDevice);
	}
	return 0;
}
