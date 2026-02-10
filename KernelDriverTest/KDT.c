#pragma warning(disable: 4100)
#include "KDT.h"
#include "messages.h"

#define IOCTL_PING CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\KDT"); // Create the device
UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\DosDevices\\KDT"); // Create the symbolic link to the device
PDEVICE_OBJECT g_DeviceObject = NULL;


// Called when the driver is unloaded
NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject)
{
    IoDeleteSymbolicLink(&symlink);
    IoDeleteDevice(g_DeviceObject);
    DebugMessage("Kernel Driver Test: Unloaded\n");
    return STATUS_SUCCESS;
}


// Handles IOCTL requests
NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG outLen = stack->Parameters.DeviceIoControl.OutputBufferLength;

    if (code == IOCTL_PING)
    {
        const char* reply = "pong";
        size_t replyLen = strlen(reply) + 1;

        
        if (Irp->AssociatedIrp.SystemBuffer && outLen >= replyLen)
        {
            RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, reply, replyLen);
            Irp->IoStatus.Information = (ULONG)replyLen;
            Irp->IoStatus.Status = STATUS_SUCCESS;
        }
        else
        {
            Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Information = 0;
        }
    }
    else
    {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Irp->IoStatus.Status;
}

// Handles all other IRP major functions
NTSTATUS Dispatch(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// Entry point of the driver
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject, PUNICODE_STRING pRegistryPath)
{
    UNREFERENCED_PARAMETER(pRegistryPath);

    NTSTATUS status = IoCreateDevice(
        pDriverObject,
        0,
        &deviceName,
        FILE_DEVICE_UNKNOWN,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &g_DeviceObject
    );

    if (!NT_SUCCESS(status)) return status;
    status = IoCreateSymbolicLink(&symlink, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(g_DeviceObject);
        return status;
    }

    // Set all dispatch routines to a generic handler
    for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        pDriverObject->MajorFunction[i] = Dispatch;

    // Set the IOCTL and unload handlers
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoControl;
    pDriverObject->DriverUnload = UnloadDriver;

    DebugMessage("Kernel Driver Test: Loaded");
    return STATUS_SUCCESS;

}
