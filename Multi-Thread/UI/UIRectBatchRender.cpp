#include "stdafx.h"
#include "UIRectBatchRender.h"
#include "../GraphSystem.h"
#include <glm/ext/matrix_transform.hpp>
#include <d3d11.h>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "UISystem.h"
#include "../MathUtil.h"

extern std::shared_ptr<GraphSystem> GGraphSystem;

UIRectBatchRender::RectRenderElementInGPU::RectRenderElementInGPU()
{

}

UIRectBatchRender::RectRenderElementInGPU::~RectRenderElementInGPU()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mInputLayout);
}

UIRectBatchRender::UIRectBatchRender()
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
	auto D3dDeviceContext = GGraphSystem->GetD3dDeviceContext();
	for (auto EffectItInGPU = AllBatchElementsInGPU.begin(); EffectItInGPU != AllBatchElementsInGPU.end();++EffectItInGPU)
	{
		const TechName2PassRenderCollectionsInGPU& tech2PassRenderCollectionsInGPU = EffectItInGPU->second;
		for (auto TechItInGPU = tech2PassRenderCollectionsInGPU.begin(); TechItInGPU != tech2PassRenderCollectionsInGPU.end(); ++TechItInGPU)
		{
			auto pEffectTech = EffectItInGPU->first->GetTechniqueByName(TechItInGPU->first.c_str());
			const PassName2RenderElementsInGPU& passName2ElementInGPU = TechItInGPU->second;
			for (auto PassItInGPU = passName2ElementInGPU.begin(); PassItInGPU != passName2ElementInGPU.end(); ++PassItInGPU)
			{
				if (auto pBasePass = pEffectTech->GetPassByName(PassItInGPU->first.c_str()))
				{
					D3dDeviceContext->IASetInputLayout(PassItInGPU->second.mInputLayout);
					D3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					UINT stride = sizeof(FVectex);
					UINT offset = 0;
					D3dDeviceContext->IASetVertexBuffers(0, 1, &PassItInGPU->second.mVB, &stride, &offset);

					pBasePass->Apply(0, D3dDeviceContext);

					D3dDeviceContext->Draw(PassItInGPU->second.VectexCount, 0);
				}
			}
		}
	}

}

void UIRectBatchRender::DrawInCPU(ID3DX11Effect* InEffect, const std::string& InTechName, const std::string& InPassName, const RectRenderElementInCPU& InElement)
{
	auto EffectIt = AllBatchElementsInCPU.find(InEffect);
	if (EffectIt == AllBatchElementsInCPU.end())
	{
		EffectIt = AllBatchElementsInCPU.insert(std::make_pair(InEffect, TechName2PassRenderCollectionsInCPU()));
	}

	TechName2PassRenderCollectionsInCPU& tech2PassRenderCollections = EffectIt->second;
	auto TechIt = tech2PassRenderCollections.find(InTechName);
	if (TechIt == tech2PassRenderCollections.end())
	{
		TechIt = tech2PassRenderCollections.insert(std::make_pair(InTechName, PassName2RenderElementsInCPU()));
	}

	PassName2RenderElementsInCPU& passName2Element = TechIt->second;
	auto PassIt = passName2Element.find(InPassName);
	if (PassIt == passName2Element.end())
	{
		PassIt = passName2Element.insert(std::make_pair(InPassName, std::vector<RectRenderElementInCPU>()));
	}

	PassIt->second.push_back(InElement);

}

