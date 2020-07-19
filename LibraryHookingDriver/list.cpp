#include "list.h"

List::List()
	: count_()
{
	::InitializeListHead(&head_);
	mutex_.Init();
}

List::~List()
{
	while (!::IsListEmpty(&head_)) {
		auto entry = ::RemoveTailList(&head_);
		delete entry;
	}
}

void List::insert(LIST_ENTRY* entry)
{
	if (count_ >= 1024) { // move to constant / registry key
		auto head = ::RemoveHeadList(&head_);
		delete head;

		count_--;
	}

	::InsertTailList(&head_, entry);
	count_++;
}

void List::remove(LIST_ENTRY* entry)
{
	::RemoveEntryList(entry);
	delete entry;

	count_--;
}

LIST_ENTRY* List::remove_tail()
{
	if (count_ > 0) {
		count_--;
		return ::RemoveTailList(&head_);
	}

	return nullptr;
}

LIST_ENTRY* List::get_head()
{
	return &head_;
}

FastMutex& List::get_mutex()
{
	return mutex_;
}