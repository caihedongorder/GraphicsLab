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

	void OnRender(int InEffectIdx);

	void PostRender(const UITransform& InTransform);

private:
	void GetVertexData(UIRectBatchRender::FVectex* OutVertexData);
private:

	UITransform _Transform;
	glm::vec2 _Size;
	glm::vec2 _ClipSize;
	glm::vec2 _Anchor;
	glm::vec2 _CanvasSize;
	std::string _EffectTechName;
	std::string _EffectPassName;

	UIRectBatchRender::RectRenderElementInCPU Element;
};