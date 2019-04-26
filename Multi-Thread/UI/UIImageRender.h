#pragma once
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11Effect.h>
#include "../Transform.h"
#include "UITransform.h"
#include "UIRectBatchRender.h"
#include <memory>

class UIImageRender
{
public:
	UIImageRender();
	~UIImageRender();

	void Init(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect,const std::string& InEffectTechName,const std::string& InEffectPassName,
		const UITransform& InTransform,
		const glm::vec2& InSize, const glm::vec2& InClipSize, const glm::vec2& InAnchor
		, const glm::vec2& InCanvasSize);

	void OnRender(UIRectBatchRender* InUIRender);

	void SetSRV(LPCSTR strVarName, ID3D11ShaderResourceView* InSRV);

	void ClearSRVs(ID3D11DeviceContext* InD3dDeviceContext, LPCSTR strPassName, LPCSTR strVarNames[], int Count);

	void PostRender(const UITransform& InTransform);

private:
	void GetVertexData(UIRectBatchRender::FVectex* OutVertexData);
private:
	ID3D11Buffer* mVB;
	ID3D11InputLayout* mInputLayout;
	ID3DX11Effect* Effect;
	ID3DX11EffectTechnique* mTech;

	UITransform _Transform;
	glm::vec2 _Size;
	glm::vec2 _ClipSize;
	glm::vec2 _Anchor;
	glm::vec2 _CanvasSize;
	std::string _EffectTechName;
	std::string _EffectPassName;

	UIRectBatchRender::RectRenderElementInCPU Element;
};