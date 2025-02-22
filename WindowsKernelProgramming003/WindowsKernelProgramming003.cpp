#include <Windows.h>
#include <stdio.h>

int main()
{
	HANDLE hDevice = CreateFile(L"\\\\.\\ProcessPower", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed in CreateFile (%u)\n", GetLastError());
		return 1;
	}

	CloseHandle(hDevice);
}
