#include "pch.h"

#include <Engine/Platform/Mutex.h>

FMutex::FMutex()
{
	InitializeCriticalSectionEx(&CriticalSection, 400, CRITICAL_SECTION_NO_DEBUG_INFO);
}

FMutex::~FMutex()
{
	DeleteCriticalSection(&CriticalSection);
}

void FMutex::Lock()
{
	EnterCriticalSection(&CriticalSection);
}

bool FMutex::TryLock()
{
	return TryEnterCriticalSection(&CriticalSection);
}

void FMutex::Unlock()
{
	LeaveCriticalSection(&CriticalSection);
}

FMutexGuard::FMutexGuard(const FMutex& MutexRef)
	: Mutex(MutexRef)
{
	Mutex.Lock();
}

FMutexGuard::~FMutexGuard()
{
	Mutex.Unlock();
}