void UIRectBatchRender::PostRender()
{

	//清除 cpu上不存在的 但是存在于gpu上的元素

	for (auto EffectItInGPU = AllBatchElementsInGPU.begin(); EffectItInGPU != AllBatchElementsInGPU.end();)
	{
		auto EffectItInCPU = AllBatchElementsInCPU.find(EffectItInGPU->first);
		if (EffectItInCPU != AllBatchElementsInCPU.end())
		{
			TechName2PassRenderCollectionsInGPU& tech2PassRenderCollectionsInGPU = EffectItInGPU->second;
			for (auto TechItInGPU = tech2PassRenderCollectionsInGPU.begin(); TechItInGPU != tech2PassRenderCollectionsInGPU.end();)
			{
				const TechName2PassRenderCollectionsInCPU& tech2PassRenderCollectionsInCPU = EffectItInCPU->second;
				auto TechItInCPU = tech2PassRenderCollectionsInCPU.find(TechItInGPU->first);
				if (TechItInCPU != tech2PassRenderCollectionsInCPU.end())
				{
					++TechItInGPU;
				}
				else
				{
					tech2PassRenderCollectionsInGPU.erase(TechItInGPU->first);
					TechItInGPU++;
				}
			}

			++EffectItInGPU;
		}
		else
		{
			AllBatchElementsInGPU.erase(EffectItInGPU->first);
			EffectItInGPU++;
		}
	}

	auto D3dDeviceContext = GGraphSystem->GetD3dDeviceContext();

	//cpu数据 同步到 gpu 
	for (auto EffectItInCPU = AllBatchElementsInCPU.begin() ; EffectItInCPU != AllBatchElementsInCPU.end() ; ++EffectItInCPU)
	{
		const TechName2PassRenderCollectionsInCPU& tech2PassRenderCollectionsInCPU = EffectItInCPU->second;

		auto EffectItInGPU = AllBatchElementsInGPU.find(EffectItInCPU->first);
		if (EffectItInGPU == AllBatchElementsInGPU.end())
		{
			EffectItInGPU = AllBatchElementsInGPU.insert(std::make_pair(EffectItInCPU->first, TechName2PassRenderCollectionsInGPU()));
		}
		TechName2PassRenderCollectionsInGPU& tech2PassRenderCollectionsInGPU = EffectItInGPU->second;

		for (auto TechItInCPU = tech2PassRenderCollectionsInCPU.begin(); TechItInCPU != tech2PassRenderCollectionsInCPU.end(); ++TechItInCPU)
		{
			auto TechItInGPU = tech2PassRenderCollectionsInGPU.find(TechItInCPU->first);
			if (TechItInGPU == tech2PassRenderCollectionsInGPU.end())
			{
				TechItInGPU = tech2PassRenderCollectionsInGPU.insert(std::make_pair(TechItInCPU->first, PassName2RenderElementsInGPU()));
			}
			PassName2RenderElementsInGPU& passName2ElementInGPU = TechItInGPU->second;


			const PassName2RenderElementsInCPU& passName2ElementInCPU = TechItInCPU->second;
	

			for (auto PassItInCPU = passName2ElementInCPU.begin(); PassItInCPU != passName2ElementInCPU.end(); ++PassItInCPU)
			{
				auto PassItInGPU = passName2ElementInGPU.find(PassItInCPU->first);
				if (PassItInGPU == passName2ElementInGPU.end())
				{
					PassItInGPU = passName2ElementInGPU.insert(std::make_pair(PassItInCPU->first, RectRenderElementInGPU()));
					CreateInputLayout(GGraphSystem->GetD3dDevice(), EffectItInGPU->first, TechItInGPU->first, PassItInGPU->first, PassItInGPU->second.mInputLayout);
				}
				int ElementCount = PassItInCPU->second.size();
				if (ElementCount > 0)
				{
					int VertexCount = ElementCount * 6;

					RectRenderElementInGPU& ElementInGPU = PassItInGPU->second;
					if (ElementInGPU.VectexCount < VertexCount)
					{
						ReleaseCOM(ElementInGPU.mVB);

						D3D11_BUFFER_DESC vbd;
						vbd.Usage = D3D11_USAGE_DYNAMIC;
						vbd.ByteWidth = sizeof(FVectex) * VertexCount;
						vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
						vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
						vbd.MiscFlags = 0;
						vbd.StructureByteStride = 0;
						D3D11_SUBRESOURCE_DATA vinitData;
						vinitData.pSysMem = PassItInCPU->second.data();
						HR(GGraphSystem->GetD3dDevice()->CreateBuffer(&vbd, &vinitData, &ElementInGPU.mVB));

						ElementInGPU.VectexCount = VertexCount;
					}
					else
					{
						D3D11_MAPPED_SUBRESOURCE mappedResource;
						auto result = D3dDeviceContext->Map(ElementInGPU.mVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
						if (SUCCEEDED(result))
						{

							// Get a pointer to the data in the constant buffer.
							auto dataPtr = (PerFrameData *)mappedResource.pData;

							memcpy(mappedResource.pData, PassItInCPU->second.data(), sizeof(FVectex) * VertexCount);

							D3dDeviceContext->Unmap(ElementInGPU.mVB, 0);
						}
					}
				}
			}
		}
	}
}

void UIRectBatchRender::BeginDraw()
{
	AllBatchElementsInCPU.clear();
}

void UIRectBatchRender::EndDraw()
{

}

