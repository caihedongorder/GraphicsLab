#include "stdafx.h"
#include "FramePerSecond.h"


FramePerSecond::FramePerSecond()
	:FrameNums(0)
	, FPS(0)
{
}


FramePerSecond::~FramePerSecond()
{
}

bool FramePerSecond::Update(float InDeltaTime)
{
	++FrameNums;

	TimeEscape += InDeltaTime;
	if (TimeEscape > 1.0f)
	{
		FPS = int(FrameNums / TimeEscape);

		TimeEscape = 0;

		FrameNums = 0;

		return true;
	}

	return false;
}
