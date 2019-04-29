#pragma once
#include <memory>

#include "FramePerSecond.h"
#include "GraphSystem.h"
#include "World.h"
#include "GameTimer.h"
#include "UI/UISystem.h"

class GraphicsLabSystem
{
public:
	GraphicsLabSystem();
	~GraphicsLabSystem();

	static GraphicsLabSystem* GetInstance();

	bool Init(HWND hWnd);

	void UnInit();

	int GetFPS() const { return _FPS->GetFPS(); }

	void Update();

	GraphSystem* GetGraphSystem() { return _GraphSystem.get(); }
	UISystem* GetUISystem() { return _UISystem.get(); }

private:
	static GraphicsLabSystem sInst;

	std::shared_ptr<FramePerSecond> _FPS;
	std::shared_ptr<GraphSystem> _GraphSystem;
	std::shared_ptr<World> _World;
	std::shared_ptr<GameTimer> _GameTimer;
	std::shared_ptr<UISystem> _UISystem;
};

