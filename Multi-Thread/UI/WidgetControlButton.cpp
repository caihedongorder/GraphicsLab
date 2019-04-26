#include "stdafx.h"
#include "WidgetControlButton.h"
#include "../GraphSystem.h"
#include "../ShaderManager.h"
#include "UIImageRender.h"
#include <memory>

extern std::shared_ptr<GraphSystem> GGraphSystem;


bool WidgetControlButton::OnInit()
{
	_uiRender = std::make_shared<UIImageRender>();

	auto d3dDevice = GGraphSystem->GetD3dDevice();
	_uiRender->Init(d3dDevice, ShaderManager::GetInstance()->GetShader(d3dDevice, TEXT("FX/UIImage.fx")),"BaseTech","UI", _Transform,
		{ _SizeX,_SizeY }, { _ClipSizeX,_ClipSizeY }, {_AnchorX,_AnchorY}, { _CanvasSizeX,_CanvasSizeY });

	return true;
}

void WidgetControlButton::OnRender(class UIRectBatchRender* InUIRender)
{
	_uiRender->OnRender(InUIRender);
}

void WidgetControlButton::OnUpdate(float InDeltaTime)
{
	_Transform.Angle += 1.0f * 0.1f;
}

void WidgetControlButton::OnPostRender()
{
	_uiRender->PostRender(_Transform);
}
