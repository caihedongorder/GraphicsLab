#pragma once
#include <chrono>
class GameTimer
{
public:
	GameTimer();
	~GameTimer();

	void Init();
	void Update();

	float GetDletaTime() const { return DeltaTime; }

private:
	float DeltaTime;

	LARGE_INTEGER frequency;	//计时器频率
	LARGE_INTEGER CurrentTime;	//计时器频率

	static GameTimer sInst;
};

