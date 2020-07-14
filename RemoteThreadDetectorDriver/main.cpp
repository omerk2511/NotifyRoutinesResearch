#include <ntddk.h>

#include "config.h"
#include "new.h"
#include "delete.h"
#include "driver_io.h"
#include "notify_routines.h"
#include "remote_thread_creation_list.h"
#include "new_processes_cache.h"

void driver_unload(PDRIVER_OBJECT driver_object);

RemoteThreadCreationList* g_remote_thread_creation_list;
NewProcessesCache* g_new_processes_cache;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = driver_unload;

	driver_object->MajorFunction[IRP_MJ_CREATE] = driver_io::create_close;
	driver_object->MajorFunction[IRP_MJ_CLOSE] = driver_io::create_close;
	driver_object->MajorFunction[IRP_MJ_READ] = driver_io::read;

	UNICODE_STRING device_name;
	::RtlInitUnicodeString(&device_name, config::kDeviceName);

	PDEVICE_OBJECT device_object;

	auto status = ::IoCreateDevice(
		driver_object,
		0,
		&device_name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		true,
		&device_object
	);

	device_object->Flags |= DO_DIRECT_IO;

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed to create a device object.\n"));

		return status;
	}

	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, config::kSymbolicLink);

	status = ::IoCreateSymbolicLink(
		&symbolic_link,
		&device_name
	);

	if (!NT_SUCCESS(status)) {
		::IoDeleteDevice(device_object);

		KdPrint(("[-] Failed to create a symbolic link.\n"));

		return status;
	}

	status = ::PsSetCreateProcessNotifyRoutineEx(notify_routines::create_process, FALSE);

	if (!NT_SUCCESS(status)) {
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(&symbolic_link);

		KdPrint(("[-] Failed to create a process notify routine.\n", status));

		return status;
	}

	status = ::PsSetCreateThreadNotifyRoutine(notify_routines::create_thread);

	if (!NT_SUCCESS(status)) {
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(&symbolic_link);
		::PsSetCreateProcessNotifyRoutineEx(notify_routines::create_process, TRUE);

		KdPrint(("[-] Failed to create a thread notify routine.\n"));

		return status;
	}

	g_remote_thread_creation_list = new (PagedPool, config::kDriverTag) RemoteThreadCreationList();

	if (!g_remote_thread_creation_list) {
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(&symbolic_link);
		::PsSetCreateProcessNotifyRoutineEx(notify_routines::create_process, TRUE);
		::PsRemoveCreateThreadNotifyRoutine(notify_routines::create_thread);

		KdPrint(("[-] Failed to create a remote thread creation list.\n"));

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	g_new_processes_cache = new (PagedPool, config::kDriverTag) NewProcessesCache();

	if (!g_new_processes_cache) {
		::IoDeleteDevice(device_object);
		::IoDeleteSymbolicLink(&symbolic_link);
		::PsSetCreateProcessNotifyRoutineEx(notify_routines::create_process, TRUE);
		::PsRemoveCreateThreadNotifyRoutine(notify_routines::create_thread);

		delete g_remote_thread_creation_list;

		KdPrint(("[-] Failed to create a new processes cache.\n"));

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	KdPrint(("[+] Loaded RemoteThreadDetector successfully.\n"));

	return STATUS_SUCCESS;
}

void driver_unload(PDRIVER_OBJECT driver_object)
{
	UNICODE_STRING symbolic_link;
	::RtlInitUnicodeString(&symbolic_link, config::kSymbolicLink);

	::IoDeleteDevice(driver_object->DeviceObject);
	::IoDeleteSymbolicLink(&symbolic_link);

	::PsSetCreateProcessNotifyRoutineEx(notify_routines::create_process, TRUE);
	::PsRemoveCreateThreadNotifyRoutine(notify_routines::create_thread);

	delete g_remote_thread_creation_list;
	delete g_new_processes_cache;

	KdPrint(("[+] Unloaded RemoteThreadDetector successfully.\n"));
}