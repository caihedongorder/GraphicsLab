#include "stdafx.h"
#include "CriticalSection.h"

FCriticalSection::FCriticalSection()
{
	InitializeCriticalSection(&cs);
}

FCriticalSection::~FCriticalSection()
{

}

void FCriticalSection::Enter()
{
	EnterCriticalSection(&cs);
}

void FCriticalSection::Leave()
{
	LeaveCriticalSection(&cs);
}

ScopeCriticalSection::ScopeCriticalSection(FCriticalSection& InCriticalSection)
	:CriticalSection(InCriticalSection)
{
	CriticalSection.Enter();
}

ScopeCriticalSection::~ScopeCriticalSection()
{
	CriticalSection.Leave();
}
