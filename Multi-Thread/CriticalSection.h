#pragma once

class FCriticalSection
{
public:
	FCriticalSection();
	~FCriticalSection();

	void Enter();
	void Leave();
private:
	CRITICAL_SECTION cs;
};


class ScopeCriticalSection
{
public:
	ScopeCriticalSection(FCriticalSection& InCriticalSection);
	~ScopeCriticalSection();
private:
	FCriticalSection& CriticalSection;
};