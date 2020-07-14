#pragma once

#include <ntddk.h>

#include "fast_mutex.h"

class List
{
public:
	List();
	~List();

	void insert(LIST_ENTRY* entry);
	void remove(LIST_ENTRY* entry);

	LIST_ENTRY* remove_tail();

	LIST_ENTRY* get_head();
	FastMutex& get_mutex();

private:
	LIST_ENTRY head_;
	FastMutex mutex_;
	int count_;
};