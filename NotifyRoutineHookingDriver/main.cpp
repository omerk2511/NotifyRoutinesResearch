#include <ntddk.h>
#include <aux_klib.h>

#pragma comment(lib, "aux_klib.lib")

#include "config.h"
#include "undocumented.h"
#include "helpers.h"
#include "hooking.h"

void driver_unload(PDRIVER_OBJECT driver_object);

undocumented::PSP_CALLBACK_OBJECT** g_PspCreateThreadNotifyRoutine;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING)
{
	driver_object->DriverUnload = driver_unload;

	auto status = ::AuxKlibInitialize();

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed loading NotifyRoutineHookingDriver: could not initialize AuxKlib.\n"));
		return status;
	}

	g_PspCreateThreadNotifyRoutine = helpers::find_PspCreateThreadNotifyRoutine();

	if (!g_PspCreateThreadNotifyRoutine) {
		KdPrint(("[-] Failed loading NotifyRoutineHookingDriver: could not find PspCreateThreadNotifyRoutine.\n"));
		return STATUS_NOT_FOUND;
	}

	status = hooking::hook_notify_routine("LibraryHookingDriver.sys");

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed loading NotifyRoutineHookingDriver: could not hook LibraryHookingDriver.sys.\n"));
	}

	hooking::set_disabled(true);

	KdPrint(("[+] Loaded NotifyRoutineHookingDriver successfully.\n"));

	return STATUS_SUCCESS;
}

void driver_unload(PDRIVER_OBJECT)
{
	hooking::unhook_notify_routine();
	KdPrint(("[+] Unloaded NotifyRoutineHookingDriver successfully.\n"));
}