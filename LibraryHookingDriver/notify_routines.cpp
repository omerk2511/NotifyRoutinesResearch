#include "notify_routines.h"

void notify_routines::create_process(HANDLE, HANDLE process_id, BOOLEAN create)
{
	if (create) {
		PEPROCESS process;
		auto status = ::PsLookupProcessByProcessId(process_id, &process);
		
		if (NT_SUCCESS(status)) {
			if (!::PsGetProcessWow64Process(process) && !::PsIsProtectedProcess(process)) {
				g_new_processes_list->add_process(HandleToULong(process_id));
			}
		}
	}
}

void notify_routines::create_thread(HANDLE process_id, HANDLE thread_id, BOOLEAN create)
{
	if (create &&
		g_new_processes_list->is_newly_created(HandleToULong(process_id))) {
		g_new_processes_list->remove_process(HandleToULong(process_id));

		PETHREAD thread;
		::PsLookupThreadByThreadId(thread_id, &thread);

		PKAPC apc = new (NonPagedPool, config::kDriverTag) KAPC;

		if (!apc) {
			KdPrint(("[-] Could not queue a kernel APC due to insufficient memory.\n"));

			return;
		}

		::KeInitializeApc(
			apc,
			thread,
			OriginalApcEnvironment,
			&apc_routines::kernel_free_kapc,
			&apc_routines::rundown_free_kapc,
			&apc_routines::normal_inject_code,
			KernelMode,
			nullptr
		);

		auto inserted = ::KeInsertQueueApc(
			apc,
			nullptr,
			nullptr,
			0
		);

		if (!inserted) {
			delete apc;

			KdPrint(("[-] Could not insert a kernel APC.\n"));

			return;
		}

		::InterlockedIncrement64(&g_apc_count);
	}
}