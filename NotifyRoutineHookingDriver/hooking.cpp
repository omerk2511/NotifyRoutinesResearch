#include "hooking.h"

NTSTATUS hooking::hook_notify_routine(char* hooked_driver_name)
{
	if (hooking::hooked_function) {
		return STATUS_ALREADY_INITIALIZED;
	}

	for (auto i = 0; i < 64; i++) {
		auto callback = reinterpret_cast<undocumented::PSP_CALLBACK_OBJECT*>(
			reinterpret_cast<ULONG64>(g_PspCreateThreadNotifyRoutine[i]) & 0xfffffffffffffff0);

		if (callback) {
			if (::ExAcquireRundownProtection(&callback->rundown_protection)) {
				char* driver_name = helpers::get_name_of_owning_driver(callback->notify_routine);

				if (driver_name) {
					if (::strlen(hooked_driver_name) == ::strlen(driver_name)) {
						auto equal_length = ::RtlCompareMemory(hooked_driver_name, driver_name, ::strlen(driver_name));
						
						if (equal_length == ::strlen(driver_name)) {
							hooking::hooked_function = reinterpret_cast<PCREATE_THREAD_NOTIFY_ROUTINE>(
								callback->notify_routine);

							hooking::context = callback->context;

							::InterlockedExchange64(
								reinterpret_cast<volatile LONG64*>(&callback->notify_routine),
								reinterpret_cast<LONG64>(&hooking::hook_function)
							);

							::ExReleaseRundownProtection(&callback->rundown_protection);

							return STATUS_SUCCESS;
						}
					}

					delete driver_name;
				}

				::ExReleaseRundownProtection(&callback->rundown_protection);
			}
		}
	}

	return STATUS_NOT_FOUND;
}

void hooking::unhook_notify_routine()
{
	if (hooking::hooked_function) {
		::PsRemoveCreateThreadNotifyRoutine(hooking::hook_function);

		switch (hooking::context) {
		case 0:
			::PsSetCreateThreadNotifyRoutine(hooking::hooked_function);
			break;

		case 1:
			::PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifyNonSystem, hooking::hooked_function);
			break;

		case 2:
			::PsSetCreateThreadNotifyRoutineEx(PsCreateThreadNotifySubsystems, hooking::hooked_function);
			break;

		default:
			break;
		}
	}
}

void hooking::hook_function(HANDLE process_id, HANDLE thread_id, BOOLEAN create)
{
	if (hooking::hooked_function && !hooking::disabled) {
		hooking::hooked_function(process_id, thread_id, create);
	}
}

void hooking::set_disabled(bool new_disabled)
{
	hooking::disabled = new_disabled;
}

PCREATE_THREAD_NOTIFY_ROUTINE hooking::hooked_function{ nullptr };
ULONG64 hooking::context{ };
volatile bool hooking::disabled{ false };