#include "stdafx.h"
#include "WidgetBase.h"

WidgetBase::WidgetBase(int InPosX, int InPosY, int InSizeX, int InSizeY, int InCanvasSizeX, int InCanvasSizeY)
	:_SizeX(InSizeX)
	,_SizeY(InSizeY)
	, _ClipSizeX(InSizeX)
	, _ClipSizeY(InSizeY)
	, _Visible(true)
	, _IsPenddingDestroy(false)
	, _AnchorX(0.5f)
	, _AnchorY(0.5f)
	, _CanvasSizeX(InCanvasSizeX)
	, _CanvasSizeY(InCanvasSizeY)
{
	_Transform.Angle = 0.0f;
	_Transform.scale = { 1.0f,1.0f };
	_Transform.translate = { InPosX,InPosY };
}

WidgetBase::~WidgetBase()
{

}

bool WidgetBase::Init(class UIRectBatchRender* InUIRender)
{

	return OnInit(InUIRender);
}
