#include <ntddk.h>

VOID UnloadDriver(PDRIVER_OBJECT DriverObject);

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
    PVOID allocatedMemory = NULL;
    const char* message = "Hello, Kernel World!";

    UNREFERENCED_PARAMETER(RegistryPath);
    DriverObject->DriverUnload = UnloadDriver;

    allocatedMemory = ExAllocatePool2(POOL_FLAG_NON_PAGED, 1024, 'Mem1');
    if (allocatedMemory == NULL)
    {
        DbgPrint("Memory allocation failed\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(allocatedMemory, message, strlen(message) + 1);
    DbgPrint("Memory content: %s\n", (char*)allocatedMemory);

    ExFreePoolWithTag(allocatedMemory, 'Mem1');
    return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
    KdPrint(("006 UnloadDriver\n"));
}
