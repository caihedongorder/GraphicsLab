#include "stdafx.h"
#include "MainFrame.h"
#include "WidgetPanel.h"
#include "WidgetControlButton.h"
#include "../JobSystem.h"

MainFrame::MainFrame(int InSizeX, int InSizeY)
	:_SizeX(InSizeX)
	,_SizeY(InSizeY)
{

}

MainFrame::~MainFrame()
{

}

std::shared_ptr<WidgetBase> MainFrame::CreateWidget(EWidgetType InWidgetType, int InPosX, int InPosY, int InSizeX, int InSizeY, int InCanvasSizeX, int InCanvasSizeY)
{
	std::shared_ptr<WidgetBase> WidgetCreated;
	if (InWidgetType == EWidgetType_Panel)
	{
		WidgetCreated = std::make_shared<WidgetPanel>(InPosX, InPosY, InSizeX, InSizeY, InCanvasSizeX, InCanvasSizeY);
	}
	else if (InWidgetType == EWidgetType_Button)
	{
		WidgetCreated = std::make_shared<WidgetControlButton>(InPosX, InPosY, InSizeX, InSizeY, InCanvasSizeX, InCanvasSizeY);
	}

	WidgetCreated->Init();

	return WidgetCreated;
}

void MainFrame::AddWidget(std::shared_ptr<WidgetBase> InWidget)
{
	_Widgets.push_back(InWidget);
}

void MainFrame::OnRender(std::shared_ptr<UIRectBatchRender> InUIRender)
{
	UIRectBatchRender* pUIRender = InUIRender.get();
	for (auto It(_Widgets.begin()); It != _Widgets.end(); ++It)
	{
		(*It)->OnRender(pUIRender);
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
