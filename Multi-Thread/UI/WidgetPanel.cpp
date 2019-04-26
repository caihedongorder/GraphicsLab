#include "stdafx.h"
#include "WidgetPanel.h"

void WidgetPanel::AddControl(std::shared_ptr<WidgetControlBase> InControl)
{
	_Controls.push_back(InControl);
}

bool WidgetPanel::OnInit()
{
	return true;
}

void WidgetPanel::OnRender(std::shared_ptr<UIRectBatchRender> InUIRender)
{
	for (auto It = _Controls.begin() ; It != _Controls.end() ; ++It)
	{
		(*It)->OnRender(InUIRender);
	}
}

void WidgetPanel::OnUpdate(float InDeltaTime)
{
	for (auto It = _Controls.begin(); It != _Controls.end(); ++It)
	{
		(*It)->OnUpdate(InDeltaTime);
	}
}

void WidgetPanel::OnPostRender()
{
	for (auto It = _Controls.begin(); It != _Controls.end(); ++It)
	{
		(*It)->OnPostRender();
	}
}
