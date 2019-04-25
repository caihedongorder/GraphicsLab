#include "stdafx.h"
#include "WidgetBase.h"

WidgetBase::WidgetBase(int InPosX, int InPosY, int InSizeX, int InSizeY)
	:_PosX(InPosX)
	,_PosY(InPosY)
	,_SizeX(InSizeX)
	,_SizeY(InSizeY)
	, _Visible(true)
	, _IsPenddingDestroy(false)
{

}

WidgetBase::~WidgetBase()
{

}

bool WidgetBase::Init()
{

	return OnInit();
}
