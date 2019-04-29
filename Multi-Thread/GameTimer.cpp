#include "stdafx.h"
#include "GameTimer.h"


GameTimer GameTimer::sInst;

GameTimer::GameTimer()
{
}


GameTimer::~GameTimer()
{
}

void GameTimer::Init()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&CurrentTime);
}

void GameTimer::Update()
{
	LARGE_INTEGER nowTime;
	QueryPerformanceCounter(&nowTime);

	DeltaTime = float(double(nowTime.QuadPart - CurrentTime.QuadPart) / frequency.QuadPart);

	CurrentTime = nowTime;
}

GameTimer* GameTimer::GetInstance()
{
	return &sInst;
}
