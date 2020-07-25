#pragma once

#include <ntddk.h>

#include "undocumented.h"
#include "helpers.h"

extern undocumented::PSP_CALLBACK_OBJECT** g_PspCreateThreadNotifyRoutine;

namespace hooking
{
	NTSTATUS hook_notify_routine(char* hooked_driver_name);
	void unhook_notify_routine();

	void hook_function(HANDLE process_id, HANDLE thread_id, BOOLEAN create);
	void set_disabled(bool new_disabled);

	extern PCREATE_THREAD_NOTIFY_ROUTINE hooked_function;
	extern ULONG64 context;
	extern volatile bool disabled;
}