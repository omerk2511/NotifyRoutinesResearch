#pragma once

#include <ntddk.h>

#include "remote_thread_creation_list.h"
#include "new_processes_cache.h"

extern RemoteThreadCreationList* g_remote_thread_creation_list;
extern NewProcessesCache* g_new_processes_cache;

namespace notify_routines
{
	void create_process(PEPROCESS process, HANDLE process_id, PPS_CREATE_NOTIFY_INFO create_info);
	void create_thread(HANDLE process_id, HANDLE thread_id, BOOLEAN create);
}