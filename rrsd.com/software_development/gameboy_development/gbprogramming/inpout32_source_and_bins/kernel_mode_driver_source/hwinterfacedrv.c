#include <ntddk.h>
#include "stdio.h"
#include "hwinterfacedrv.h"


NTSTATUS hwinterfaceDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
VOID hwinterfaceUnload(IN PDRIVER_OBJECT DriverObject);

NTSTATUS hwinterfaceCreateDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{


    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    WCHAR NameBuffer[] = L"\\Device\\hwinterface";
    WCHAR DOSNameBuffer[] = L"\\DosDevices\\hwinterface";
    UNICODE_STRING uniNameString, uniDOSString;

    RtlInitUnicodeString(&uniNameString, NameBuffer);
    RtlInitUnicodeString(&uniDOSString, DOSNameBuffer);

    status = IoCreateDevice(DriverObject, 
                            0,
                            &uniNameString,
                            FILE_DEVICE_UNKNOWN,
                            0, 
                            FALSE, 
                            &deviceObject);

    if(!NT_SUCCESS(status))
        return status;

    status = IoCreateSymbolicLink (&uniDOSString, &uniNameString);

    if (!NT_SUCCESS(status))
        return status;

    DriverObject->MajorFunction[IRP_MJ_CREATE] = hwinterfaceCreateDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = hwinterfaceDeviceControl;
    DriverObject->DriverUnload = hwinterfaceUnload;

    return STATUS_SUCCESS;
}

NTSTATUS
hwinterfaceDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp
    )

{
    PIO_STACK_LOCATION  stkloc;
    NTSTATUS            ntStatus = STATUS_SUCCESS; 
	UCHAR				Value;
    PUCHAR              data; 
    PUSHORT             address;
	ULONG               inBuffersize;   
    ULONG               outBuffersize;  
    ULONG               inBuf;         
    PVOID               CtrlBuff;
 

    stkloc = IoGetCurrentIrpStackLocation( pIrp );
    inBuffersize = stkloc->Parameters.DeviceIoControl.InputBufferLength;
    outBuffersize = stkloc->Parameters.DeviceIoControl.OutputBufferLength;

    CtrlBuff    = pIrp->AssociatedIrp.SystemBuffer;

    data  = (PUCHAR) CtrlBuff;
    address = (PUSHORT) CtrlBuff;


    switch ( stkloc->Parameters.DeviceIoControl.IoControlCode )
     {
  
      case IOCTL_READ_PORT_UCHAR:
 
            if ((inBuffersize >= 2) && (outBuffersize >= 1)) 
			{
				
                (UCHAR)Value = READ_PORT_UCHAR((PUCHAR)address[0]);
				data[0]= Value;
            } 
			else 
			{	
			ntStatus = STATUS_BUFFER_TOO_SMALL;
			}



			pIrp->IoStatus.Information = 1; 
            ntStatus = STATUS_SUCCESS;

            break;

      case IOCTL_WRITE_PORT_UCHAR:
            if (inBuffersize >= 3) 
			{
                WRITE_PORT_UCHAR((PUCHAR)address[0], data[2]);
				pIrp->IoStatus.Information = 10;
            } 
			else 
			{
			ntStatus = STATUS_BUFFER_TOO_SMALL;
            pIrp->IoStatus.Information = 0; 
            ntStatus = STATUS_SUCCESS;
			}
            break;

      default:
            ntStatus = STATUS_UNSUCCESSFUL;
            pIrp->IoStatus.Information = 0;
            break;

    }
    pIrp->IoStatus.Status = ntStatus;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return ntStatus;
}

VOID hwinterfaceUnload(IN PDRIVER_OBJECT DriverObject)
{
    WCHAR DOSNameBuffer[] = L"\\DosDevices\\hwinterface";
    UNICODE_STRING uniDOSString;

    RtlInitUnicodeString(&uniDOSString, DOSNameBuffer);
    IoDeleteSymbolicLink (&uniDOSString);
    IoDeleteDevice(DriverObject->DeviceObject);
}