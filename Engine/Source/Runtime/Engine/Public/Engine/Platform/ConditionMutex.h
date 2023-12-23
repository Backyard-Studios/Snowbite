#pragma once

#include <Windows.h>

#include <Engine/Platform/Mutex.h>

class FConditionMutex
{
public:
	FConditionMutex();
	FConditionMutex(const FConditionMutex&) = default;
	~FConditionMutex();

	/**
	 * Wait for the condition variable to be notified.
	 * @param Mutex The mutex to wait on.
	 * @param WaitTime The time to wait for the condition variable to be notified.
	 * @return true if the condition variable was notified, false if the wait timed out.
	 */
	bool Wait(const FMutex& Mutex, DWORD WaitTime = INFINITE);

	/**
	 * Notifies one waiting thread.
	 */
	void NotifyOne();
	/**
	 * Notifies all waiting threads.
	 */
	void NotifyAll();

	FConditionMutex& operator=(const FConditionMutex&) = default;

private:
	CONDITION_VARIABLE ConditionVariable;
};
