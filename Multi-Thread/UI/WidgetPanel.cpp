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

void WidgetPanel::OnRender(ID3D11DeviceContext* InD3dContext)
{
	for (auto It = _Controls.begin() ; It != _Controls.end() ; ++It)
	{
		(*It)->OnRender(InD3dContext);
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
