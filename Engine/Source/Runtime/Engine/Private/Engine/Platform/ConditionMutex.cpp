#include "pch.h"

#include <Engine/Platform/ConditionMutex.h>

FConditionMutex::FConditionMutex()
{
	InitializeConditionVariable(&ConditionVariable);
}

FConditionMutex::~FConditionMutex() = default;

bool FConditionMutex::Wait(const FMutex& Mutex, const DWORD WaitTime)
{
	return SleepConditionVariableCS(&ConditionVariable, &Mutex.CriticalSection, WaitTime);
}

void FConditionMutex::NotifyOne()
{
	WakeConditionVariable(&ConditionVariable);
}

void FConditionMutex::NotifyAll()
{
	WakeAllConditionVariable(&ConditionVariable);
}
