#include "stdafx.h"
#include "UIImageRender.h"
#include "../GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "UISystem.h"


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

void UIImageRender::Init(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect)
{
	int FrameSizeX = UISystem::GetInstance()->GetMainFrame()->GetSizeX();
	int FrameSizeY = UISystem::GetInstance()->GetMainFrame()->GetSizeY();

	_Transform.translate = { 200,200 };
	_Transform.scale = { 1,1 };
	_Transform.Angle = glm::radians(0.0f);

	const glm::vec2 UVs[] = {
		{	glm::vec2(0.0,0.0f)	},
		{	glm::vec2(0.0,1.0f)	},
		{	glm::vec2(1.0,0.0f)	},
		{	glm::vec2(1.0,1.0f)	},
	};

	int ButtonWidth = 200;
	int ButtonHeight = 200;

	FVectex Vertex[4];

	glm::vec2 Anchor = { 0.5f,0.5f };

	int ClipWidth = 100;
	int ClipHeight = 100;

	auto ClipSize = glm::vec2(ClipWidth, ClipHeight);

	glm::vec4 ParentClipRect = { 0,0,800,600 };

	for (int iVertexIdx = 0 ; iVertexIdx < 4 ; ++iVertexIdx)
	{
		Vertex[iVertexIdx].LocationAndAnchor.x = (UVs[iVertexIdx].x - Anchor.x) * ButtonWidth;
		Vertex[iVertexIdx].LocationAndAnchor.y = (UVs[iVertexIdx].y - Anchor.y) * ButtonHeight;

		Vertex[iVertexIdx].ClipRect = { _Transform.translate - Anchor* ClipSize,_Transform.translate + ClipSize*(glm::vec2(1.0f,1.0f) - Anchor) };

		Vertex[iVertexIdx].TranslateAndScale = { _Transform.translate,_Transform.scale };
		Vertex[iVertexIdx].CanvasSizeAndWidgetSize = { 800,600,ButtonWidth,ButtonWidth };
	
		Vertex[iVertexIdx].LocationAndAnchor.z = Anchor.x;
		Vertex[iVertexIdx].LocationAndAnchor.w = Anchor.y;
		Vertex[iVertexIdx].RotateAngle = _Transform.Angle;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(FVectex) * 4;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
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
		InD3dDeviceContext->Draw(6, 0);
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
