#include "stdafx.h"
#include "UIImageRender.h"
#include "../GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "UISystem.h"
#include "../MathUtil.h"
#include "../GraphicsLabSystem.h"

extern std::shared_ptr<GraphSystem> GGraphSystem;


UIImageRender::UIImageRender()
	: mVB(NULL)
	, Effect(NULL)
	, mTech(NULL)
	, mInputLayout(NULL)
{

}


UIImageRender::~UIImageRender()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mInputLayout);
}

void UIImageRender::Init(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect, const std::string& InEffectTechName, const std::string& InEffectPassName, 
	const UITransform& InTransform,
	const glm::vec2& InSize, const glm::vec2& InClipSize, const glm::vec2& InAnchor, const glm::vec2& InCanvasSize)
{
	Effect = InEffect;
	_EffectTechName = InEffectTechName;
	_EffectPassName = InEffectPassName;
	_Transform = InTransform;
	_Size = InSize;
	_ClipSize = InClipSize;
	_Anchor = InAnchor;
	_CanvasSize = InCanvasSize;
}

void UIImageRender::OnRender(int InEffectIdx)
{
	GetVertexData(Element.VertexData);
	GraphicsLabSystem::GetInstance()->GetUISystem()->GetUIRectBatchRender()->DrawInCPU(InEffectIdx,Element);
}

void UIImageRender::SetSRV(LPCSTR strVarName, ID3D11ShaderResourceView* InSRV)
{
	if (auto pVariable = Effect->GetVariableByName(strVarName))
	{
		if (auto pShaderResource = pVariable->AsShaderResource())
		{
			pShaderResource->SetResource(InSRV);
		}
	}
}

void UIImageRender::ClearSRVs(ID3D11DeviceContext* InD3dDeviceContext, LPCSTR strPassName, LPCSTR strVarNames[], int Count)
{
	if (auto pBasePass = mTech->GetPassByName(strPassName))
	{

		for (int iVar = 0; iVar < Count; ++iVar)
		{
			if (auto pVariable = Effect->GetVariableByName(strVarNames[iVar]))
			{
				if (auto pShaderResource = pVariable->AsShaderResource())
				{
					pShaderResource->SetResource(nullptr);
				}
			}
		}

		pBasePass->Apply(0, InD3dDeviceContext);
	}
}

void UIImageRender::PostRender(const UITransform& InTransform)
{
	_Transform = InTransform;
}

void UIImageRender::GetVertexData(UIRectBatchRender::FVectex* OutVertexData)
{
	const glm::vec2 UVs[] = {
	{	glm::vec2(0.0,0.0f)	},
	{	glm::vec2(0.0,1.0f)	},
	{	glm::vec2(1.0,0.0f)	},
	{	glm::vec2(1.0,0.0f)	},
	{	glm::vec2(1.0,1.0f)	},
	{	glm::vec2(0.0,1.0f)	},
	};

	glm::vec4 ParentClipRect = { 0,0,800,600 };
	glm::vec4 ClipRect = { _Transform.translate - _Anchor * _ClipSize,_Transform.translate + _ClipSize * (glm::vec2(1.0f,1.0f) - _Anchor) };

	glm::vec4 IntersectedRect;
	MathUtil::TwoRectIntersect(ParentClipRect, ClipRect, IntersectedRect);

	for (int iVertexIdx = 0; iVertexIdx < 6; ++iVertexIdx)
	{
		OutVertexData[iVertexIdx].LocationAndAnchor.x = (UVs[iVertexIdx].x - _Anchor.x) * _Size.x;
		OutVertexData[iVertexIdx].LocationAndAnchor.y = (UVs[iVertexIdx].y - _Anchor.y) * _Size.y;

		OutVertexData[iVertexIdx].ClipRect = IntersectedRect;

		OutVertexData[iVertexIdx].TranslateAndScale = { _Transform.translate,_Transform.scale };
		OutVertexData[iVertexIdx].CanvasSizeAndWidgetSize = { _CanvasSize.x,_CanvasSize.y,_Size.x,_Size.y };

		OutVertexData[iVertexIdx].LocationAndAnchor.z = _Anchor.x;
		OutVertexData[iVertexIdx].LocationAndAnchor.w = _Anchor.y;
		OutVertexData[iVertexIdx].RotateAngle = glm::radians(_Transform.Angle);
	}
}
