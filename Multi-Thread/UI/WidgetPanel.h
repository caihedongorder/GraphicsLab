#pragma once
#include "WidgetBase.h"
#include "WidgetControlBase.h"
#include <vector>
#include <memory>

class WidgetPanel : public WidgetBase
{
public:
	using WidgetBase::WidgetBase;

	void AddControl(std::shared_ptr<WidgetControlBase> InControl);
private:

	virtual EWidgetType GetWidgetType() final { return EWidgetType_Panel; };

private:
	virtual bool OnInit();

protected:
	virtual void OnUpdate(float InDeltaTime);
	virtual void OnPostRender();

private:
	std::vector<std::shared_ptr<WidgetControlBase>> _Controls;
};