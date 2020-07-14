#include "remote_thread_creation_list.h"

void RemoteThreadCreationList::add_remote_thread_creation(ULONG thread_id, ULONG process_id, ULONG creator_process_id)
{
	AutoLock lock(list_.get_mutex());

	auto entry = new (PagedPool, config::kDriverTag) RemoteThreadCreationEntry();

	if (!entry) {
		KdPrint(("[-] Failed to log a remote thread creation detected due to insufficient memory.\n"));
		return;
	}

	entry->remote_thread_creation.thread_id = thread_id;
	entry->remote_thread_creation.process_id = process_id;
	entry->remote_thread_creation.creator_process_id = creator_process_id;

	list_.insert(&entry->list_entry);
}

RemoteThreadCreationEntry* RemoteThreadCreationList::remove_remote_thread_creation()
{
	AutoLock lock(list_.get_mutex());

	auto entry = list_.remove_tail();

	if (entry != list_.get_head()) {
		return CONTAINING_RECORD(entry, RemoteThreadCreationEntry, list_entry);
	}
	
	return nullptr;
}