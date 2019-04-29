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

	_World = std::make_shared<World>();

	_GraphSystem = std::shared_ptr<GraphSystem>(new GraphSystem(_World.get()));


	if (!_GraphSystem->InitDirect3D(hWnd, 800, 600))	return false;

	_World->Init(_GraphSystem->GetD3dDevice());

	_World->CreateBox(_GraphSystem->GetD3dDevice(), {
		glm::vec3(0.0f),
		glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f)),
		glm::vec3(1.0f) });

	_GameTimer = std::make_shared<GameTimer>();
	_GameTimer->Init();
	_UISystem = std::make_shared<UISystem>();
	_UISystem->Init();

	_FPS = std::make_shared<FramePerSecond>();

	return true;
}

void GraphicsLabSystem::UnInit()
{
	JobSystem::UnInit();
}

void GraphicsLabSystem::Update()
{
	_GameTimer->Update();

	_UISystem->BeginFrame();

	_FrameUpdateJob = JobSystem::createSimpleJob<void>(nullptr, [](void* data) {}, [](void* data) {},true);

	//开始渲染上一帧数据
	_GraphSystem->BeginRender();

	float DeltaTime = _GameTimer->GetDletaTime();

	_UISystem->OnUpdate(DeltaTime);

	_FPS->Update(DeltaTime);

	//更新当前帧逻辑数据
	JobSystem::Update();

	//等待渲染完成
	_GraphSystem->WaitforFinishRender();

	_UISystem->OnPostRender();

	JobSystem::waitForJob(_FrameUpdateJob);


	_UISystem->EndFrame();


	_FrameUpdateJob = nullptr;

	Sleep(1);
}
