#include "stdafx.h"
#include "GraphicsLabSystem.h"
#include "UI/UISystem.h"
#include "JobSystem.h"
#include "GameTimer.h"
#include "World.h"
#include <memory>
#include <iosfwd>

GraphicsLabSystem GraphicsLabSystem::sInst;


GraphicsLabSystem::GraphicsLabSystem()
{
}


GraphicsLabSystem::~GraphicsLabSystem()
{
}

GraphicsLabSystem* GraphicsLabSystem::GetInstance()
{
	return &sInst;
}

bool GraphicsLabSystem::Init(HWND hWnd)
{

	JobSystem::Init(4096);

	GWorld = std::make_shared<World>();

	GGraphSystem = std::shared_ptr<GraphSystem>(new GraphSystem(GWorld.get()));


	if (!GGraphSystem->InitDirect3D(hWnd, 800, 600))	return false;

	GWorld->Init(GGraphSystem->GetD3dDevice());

	GWorld->CreateBox(GGraphSystem->GetD3dDevice(), {
		glm::vec3(0.0f),
		glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f)),
		glm::vec3(1.0f) });


	GameTimer::GetInstance()->Init();
	UISystem::GetInstance()->Init();

	GFPS = std::make_shared<FramePerSecond>();

	return true;
}

void GraphicsLabSystem::UnInit()
{
	JobSystem::UnInit();
}

void GraphicsLabSystem::Update()
{
	GameTimer::GetInstance()->Update();

	UISystem::GetInstance()->BeginFrame();

	//开始渲染上一帧数据
	GGraphSystem->BeginRender();

	float DeltaTime = GameTimer::GetInstance()->GetDletaTime();

	UISystem::GetInstance()->OnUpdate(DeltaTime);

	//更新当前帧逻辑数据
	JobSystem::Update();

	//等待渲染完成
	GGraphSystem->WaitforFinishRender();

	UISystem::GetInstance()->OnPostRender();

	UISystem::GetInstance()->EndFrame();

	Sleep(1);
}
