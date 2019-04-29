#include "stdafx.h"
#include "UISystem.h"
#include "WidgetPanel.h"


UISystem UISystem::_sInst;


UISystem::UISystem()
{

}

UISystem::~UISystem()
{

}

bool UISystem::Init()
{
	_UIRectBatchRender = std::make_shared<UIRectBatchRender>();

	_MainFrame = std::shared_ptr<MainFrame>(new MainFrame(800,600));
	auto Widget = _MainFrame->CreateWidget(EWidgetType_Panel, 0, 0, 800, 600, 800, 600);

	int StartPosX = 20;
	int StartPosY = 20;

	int Step = 10;
	int CurrentPosX = StartPosX;
	int CurrentPosY = StartPosY;
	while (CurrentPosY < 600)
	{
		CurrentPosX = StartPosX;
		while (CurrentPosX < 800)
		{
			auto Button = _MainFrame->CreateWidget(EWidgetType_Button, CurrentPosX, CurrentPosY, 20, 20, 800, 600);
			std::static_pointer_cast<WidgetPanel>(Widget)->AddControl(std::static_pointer_cast<WidgetControlBase>(Button));
			CurrentPosX += Step;
		}
		CurrentPosY += Step;
	}
	
	_MainFrame->AddWidget(Widget);


	return true;
}

IUIMainFrame* UISystem::GetMainFrame()
{
	return _MainFrame.get();
}

UISystem* UISystem::GetInstance()
{
	return &_sInst;
}

void UISystem::OnRender()
{
	_UIRectBatchRender->OnRender();
}

void UISystem::OnUpdate(float InDeltaTime)
{
 	_MainFrame->OnUpdate(InDeltaTime);
}

void UISystem::OnPostRender()
{
 	_MainFrame->OnPostRender();
 	_UIRectBatchRender->PostRender();
}

void UISystem::BeginFrame()
{
	_UIRectBatchRender->BeginFrame();
}

void UISystem::EndFrame()
{
	_UIRectBatchRender->EndFrame();
}
