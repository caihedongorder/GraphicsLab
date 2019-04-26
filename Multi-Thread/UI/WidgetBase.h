#pragma once
#include "UISystemInterface.h"
#include <d3d11.h>
#include "UITransform.h"
#include <memory>

class WidgetBase : public IWidget
{
	friend class MainFrame;
	friend class WidgetPanel;
public:
	WidgetBase(int InPosX,int InPosY,int InSizeX,int InSizeY, int InCanvasSizeX, int InCanvasSizeY);
	~WidgetBase();

	virtual int GetSizeX() final { return _SizeX; };
	virtual int GetSizeY() final { return _SizeY; };
	virtual int GetPosX() final { return (int)_Transform.translate.x; };
	virtual int GetPosY() final { return (int)_Transform.translate.y; };

	virtual bool IsVisible() const final { return _Visible; };
	virtual void SetVisible(bool InVisible) { _Visible = InVisible; };

	bool Init();

	bool IsPenddingDestroy() const { return _IsPenddingDestroy; }
	void SetPenddingDestroy(bool bInPendingDestroy) { _IsPenddingDestroy = bInPendingDestroy; }

private:
	virtual bool OnInit() { return true; }

protected:
	virtual void OnRender(class UIRectBatchRender* InUIRender) {}
	virtual void OnUpdate(float InDeltaTime) {}
	virtual void OnPostRender() {}

	void SetCanvasSize(int InCanvasSizeX, int InCanvasSizeY) { _CanvasSizeX = InCanvasSizeX; _CanvasSizeY = InCanvasSizeY; }

protected:
	bool _IsPenddingDestroy;
	bool _Visible;
	int _SizeX;
	int _SizeY;
	int _ClipSizeX;
	int _ClipSizeY;
	int _CanvasSizeX;
	int _CanvasSizeY;
	float _AnchorX;
	float _AnchorY;

	UITransform _Transform;
};