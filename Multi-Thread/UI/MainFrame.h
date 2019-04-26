#pragma once
#include "WidgetBase.h"
#include <vector>
#include <memory>

#include <d3d11.h>

class MainFrame : public IUIMainFrame
{
	friend class UISystem;
public:
	MainFrame(int InSizeX,int InSizeY);
	virtual ~MainFrame();

	virtual int GetSizeX()  final { return _SizeX; };
	virtual int GetSizeY() final { return _SizeY; };

	static std::shared_ptr<WidgetBase> CreateWidget(EWidgetType InWidgetType,int InPosX,int InPosY,int InSizeX,int InSizeY,int InCanvasSizeX,int InCanvasSizeY);
	void AddWidget(std::shared_ptr<WidgetBase> InWidget);

private:
	void OnRender(std::shared_ptr<UIRectBatchRender> InUIRender);
	void OnUpdate(float InDeltaTime);
	void OnPostRender();
private:
	int _SizeX;
	int _SizeY;

	std::vector<std::shared_ptr<WidgetBase>> _Widgets;
};