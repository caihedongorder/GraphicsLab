#include "stdafx.h"
#include "World.h"
#include "GraphSystem.h"
#include "ShaderManager.h"


World::World()
	:mPerFrameConstBuff(nullptr)
{
}


World::~World()
{
	ReleaseCOM(mPerFrameConstBuff);
}

void World::Init(ID3D11Device* Ind3dDevice)
{
	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(PerFrameData);
	vbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	HR(Ind3dDevice->CreateBuffer(&vbd, nullptr, &mPerFrameConstBuff));


	//初始化 相机和投影矩阵
	float mPhi = glm::radians(180.0f) * 0.25f;
	float mTheta = glm::radians(180.0f) * 1.5f;
	float mRadius = 5.0f;
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	auto local2WorldMat = glm::mat4(1.0f);

	mPerFrameData.viewMat = glm::lookAtLH(glm::vec3(x, y, z),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0));

	mPerFrameData.projMat = glm::perspectiveLH(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 1000.0f);
}

void World::CreateBox(ID3D11Device* Ind3dDevice, const Transform& InLocal2WorldTransform)
{
	auto pBoxMesh = std::shared_ptr<StaticMeshRenderData>(new StaticMeshRenderData(InLocal2WorldTransform));

	StaticMeshRenderData::FVectex vertices[] =
	{
		{ glm::vec3(-1.0, 1.0,-1.0),	glm::vec3(0.0, 0.0, -1.0) ,glm::vec2(0.0,0.0)},
		{ glm::vec3(1.0, 1.0,-1.0),		glm::vec3(0.0, 0.0, -1.0) ,glm::vec2(1.0,0.0)},
		{ glm::vec3(-1.0,-1.0,-1.0),	glm::vec3(0.0, 0.0, -1.0) ,glm::vec2(0.0,1.0)},
		{ glm::vec3(-1.0,-1.0,-1.0),	glm::vec3(0.0, 0.0, -1.0) ,glm::vec2(0.0,1.0)},
		{ glm::vec3(1.0, 1.0,-1.0),		glm::vec3(0.0, 0.0, -1.0) ,glm::vec2(1.0,0.0)},
		{ glm::vec3(1.0,-1.0,-1.0),		glm::vec3(0.0, 0.0, -1.0) ,glm::vec2(1.0,1.0)},
		
		{ glm::vec3(1.0 , 1.0, -1.0),	glm::vec3(1.0, 0.0, 0.0) ,glm::vec2(0.0, 0.0)},
		{ glm::vec3(1.0 , 1.0,  1.0),	glm::vec3(1.0, 0.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(1.0 ,-1.0, -1.0),	glm::vec3(1.0, 0.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(1.0 ,-1.0, -1.0),	glm::vec3(1.0, 0.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(1.0 , 1.0,  1.0),	glm::vec3(1.0, 0.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(1.0 ,-1.0,  1.0),	glm::vec3(1.0, 0.0, 0.0) ,glm::vec2(1.0, 1.0)},

		{ glm::vec3(1.0 , 1.0,  1.0),	glm::vec3(0.0, 0.0, 1.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(-1.0 , 1.0,  1.0),	glm::vec3(0.0, 0.0, 1.0) ,glm::vec2(1.0, 1.0)},
		{ glm::vec3(1.0 ,-1.0,  1.0),	glm::vec3(0.0, 0.0, 1.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(1.0 ,-1.0,  1.0),	glm::vec3(0.0, 0.0, 1.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(-1.0 , 1.0,  1.0),	glm::vec3(0.0, 0.0, 1.0) ,glm::vec2(1.0, 1.0)},
		{ glm::vec3(-1.0 ,-1.0,  1.0),	glm::vec3(0.0, 0.0, 1.0) ,glm::vec2(1.0, 1.0)},

		{ glm::vec3(-1.0 , 1.0,  1.0),	glm::vec3(-1.0, 0.0, 0.0) ,glm::vec2(0.0, 0.0)},
		{ glm::vec3(-1.0 , 1.0, -1.0),	glm::vec3(-1.0, 0.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(-1.0 ,-1.0,  1.0),	glm::vec3(-1.0, 0.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(-1.0 ,-1.0,  1.0),	glm::vec3(-1.0, 0.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(-1.0 , 1.0, -1.0),	glm::vec3(-1.0, 0.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(-1.0 ,-1.0, -1.0),	glm::vec3(-1.0, 0.0, 0.0) ,glm::vec2(1.0, 1.0)},

		{ glm::vec3(-1.0 , 1.0,  1.0),	glm::vec3(0.0, 1.0, 0.0) ,glm::vec2(0.0, 0.0)},
		{ glm::vec3(1.0 , 1.0,  1.0),	glm::vec3(0.0, 1.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(-1.0 , 1.0, -1.0),	glm::vec3(0.0, 1.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(-1.0 , 1.0, -1.0),	glm::vec3(0.0, 1.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(1.0 , 1.0,  1.0),	glm::vec3(0.0, 1.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(1.0 , 1.0, -1.0),	glm::vec3(0.0, 1.0, 0.0) ,glm::vec2(1.0, 1.0)},

		{ glm::vec3(-1.0 ,-1.0, -1.0),	glm::vec3(0.0, -1.0, 0.0) ,glm::vec2(0.0, 0.0)},
		{ glm::vec3(1.0 ,-1.0, -1.0),	glm::vec3(0.0, -1.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(-1.0 ,-1.0,  1.0),	glm::vec3(0.0, -1.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(-1.0 ,-1.0,  1.0),	glm::vec3(0.0, -1.0, 0.0) ,glm::vec2(0.0, 1.0)},
		{ glm::vec3(1.0 ,-1.0, -1.0),	glm::vec3(0.0, -1.0, 0.0) ,glm::vec2(1.0, 0.0)},
		{ glm::vec3(1.0 ,-1.0,  1.0),	glm::vec3(0.0, -1.0, 0.0) ,glm::vec2(1.0, 1.0)},
	};

	UINT indices[] = {
		// front face
		0, 1, 2,
		3, 4, 5,

		// back face
		6, 7, 8,
		9, 10, 11,

		// left face
		12, 13, 14,
		15, 16, 17,

		// right face
		18, 19, 20,
		21, 22, 23,

		// top face
		24, 25, 26,
		27, 28, 29,

		// bottom face
		30, 31, 32,
		33, 34, 35
	};

	auto pEffect = ShaderManager::GetInstance()->GetShader(Ind3dDevice, TEXT("FX/StaticMesh.fx"));
	pBoxMesh->Init(Ind3dDevice, vertices, sizeof(vertices) / sizeof(vertices[0]), indices, sizeof(indices) / sizeof(indices[0]), pEffect);

	allRenderDatas.push_back(pBoxMesh);
}

void World::OnRenderDeferedBasePass(ID3D11DeviceContext* InD3dDeviceContext)
{
	for (auto It = allRenderDatas.begin() ; It != allRenderDatas.end() ; ++It)
	{
		(*It)->OnRenderDeferedBasePass(InD3dDeviceContext, mPerFrameConstBuff);
	}
}

void World::OnRenderForwardBasePass(ID3D11DeviceContext* InD3dDeviceContext)
{
	for (auto It = allRenderDatas.begin(); It != allRenderDatas.end(); ++It)
	{
		(*It)->OnRenderForwardBasePass(InD3dDeviceContext, mPerFrameConstBuff);
	}
}

void World::UpdatePerFrameCBuffer(ID3D11DeviceContext* InD3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	auto result = InD3dDeviceContext->Map(mPerFrameConstBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(result))
	{
		// Get a pointer to the data in the constant buffer.
		auto dataPtr = (PerFrameData *)mappedResource.pData;

		// Copy the matrices into the constant buffer.
		dataPtr->viewMat = glm::transpose(mPerFrameData.viewMat);
		dataPtr->projMat = glm::transpose(mPerFrameData.projMat);

		InD3dDeviceContext->Unmap(mPerFrameConstBuff, 0);
	}
}
