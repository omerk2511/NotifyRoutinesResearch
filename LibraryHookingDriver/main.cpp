#include <ntifs.h>

#include "config.h"
#include "new.h"
#include "delete.h"
#include "notify_routines.h"
#include "new_processes_list.h"

void driver_unload(PDRIVER_OBJECT driver_object);

NewProcessesList* g_new_processes_list = nullptr;
volatile LONG64 g_apc_count = 0;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = driver_unload;

	g_new_processes_list = new (PagedPool, config::kDriverTag) NewProcessesList();

	if (!g_new_processes_list) {
		KdPrint(("[-] Failed to create a remote thread creation list.\n"));

		return STATUS_INSUFFICIENT_RESOURCES;
	}

	auto status = ::PsSetCreateProcessNotifyRoutine(notify_routines::create_process, FALSE);

	if (!NT_SUCCESS(status)) {
		delete g_new_processes_list;

		KdPrint(("[-] Failed to create a process notify routine.\n", status));

		return status;
	}

	status = ::PsSetCreateThreadNotifyRoutine(notify_routines::create_thread);

	if (!NT_SUCCESS(status)) {
		::PsSetCreateProcessNotifyRoutine(notify_routines::create_process, TRUE);
		delete g_new_processes_list;

		KdPrint(("[-] Failed to create a thread notify routine.\n"));

		return status;
	}

	KdPrint(("[+] Loaded LibraryHookingDriver successfully.\n"));

	return STATUS_SUCCESS;
}

void driver_unload(PDRIVER_OBJECT)
{
	::PsSetCreateProcessNotifyRoutine(notify_routines::create_process, TRUE);
	::PsRemoveCreateThreadNotifyRoutine(notify_routines::create_thread);

	delete g_new_processes_list;

	while (g_apc_count > 0);

	KdPrint(("[+] Unloaded LibraryHookingDriver successfully.\n"));
}