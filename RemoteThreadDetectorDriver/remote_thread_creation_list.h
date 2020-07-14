#pragma once

#include <ntddk.h>

#include "config.h"
#include "new.h"
#include "list.h"
#include "auto_lock.h"

struct RemoteThreadCreation
{
	ULONG thread_id;
	ULONG process_id;
	ULONG creator_process_id;
};

struct RemoteThreadCreationEntry
{
	LIST_ENTRY list_entry;
	RemoteThreadCreation remote_thread_creation;
};

class RemoteThreadCreationList
{
public:
	void add_remote_thread_creation(ULONG thread_id, ULONG process_id, ULONG creator_process_id);
	RemoteThreadCreationEntry* remove_remote_thread_creation();

private:
	List list_;
};