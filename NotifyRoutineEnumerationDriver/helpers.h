#pragma once

#include <ntddk.h>
#include <aux_klib.h>

#include "config.h"
#include "new.h"
#include "delete.h"
#include "undocumented.h"

namespace helpers
{
	undocumented::PSP_CALLBACK_OBJECT** find_PspCreateThreadNotifyRoutine();
	void enumerate_create_thread_notify_routines(undocumented::PSP_CALLBACK_OBJECT** PspCreateThreadNotifyRoutine);

	char* get_name_of_owning_driver(void* address);
}