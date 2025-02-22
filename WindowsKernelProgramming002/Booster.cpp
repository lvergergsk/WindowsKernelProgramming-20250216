#include <ntddk.h>

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("DriverEntry called\n"));

	DriverObject->DriverUnload = BoosterUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoosterCreateClose;

	PDEVICE_OBJECT DeviceObject = nullptr;
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\Booster");
	NTSTATUS Status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject); 
	if (!NT_SUCCESS(Status))
	{
		KdPrint(("Failed to create device object (0x%08X)\n", Status));
		return Status;
	}
	return STATUS_SUCCESS;
}