#pragma once
#include <memory>

#include "FramePerSecond.h"
#include "GraphSystem.h"
#include "World.h"

class GraphicsLabSystem
{
public:
	GraphicsLabSystem();
	~GraphicsLabSystem();

	static GraphicsLabSystem* GetInstance();

	bool Init(HWND hWnd);

	void UnInit();

	int GetFPS() const { return GFPS->GetFPS(); }

	void Update();

	GraphSystem* GetGraphSystem() { return GGraphSystem.get(); }

private:
	static GraphicsLabSystem sInst;

	std::shared_ptr<FramePerSecond> GFPS;
	std::shared_ptr<GraphSystem> GGraphSystem;
	std::shared_ptr<World> GWorld;

};

