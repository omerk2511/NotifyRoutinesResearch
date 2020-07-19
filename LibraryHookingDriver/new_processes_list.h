#pragma once

#include <ntifs.h>

#include "config.h"
#include "new.h"
#include "list.h"
#include "auto_lock.h"

struct NewProcessEntry
{
	LIST_ENTRY list_entry;
	ULONG process_id;
};

class NewProcessesList
{
public:
	void add_process(ULONG process_id);
	void remove_process(ULONG process_id);
	bool is_newly_created(ULONG process_id);

private:
	List list_;
};