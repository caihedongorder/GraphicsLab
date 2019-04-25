#pragma once
#include "UISystemInterface.h"
#include <d3d11.h>

class WidgetBase : public IWidget
{
	friend class MainFrame;
	friend class WidgetPanel;
public:
	WidgetBase(int InPosX,int InPosY,int InSizeX,int InSizeY);
	~WidgetBase();

	virtual int GetSizeX() final { return _SizeX; };
	virtual int GetSizeY() final { return _SizeY; };
	virtual int GetPosX() final { return _PosX; };
	virtual int GetPosY() final { return _PosY; };

	virtual bool IsVisible() const final { return _Visible; };
	virtual void SetVisible(bool InVisible) { _Visible = InVisible; };

	bool Init();

	bool IsPenddingDestroy() const { return _IsPenddingDestroy; }
	void SetPenddingDestroy(bool bInPendingDestroy) { _IsPenddingDestroy = bInPendingDestroy; }

private:
	virtual bool OnInit() { return true; }

protected:
	virtual void OnRender(ID3D11DeviceContext* InD3dContext) {}
	virtual void OnUpdate(float InDeltaTime) {}
	virtual void OnPostRender() {}

private:
	bool _IsPenddingDestroy;
	bool _Visible;
	int _PosX;
	int _PosY;
	int _SizeX;
	int _SizeY;
};