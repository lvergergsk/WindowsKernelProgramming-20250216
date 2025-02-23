#include <Windows.h>
#include <stdio.h>
#include "..\WindowsKernelProgramming002\ProcessPowerCommon.h"
#include <Psapi.h>

static void DumpProcessModules(HANDLE hProcess)
{
	HMODULE hModules[4096];
	DWORD bytesNeeded;
	if (!EnumProcessModulesEx(hProcess, hModules, sizeof(hModules), &bytesNeeded, LIST_MODULES_ALL))
	{
		printf("Failed in EnumProcessModules (%u)\n", GetLastError());
		return;
	}

	DWORD numModules = bytesNeeded / sizeof(HMODULE);
	printf("Number of modules: %u\n", numModules);

	WCHAR name[MAX_PATH];
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	for (unsigned int i = 0; i < numModules; i++)
	{
		if (GetModuleBaseName(hProcess, hModules[i], name, _countof(name)))
		{
			MODULEINFO modInfo;
			if (GetModuleInformation(hProcess, hModules[i], &modInfo, sizeof(modInfo)))
			{
				BOOL is32Bit = (modInfo.EntryPoint < sysInfo.lpMaximumApplicationAddress);
				printf("Module 0x%p : %ws (%s-bit)\n", hModules[i], name, is32Bit ? "32" : "64");
			}
		}
		else
		{
			printf("Failed in GetModuleBaseName for module 0x%p (%u)\n", hModules[i], GetLastError());
		}
		printf("\n");
	}
}


int main(int argc, const char* argv[])
{
	if (argc < 2) {
		printf("Usage: %s <process id>\n", argv[0]);
		return 1;
	}
	int	pid = atoi(argv[1]);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (hProcess)
	{
		DumpProcessModules(hProcess);
		return 0;
	}
	else {
		printf("Failed in OpenProcess (%u)\n", GetLastError());
	}

	HANDLE hDevice = CreateFile(L"\\\\.\\ProcessPower", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		printf("Failed in CreateFile (%u)\n", GetLastError());
		return 1;
	}

	ProcessPowerInput input;
	input.ProcessId = pid;

	ProcessPowerOutput output;
	DWORD bytesReturned;
	BOOL ok = DeviceIoControl(hDevice, IOCTL_OPEN_PROCESS, &input, sizeof(input), &output, sizeof(output), &bytesReturned, nullptr);
	if (!ok)
	{
		printf("Failed in DeviceIoControl (%u)\n", GetLastError());
		CloseHandle(hDevice);
		return 1;
	}
	printf("Success!\n");

	DumpProcessModules(output.hProcess);

	CloseHandle(output.hProcess);
	CloseHandle(hDevice);

}
