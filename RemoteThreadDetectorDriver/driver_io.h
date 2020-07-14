#pragma once

#include <ntddk.h>

#include "irp_handler.h"
#include "remote_thread_creation_list.h"

extern RemoteThreadCreationList* g_remote_thread_creation_list;

namespace driver_io
{
	NTSTATUS create_close(PDEVICE_OBJECT device_object, PIRP irp);
	NTSTATUS read(PDEVICE_OBJECT device_object, PIRP irp);
}