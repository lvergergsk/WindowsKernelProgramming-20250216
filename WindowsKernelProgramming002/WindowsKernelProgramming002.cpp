#include <ntddk.h>
#include "ProcessPowerCommon.h"

void ProcessPowerUnload(PDRIVER_OBJECT);
NTSTATUS ProcessPowerCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcessPowerDeviceControl(PDEVICE_OBJECT, PIRP);

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("ProcessPower: DriverEntry\n"));
	KdPrint(("Registry path: %wZ\n", RegistryPath));

	DriverObject->DriverUnload = ProcessPowerUnload;

	RTL_OSVERSIONINFOW osInfo = { sizeof(osInfo) };
	NTSTATUS status = RtlGetVersion(&osInfo);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed in RtlGetVersion (0x%X)\n", status));
		return status;
	}

	KdPrint(("Windows version: %u.%u.%u\n", osInfo.dwMajorVersion, osInfo.dwMinorVersion, osInfo.dwBuildNumber));

	DriverObject->MajorFunction[IRP_MJ_CREATE] = ProcessPowerCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ProcessPowerCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcessPowerDeviceControl;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\WindowsKernelProgramming002");
	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed in IoCreateDevice (0x%X)\n", status));
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WindowsKernelProgramming002");

	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed in IoCreateSymbolicLink (0x%X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	return STATUS_SUCCESS;
}

void ProcessPowerUnload(PDRIVER_OBJECT DriverObject)
{
	KdPrint(("ProcessPower: ProcessPowerUnload\n"));
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WindowsKernelProgramming002");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS ProcessPowerCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	KdPrint(("ProcessPower: ProcessPowerCreateClose\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS; // Do not return Irp->IoStatus.Status, it is poisoned after IoCompleteRequest.
}

NTSTATUS ProcessPowerDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	KdPrint(("ProcessPower: ProcessPowerDeviceControl\n"));
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
	auto& dic = stack->Parameters.DeviceIoControl;
	ULONG len = 0;
	switch (dic.IoControlCode) {
	case IOCTL_OPEN_PROCESS:
		if (dic.Type3InputBuffer == nullptr || Irp->UserBuffer == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		if (dic.InputBufferLength < sizeof(ProcessPowerInput) || dic.OutputBufferLength < sizeof(ProcessPowerOutput)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		auto input = (ProcessPowerInput*)dic.Type3InputBuffer;
		auto output = (ProcessPowerOutput*)Irp->UserBuffer;
		OBJECT_ATTRIBUTES attr = {0};
		InitializeObjectAttributes(&attr, nullptr, 0, nullptr, nullptr);
		CLIENT_ID cid = { 0 };
		cid.UniqueProcess = UlongToHandle(input->ProcessId);
		status = ZwOpenProcess(&output->hProcess, PROCESS_ALL_ACCESS, &attr, &cid);
		if (NT_SUCCESS(status))
		{
			len = sizeof(output);
		}
		break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}