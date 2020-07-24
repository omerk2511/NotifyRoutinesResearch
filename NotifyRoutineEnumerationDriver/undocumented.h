#pragma once

#include <ntddk.h>

namespace undocumented
{
	struct PSP_CALLBACK_OBJECT {
		EX_RUNDOWN_REF rundown_protection;
		void* notify_routine;
		void* context;
	};
}