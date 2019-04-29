#include "stdafx.h"
#include "WidgetControlButton.h"
#include "../GraphSystem.h"
#include "../ShaderManager.h"
#include "UIImageRender.h"
#include <memory>
#include "UISystem.h"

extern std::shared_ptr<GraphSystem> GGraphSystem;


bool WidgetControlButton::OnInit()
{
	_uiRender = std::make_shared<UIImageRender>();

	auto d3dDevice = GGraphSystem->GetD3dDevice();
	_uiRender->Init(d3dDevice, ShaderManager::GetInstance()->GetShader(d3dDevice, TEXT("FX/UIImage.fx")),"BaseTech","UI", _Transform,
		{ _SizeX,_SizeY }, { _ClipSizeX,_ClipSizeY }, {_AnchorX,_AnchorY}, { _CanvasSizeX,_CanvasSizeY });

	auto pEffect = ShaderManager::GetInstance()->GetShader(d3dDevice, TEXT("FX/UIImage.fx"));
	auto pTech = pEffect->GetTechniqueByName("BaseTech");
	auto pass = pTech->GetPassByName("UI");
	UISystem::GetInstance()->GetUIRectBatchRender()->RegisterEffect(pass, EffectIdx);

	return EffectIdx != -1;
}

void WidgetControlButton::OnUpdate(float InDeltaTime)
{
	_Transform.Angle += 1.0f * 0.1f;

	_uiRender->OnRender(EffectIdx);

}

void WidgetControlButton::OnPostRender()
{
	_uiRender->PostRender(_Transform);
}
