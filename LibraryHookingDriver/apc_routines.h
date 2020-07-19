#pragma once

#include <ntifs.h>

#include "config.h"
#include "new.h"
#include "delete.h"
#include "undocumented_imports.h"
#include "injected_shellcode.h"

extern volatile LONG64 g_apc_count;

namespace apc_routines
{
	void kernel_free_kapc(PKAPC apc, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*);
	void rundown_free_kapc(PKAPC apc);
	void normal_inject_code(PVOID, PVOID, PVOID);
}