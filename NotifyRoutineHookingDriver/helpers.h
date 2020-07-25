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

	char* get_name_of_owning_driver(void* address);
}