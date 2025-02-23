#include <ntifs.h> // Need to be before ntddk.h
#include <ntddk.h>
#include "BoosterCommon.h"

void BoosterUnload(PDRIVER_OBJECT);
NTSTATUS BoosterCreateClose(PDEVICE_OBJECT, PIRP);
NTSTATUS BoosterDeviceControl(PDEVICE_OBJECT, PIRP);

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	KdPrint(("Booster: DriverEntry\n"));
	DriverObject->DriverUnload = BoosterUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = BoosterDeviceControl;

	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\WindowsKernelProgramming004");
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed in IoCreateDevice (0x%X)\n", status));
		return status;
	}

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WindowsKernelProgramming004");
	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Failed in IoCreateSymbolicLink (0x%X)\n", status));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	return STATUS_SUCCESS;
}

void BoosterUnload(PDRIVER_OBJECT DriverObject)
{
	KdPrint(("Booster: BoosterUnload\n"));
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\WindowsKernelProgramming004");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS BoosterCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS BoosterDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	auto& dic = stack->Parameters.DeviceIoControl;
	switch (dic.IoControlCode) {
	case IOCTL_SET_PRIORITY:
		if (dic.InputBufferLength < sizeof(ThreadData))
		{
			Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}
		ThreadData* data = (ThreadData*)Irp->AssociatedIrp.SystemBuffer;
		if (data == nullptr)
		{
			Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			break;
		}
		if (data->Priority < 1 || data->Priority > 31)
		{
			Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
			break;
		}
		PETHREAD Thread;
		status = PsLookupThreadByThreadId(UlongToHandle(data->ThreadId), &Thread);
		if (!NT_SUCCESS(status)) {
			KdPrint(("Failed in PsLookupThreadByThreadId (0x%X)\n", status));
			break;
		}
		KeSetPriorityThread((PKTHREAD)Thread, data->Priority);
		ObDereferenceObject(Thread);
		break;
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}