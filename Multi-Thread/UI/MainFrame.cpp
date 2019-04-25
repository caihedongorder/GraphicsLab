#include "stdafx.h"
#include "MainFrame.h"
#include "WidgetPanel.h"
#include "WidgetControlButton.h"

MainFrame::MainFrame(int InSizeX, int InSizeY)
	:_SizeX(InSizeX)
	,_SizeY(InSizeY)
{

}

MainFrame::~MainFrame()
{

}

std::shared_ptr<WidgetBase> MainFrame::CreateWidget(EWidgetType InWidgetType, int InPosX, int InPosY, int InSizeX, int InSizeY)
{
	std::shared_ptr<WidgetBase> WidgetCreated;
	if (InWidgetType == EWidgetType_Panel)
	{
		WidgetCreated = std::make_shared<WidgetPanel>(InPosX, InPosY, InSizeX, InSizeY);
	}
	else if (InWidgetType == EWidgetType_Button)
	{
		WidgetCreated = std::make_shared<WidgetControlButton>(InPosX, InPosY, InSizeX, InSizeY);
	}

	WidgetCreated->Init();

	return WidgetCreated;
}

void MainFrame::AddWidget(std::shared_ptr<WidgetBase> InWidget)
{
	_Widgets.push_back(InWidget);
}

void MainFrame::OnRender(ID3D11DeviceContext* InD3dContext)
{
	for (auto It(_Widgets.begin()); It != _Widgets.end(); ++It)
	{
		(*It)->OnRender(InD3dContext);
	}
}

void MainFrame::OnUpdate(float InDeltaTime)
{
	for (auto It(_Widgets.begin());It != _Widgets.end();++It)
	{
		(*It)->OnUpdate(InDeltaTime);
	}
}

void MainFrame::OnPostRender()
{
	for (auto It(_Widgets.begin()); It != _Widgets.end(); ++It)
	{
		(*It)->OnPostRender();
	}
}
