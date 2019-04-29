#pragma once
#include "WidgetControlBase.h"
#include <memory>
#include "UIImageRender.h"

class WidgetControlButton : public WidgetControlBase
{
public:
	using WidgetControlBase::WidgetControlBase;
private:
	virtual EWidgetType GetWidgetType() final { return EWidgetType_Button; };

private:
	virtual bool OnInit();

protected:
	virtual void OnUpdate(float InDeltaTime);
	virtual void OnPostRender();
private:
	std::shared_ptr<UIImageRender> _uiRender;
	int EffectIdx = -1;
};

