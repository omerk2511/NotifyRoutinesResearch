#include "fast_mutex.h"

void FastMutex::Init()
{
	::ExInitializeFastMutex(&fast_mutex_);
}

void FastMutex::Lock()
{
	::ExAcquireFastMutex(&fast_mutex_);
}

void FastMutex::Unlock()
{
	::ExReleaseFastMutex(&fast_mutex_);
}