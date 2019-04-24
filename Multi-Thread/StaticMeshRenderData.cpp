#include "stdafx.h"
#include "StaticMeshRenderData.h"
#include "GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>

StaticMeshRenderData::StaticMeshRenderData(const Transform& InLocal2WorldTransform)
	:local2World(InLocal2WorldTransform)
	, mIB(NULL)
	, mVB(NULL)
	, Effect(NULL)
	, mTech(NULL)
	, mInputLayout(NULL)
{

}


StaticMeshRenderData::~StaticMeshRenderData()
{
	ReleaseCOM(mIB);
	ReleaseCOM(mVB);
	ReleaseCOM(mInputLayout);
}

void StaticMeshRenderData::Init(ID3D11Device* Ind3dDevice, const FVectex* InVectexBasePtr, int InVertexCount, const UINT* InIndicesBasePtr, int InIndicesCount, ID3DX11Effect* InEffect)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(FVectex) * InVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = InVectexBasePtr;
	HR(Ind3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * InIndicesCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = InIndicesBasePtr;
	HR(Ind3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));

	Effect = InEffect;

	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	mTech = Effect->GetTechniqueByName("ColorTech");

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(Ind3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}

void StaticMeshRenderData::OnRenderDeferedBasePass(ID3D11DeviceContext* InD3dDeviceContext, ID3D11Buffer* InPerFrameConstBuff)
{
	auto local2WorldMat = glm::mat4(1.0f);

	auto WorldViewProj = Effect->GetVariableByName("modelMat")->AsMatrix();
	WorldViewProj->SetMatrix(glm::value_ptr(local2WorldMat));

	InD3dDeviceContext->IASetInputLayout(mInputLayout);
	InD3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(FVectex);
	UINT offset = 0;
	InD3dDeviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	InD3dDeviceContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);


	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);

	if (auto pBasePass =  mTech->GetPassByName("DeferedBasePass"))
	{
		pBasePass->Apply(0, InD3dDeviceContext);

		InD3dDeviceContext->VSSetConstantBuffers(0, 1, &InPerFrameConstBuff);

		// 36 indices for the box.
		InD3dDeviceContext->DrawIndexed(36, 0, 0);
	}
	
}

void StaticMeshRenderData::OnRenderForwardBasePass(ID3D11DeviceContext* InD3dDeviceContext, ID3D11Buffer* InPerFrameConstBuff)
{
	auto local2WorldMat = glm::mat4(1.0f);

	auto WorldViewProj = Effect->GetVariableByName("modelMat")->AsMatrix();
	WorldViewProj->SetMatrix(glm::value_ptr(local2WorldMat));

	InD3dDeviceContext->IASetInputLayout(mInputLayout);
	InD3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(FVectex);
	UINT offset = 0;
	InD3dDeviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	InD3dDeviceContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);


	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);

	if (auto pBasePass = mTech->GetPassByName("ForwardBasePass"))
	{
		pBasePass->Apply(0, InD3dDeviceContext);

		InD3dDeviceContext->VSSetConstantBuffers(0, 1, &InPerFrameConstBuff);

		// 36 indices for the box.
		InD3dDeviceContext->DrawIndexed(36, 0, 0);
	}

}
