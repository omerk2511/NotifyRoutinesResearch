#pragma once

template<typename T>
class AutoLock
{
public:
	AutoLock(T& lock)
		: lock_(lock)
	{
		lock_.Lock();
	}

	~AutoLock()
	{
		lock_.Unlock();
	}

private:
	T& lock_;
};