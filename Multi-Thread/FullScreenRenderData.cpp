#include "stdafx.h"
#include "FullScreenRenderData.h"
#include "GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>

FullScreenRenderData::FullScreenRenderData()
	: mVB(NULL)
	, Effect(NULL)
	, mTech(NULL)
	, mInputLayout(NULL)
{

}


FullScreenRenderData::~FullScreenRenderData()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mInputLayout);
}

void FullScreenRenderData::Init(ID3D11Device* Ind3dDevice,ID3DX11Effect* InEffect)
{

	const FVectex Vertex[] = {
		{	glm::vec2(0.0,0.0f)	},
		{	glm::vec2(1.0,1.0f)	},
		{	glm::vec2(0.0,1.0f)	},
		{	glm::vec2(0.0,0.0f)	},
		{	glm::vec2(1.0,0.0f)	},
		{	glm::vec2(1.0,1.0f)	},
	};
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(FVectex) * 6;
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
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	mTech = Effect->GetTechniqueByName("BaseTech");

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByName("Composite")->GetDesc(&passDesc);
	HR(Ind3dDevice->CreateInputLayout(vertexDesc, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}

void FullScreenRenderData::OnRender(ID3D11DeviceContext* InD3dDeviceContext, ID3D11Buffer* InPerFrameConstBuff, LPCSTR strPassName)
{
	InD3dDeviceContext->IASetInputLayout(mInputLayout);
	InD3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(FVectex);
	UINT offset = 0;
	InD3dDeviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	

	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);

	if (auto pBasePass = mTech->GetPassByName(strPassName))
	{
		pBasePass->Apply(0, InD3dDeviceContext);

		InD3dDeviceContext->VSSetConstantBuffers(0, 1, &InPerFrameConstBuff);

		// 36 indices for the box.
		InD3dDeviceContext->Draw(6, 0);
	}

}

void FullScreenRenderData::SetSRV(LPCSTR strVarName, ID3D11ShaderResourceView* InSRV)
{
	if (auto pVariable = Effect->GetVariableByName(strVarName))
	{
		if (auto pShaderResource = pVariable->AsShaderResource())
		{
			pShaderResource->SetResource(InSRV);
		}
	}
}

void FullScreenRenderData::ClearSRVs(ID3D11DeviceContext* InD3dDeviceContext, LPCSTR strPassName,LPCSTR strVarNames[], int Count)
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
