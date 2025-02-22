#include <ntddk.h>

void ProcessPowerUnload(PDRIVER_OBJECT);
NTSTATUS ProcessPowerCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS ProcessPowerDeviceControl(PDEVICE_OBJECT, PIRP);

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
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
	//DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ProcessPowerDeviceControl;

	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\ProcessPower");
	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed in IoCreateDevice (0x%X)\n", status));
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\ProcessPower");

	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(DeviceObject);
		KdPrint(("Failed in IoCreateSymbolicLink (0x%X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	return STATUS_SUCCESS;
}

void ProcessPowerUnload(PDRIVER_OBJECT DriverObject)
{
	KdPrint(("ProcessPower: ProcessPowerUnload\n"));
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\ProcessPower");
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
