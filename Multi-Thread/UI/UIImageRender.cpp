#include "stdafx.h"
#include "UIImageRender.h"
#include "../GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "UISystem.h"
#include "../MathUtil.h"

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

void UIImageRender::Init(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect, const UITransform& InTransform, 
	const glm::vec2& InSize, const glm::vec2& InClipSize, const glm::vec2& InAnchor, const glm::vec2& InCanvasSize)
{
	int FrameSizeX = UISystem::GetInstance()->GetMainFrame()->GetSizeX();
	int FrameSizeY = UISystem::GetInstance()->GetMainFrame()->GetSizeY();

	_Transform = InTransform;
	_Size = InSize;
	_ClipSize = InClipSize;
	_Anchor = InAnchor;
	_CanvasSize = InCanvasSize;


	FVectex Vertex[4];
	GetVertexData(Vertex);
	
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(FVectex) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = Vertex;
	HR(Ind3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));


	Effect = InEffect;

	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"CLIP", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"CW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"LA", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"ROTATE", 0, DXGI_FORMAT_R32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	mTech = Effect->GetTechniqueByName("BaseTech");

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByName("UI")->GetDesc(&passDesc);
	HR(Ind3dDevice->CreateInputLayout(vertexDesc, sizeof(vertexDesc)/sizeof(vertexDesc[0]), passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}

void UIImageRender::OnRender(ID3D11DeviceContext* InD3dDeviceContext,LPCSTR strPassName)
{
	InD3dDeviceContext->IASetInputLayout(mInputLayout);
	InD3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(FVectex);
	UINT offset = 0;
	InD3dDeviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);


	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);

	if (auto pBasePass = mTech->GetPassByName(strPassName))
	{
		pBasePass->Apply(0, InD3dDeviceContext);

		// 36 indices for the box.
		InD3dDeviceContext->Draw(4, 0);
	}

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

	auto D3dDeviceContext = GGraphSystem->GetD3dDeviceContext();
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto result = D3dDeviceContext->Map(mVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		FVectex Vertex[4];
		GetVertexData(Vertex);

		// Get a pointer to the data in the constant buffer.
		auto dataPtr = (PerFrameData *)mappedResource.pData;

		memcpy(mappedResource.pData, Vertex, sizeof(Vertex));

		D3dDeviceContext->Unmap(mVB, 0);
	}
}

void UIImageRender::GetVertexData(FVectex* OutVertexData)
{
	const glm::vec2 UVs[] = {
	{	glm::vec2(0.0,0.0f)	},
	{	glm::vec2(0.0,1.0f)	},
	{	glm::vec2(1.0,0.0f)	},
	{	glm::vec2(1.0,1.0f)	},
	};

	glm::vec4 ParentClipRect = { 0,0,800,600 };
	glm::vec4 ClipRect = { _Transform.translate - _Anchor * _ClipSize,_Transform.translate + _ClipSize * (glm::vec2(1.0f,1.0f) - _Anchor) };

	glm::vec4 IntersectedRect;
	MathUtil::TwoRectIntersect(ParentClipRect, ClipRect, IntersectedRect);

	for (int iVertexIdx = 0; iVertexIdx < 4; ++iVertexIdx)
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
