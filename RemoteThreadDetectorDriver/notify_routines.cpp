#include "notify_routines.h"

void notify_routines::create_process(HANDLE, HANDLE process_id, BOOLEAN create)
{
	if (create) {
		g_new_processes_cache->add_process(HandleToULong(process_id));
	}
}

void notify_routines::create_thread(HANDLE process_id, HANDLE thread_id, BOOLEAN create)
{
	if (create) {
		HANDLE creator_process_id = ::PsGetCurrentProcessId();

		if (process_id != creator_process_id &&
			HandleToULong(creator_process_id) != 4 &&
			!g_new_processes_cache->is_newly_created(HandleToULong(process_id))) {
			g_remote_thread_creation_list->add_remote_thread_creation(
				HandleToULong(thread_id),
				HandleToULong(process_id),
				HandleToULong(creator_process_id)
			);

			KdPrint(("[*] Thread %d in process %d was created remotely from process %d.\n",
				thread_id, process_id, creator_process_id));
		} else if (g_new_processes_cache->is_newly_created(HandleToULong(process_id))) {
			g_new_processes_cache->remove_process(HandleToULong(process_id));
		}
	}
}