#include "stdafx.h"
#include "UIRectBatchRender.h"
#include "../GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "UISystem.h"
#include "../MathUtil.h"
#include <memory>
#include "../GraphicsLabSystem.h"

UIRectBatchRender::RectRenderEffectInfo::RectRenderEffectInfo(ID3DX11EffectPass* InPass, int InIndex) :Pass(InPass)
, index(InIndex)
, RenderElementMemoryPool(10000)
, VertexCount(0)
, mVB(nullptr)
, mInputLayout(nullptr)
{
	// Create the vertex input layout.
	const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"CLIP", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"CW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"LA", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"ROTATE", 0, DXGI_FORMAT_R32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	InPass->GetDesc(&passDesc);
	HR(GraphicsLabSystem::GetInstance()->GetGraphSystem()->GetD3dDevice()->CreateInputLayout(vertexDesc, sizeof(vertexDesc) / sizeof(vertexDesc[0]), passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}



UIRectBatchRender::RectRenderEffectInfo::~RectRenderEffectInfo()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mInputLayout);
}

UIRectBatchRender::UIRectBatchRender()
	:_RenderEffectIdxMemoryPool(10000)
{

}


UIRectBatchRender::~UIRectBatchRender()
{
}

void UIRectBatchRender::CreateInputLayout(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect,const std::string& InTechName,const std::string& InPassName, ID3D11InputLayout*& OutInputLayout)
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"CLIP", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"CW", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"LA", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"ROTATE", 0, DXGI_FORMAT_R32_FLOAT, 0, 64, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	auto pTech = InEffect->GetTechniqueByName(InTechName.c_str());

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	pTech->GetPassByName(InPassName.c_str())->GetDesc(&passDesc);
	HR(Ind3dDevice->CreateInputLayout(vertexDesc, sizeof(vertexDesc) / sizeof(vertexDesc[0]), passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &OutInputLayout));
}

void UIRectBatchRender::OnRender()
{
	auto D3dDeviceContext = GraphicsLabSystem::GetInstance()->GetGraphSystem()->GetD3dDeviceContext();
	//渲染
	for (auto EffectIt = _EffectInfos.begin(); EffectIt != _EffectInfos.end(); ++EffectIt)
	{
		const RectRenderEffectInfo* pEffectInfo = (*EffectIt).get();

		D3dDeviceContext->IASetInputLayout(pEffectInfo->mInputLayout);
		D3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		UINT stride = sizeof(FVectex);
		UINT offset = 0;
		D3dDeviceContext->IASetVertexBuffers(0, 1, &pEffectInfo->mVB, &stride, &offset);

		pEffectInfo->Pass->Apply(0, D3dDeviceContext);

		D3dDeviceContext->Draw(pEffectInfo->VertexCount, 0);
	}
}

void UIRectBatchRender::DrawInCPU(int InEffectIdx,const RectRenderElementInCPU& InElement)
{
	new(_RenderEffectIdxMemoryPool.Alloc(1))int(InEffectIdx);
	new(_EffectInfos[InEffectIdx]->RenderElementMemoryPool.Alloc(1))RectRenderElementInCPU(InElement);
}

void UIRectBatchRender::PostRender()
{
	auto D3dDeviceContext = GraphicsLabSystem::GetInstance()->GetGraphSystem()->GetD3dDeviceContext();

	//cpu数据 同步到 gpu 
	for (auto EffectIt = _EffectInfos.begin() ; EffectIt != _EffectInfos.end() ; ++EffectIt)
	{
		RectRenderEffectInfo* pEffectInfo = (*EffectIt).get();
		int ElementCount = pEffectInfo->RenderElementMemoryPool.ElementCount();
		int VertexCount = ElementCount * 6;
		if (pEffectInfo->VertexCount < VertexCount)
		{
			ReleaseCOM(pEffectInfo->mVB);

			D3D11_BUFFER_DESC vbd;
			vbd.Usage = D3D11_USAGE_DYNAMIC;
			vbd.ByteWidth = sizeof(FVectex) * VertexCount;
			vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			vbd.MiscFlags = 0;
			vbd.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA vinitData;
			vinitData.pSysMem = pEffectInfo->RenderElementMemoryPool.GetBasePtr();
			HR(GraphicsLabSystem::GetInstance()->GetGraphSystem()->GetD3dDevice()->CreateBuffer(&vbd, &vinitData, &pEffectInfo->mVB));

			pEffectInfo->VertexCount = VertexCount;
		}
		else
		{
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			auto result = D3dDeviceContext->Map(pEffectInfo->mVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (SUCCEEDED(result))
			{
				// Get a pointer to the data in the constant buffer.
				auto dataPtr = (PerFrameData *)mappedResource.pData;

				memcpy(mappedResource.pData, pEffectInfo->RenderElementMemoryPool.GetBasePtr(), sizeof(FVectex) * VertexCount);

				D3dDeviceContext->Unmap(pEffectInfo->mVB, 0);
			}
		}
	}
}

void UIRectBatchRender::BeginFrame()
{
	_RenderEffectIdxMemoryPool.BeginFrame();
	for (auto it = _EffectInfos.begin(); it != _EffectInfos.end(); ++it)
	{
		(*it)->RenderElementMemoryPool.BeginFrame();
	}
}

void UIRectBatchRender::EndFrame()
{
	_RenderEffectIdxMemoryPool.EndFrame();

	for (auto it = _EffectInfos.begin(); it != _EffectInfos.end(); ++it)
	{
		(*it)->RenderElementMemoryPool.EndFrame();
	}
}

void UIRectBatchRender::RegisterEffect(ID3DX11EffectPass* InPass, int& OutEffectIndex)
{
	for (auto it = _EffectInfos.begin() ; it != _EffectInfos.end() ; ++it)
	{
		if (InPass == (*it)->Pass)
		{
			OutEffectIndex = (*it)->index;
			return;
		}
	}
	OutEffectIndex = _EffectInfos.size();

	_EffectInfos.push_back(std::make_shared<RectRenderEffectInfo>(InPass, OutEffectIndex));
}

