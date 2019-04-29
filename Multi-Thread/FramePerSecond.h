#pragma once
class FramePerSecond
{
public:
	FramePerSecond();
	~FramePerSecond();

	int GetFPS() const { return FPS; }
	bool Update(float InDeltaTime);
private:
	float TimeEscape = 0;
	int FPS;
	int FrameNums = 0;
};

