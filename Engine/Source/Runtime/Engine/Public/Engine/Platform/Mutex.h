﻿#pragma once

#include <Windows.h>

/**
 * A simple mutex class that can be used to synchronize access to data shared between threads.
 */
class FMutex
{
public:
	FMutex();
	FMutex(const FMutex&) = default;
	~FMutex();

	/**
	 * Locks the mutex. If another thread has already locked the mutex, this call will block until the lock is released.
	 */
	void Lock();

	/**
	 * Tries to lock the mutex. If another thread has already locked the mutex, this call will return false immediately.
	 * @return true if the lock was acquired, false otherwise.
	 */
	bool TryLock();
	/**
	 * Unlocks the mutex so that it can be acquired by other threads.
	 */
	void Unlock();

	FMutex& operator=(const FMutex&) = default;

private:
	mutable CRITICAL_SECTION CriticalSection;

	friend class FConditionMutex;
};

/**
 * A helper class that locks a mutex in its constructor and unlocks the mutex in its destructor.
 * @note This class should not be a pointer or member variable of a class
 */
class FMutexGuard
{
public:
	FMutexGuard(const FMutex& MutexRef);
	~FMutexGuard();

private:
	FMutex Mutex;
};