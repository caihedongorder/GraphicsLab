#pragma once


enum EWidgetType
{
	EWidgetType_Panel,
	EWidgetType_Button,
	EWidgetType_Max,
};

class IWidget
{
public:
	virtual int GetSizeX() = 0;
	virtual int GetSizeY() = 0;
	virtual int GetPosX() = 0;
	virtual int GetPosY() = 0;
	virtual EWidgetType GetWidgetType() = 0;
	virtual bool IsVisible() const = 0;
	virtual void SetVisible(bool InVisible) = 0;
};


class IUIMainFrame
{
public:
	virtual int GetSizeX() = 0;
	virtual int GetSizeY() = 0;
};

class IUISystemInterface
{
public:
	virtual IUIMainFrame* GetMainFrame() = 0;
};


