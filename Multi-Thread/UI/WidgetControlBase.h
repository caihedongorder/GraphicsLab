#pragma once
#include "WidgetBase.h"
#include <string>

class WidgetControlBase : public WidgetBase
{
	friend class MainFrame;
public:
	using WidgetBase::WidgetBase;

	int GetZOrder() const { return _ZOrder; }
	int GetControlId() const { return _ControlId; }
	std::string GetControlName() const { return _ControlName; }

private:
	void SetZOrder(int InZOrder) { _ZOrder = InZOrder; }
	void SetControlId(int InControlId) { _ControlId = InControlId; }
	void SetControlId(std::string InControlName) { _ControlName = InControlName; }

protected:
	int _ZOrder;
	int _ControlId;
	std::string _ControlName;
};