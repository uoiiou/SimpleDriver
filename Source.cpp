#include <ntddk.h>
#include <wdmsec.h>
#include <strsafe.h>
#include <tchar.h>  

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, _In_ PUNICODE_STRING RegistryPath);
VOID UnloadRoutine(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS StartProcessCallback();
VOID PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create);

#define NT_DEVICE_NAME		L"\\Device\\VirtualProcessNotifier"

PDEVICE_OBJECT g_DeviceObj;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS Status;
	UNICODE_STRING NtDeviceName;

	DbgPrint("Driver Start!\n");
	RtlInitUnicodeString(&NtDeviceName, NT_DEVICE_NAME);
	Status = IoCreateDevice(DriverObject, 0, &NtDeviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &g_DeviceObj);
	if (!NT_SUCCESS(Status)) 
	{
		return Status;
	}
	
	DriverObject->DriverUnload = UnloadRoutine;
	Status = StartProcessCallback();

	return STATUS_SUCCESS;
}

NTSTATUS StartProcessCallback()
{
	NTSTATUS Status = PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, FALSE);

	if (!NT_SUCCESS(Status)) 
	{
		DbgPrint("StartProcessCallback. ERROR PsSetCreateProcessNotifyRoutine\n");
		return Status;
	}

	return STATUS_SUCCESS;
}

VOID PcreateProcessNotifyRoutine(HANDLE ParentId, HANDLE ProcessId, BOOLEAN Create)
{
	UNREFERENCED_PARAMETER(ParentId);

	if (Create)
		DbgPrint("Process Create: %d", (int)ProcessId);
}

VOID UnloadRoutine(_In_ PDRIVER_OBJECT DriverObject)
{
	NTSTATUS Status = PsSetCreateProcessNotifyRoutine(PcreateProcessNotifyRoutine, TRUE);
	if (!NT_SUCCESS(Status)) 
	{
		DbgPrint("UnloadRoutine. PsSetCreateProcessNotifyRoutine ERROR\n");
		return;
	}
	IoDeleteDevice(DriverObject->DeviceObject);
	DbgPrint("Driver End!\n");
}
