#pragma once

#include <ntifs.h>

#include "new.h"
#include "delete.h"
#include "undocumented_imports.h"
#include "new_processes_list.h"
#include "apc_routines.h"

extern NewProcessesList* g_new_processes_list;
extern volatile LONG64 g_apc_count;

namespace notify_routines
{
	void create_process(HANDLE parent_id, HANDLE process_id, BOOLEAN create);
	void create_thread(HANDLE process_id, HANDLE thread_id, BOOLEAN create);
}