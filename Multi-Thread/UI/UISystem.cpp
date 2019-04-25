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
	_MainFrame = std::shared_ptr<MainFrame>(new MainFrame(800,600));
	auto Widget = _MainFrame->CreateWidget(EWidgetType_Panel, 0, 0, 800, 600);
	auto Button = _MainFrame->CreateWidget(EWidgetType_Button, 0, 0, 100, 100);
	std::static_pointer_cast<WidgetPanel>(Widget)->AddControl(std::static_pointer_cast<WidgetControlBase>(Button));
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

void UISystem::OnRender(ID3D11DeviceContext* InD3dContext)
{
	_MainFrame->OnRender(InD3dContext);
}

void UISystem::OnUpdate(float InDeltaTime)
{
	_MainFrame->OnUpdate(InDeltaTime);
}

void UISystem::OnPostRender()
{
	_MainFrame->OnPostRender();
}