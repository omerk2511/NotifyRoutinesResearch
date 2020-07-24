#include <ntddk.h>
#include <aux_klib.h>

#pragma comment(lib, "aux_klib.lib")

#include "config.h"
#include "undocumented.h"
#include "helpers.h"

void driver_unload(PDRIVER_OBJECT driver_object);

undocumented::PSP_CALLBACK_OBJECT** g_PspCreateThreadNotifyRoutine;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = driver_unload;

	auto status = ::AuxKlibInitialize();

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed loading NotifyRoutineEnumeration: could not initialize AuxKlib.\n"));
		return status;
	}

	g_PspCreateThreadNotifyRoutine = helpers::find_PspCreateThreadNotifyRoutine();

	if (!g_PspCreateThreadNotifyRoutine) {
		KdPrint(("[-] Failed loading NotifyRoutineEnumeration: could not find PspCreateThreadNotifyRoutine.\n"));
		return STATUS_NOT_FOUND;
	}

	KdPrint(("PspCreateThreadNotifyRoutine is @0x%p.\n", g_PspCreateThreadNotifyRoutine));

	KdPrint(("[+] Loaded NotifyRoutineEnumerationDriver successfully.\n"));

	return STATUS_SUCCESS;
}

void driver_unload(PDRIVER_OBJECT)
{
	KdPrint(("[+] Unloaded NotifyRoutineEnumerationDriver successfully.\n"));
}