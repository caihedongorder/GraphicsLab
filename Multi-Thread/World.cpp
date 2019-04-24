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
		{ glm::vec3(-1.0f, -1.0f, -1.0f), Colors::White   },
		{ glm::vec3(-1.0f, +1.0f, -1.0f), Colors::Black   },
		{ glm::vec3(+1.0f, +1.0f, -1.0f), Colors::Red     },
		{ glm::vec3(+1.0f, -1.0f, -1.0f), Colors::Green   },
		{ glm::vec3(-1.0f, -1.0f, +1.0f), Colors::Blue    },
		{ glm::vec3(-1.0f, +1.0f, +1.0f), Colors::Yellow  },
		{ glm::vec3(+1.0f, +1.0f, +1.0f), Colors::Cyan    },
		{ glm::vec3(+1.0f, -1.0f, +1.0f), Colors::Magenta }
	};

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	auto pEffect = ShaderManager::GetInstance()->GetShader(Ind3dDevice, TEXT("FX/color.fx"));
	pBoxMesh->Init(Ind3dDevice, vertices, sizeof(vertices) / sizeof(vertices[0]), indices, sizeof(indices) / sizeof(indices[0]), pEffect);

	allRenderDatas.push_back(pBoxMesh);
}

void World::OnRenderBasePass(ID3D11DeviceContext* InD3dDeviceContext)
{
	for (auto It = allRenderDatas.begin() ; It != allRenderDatas.end() ; ++It)
	{
		(*It)->OnRenderBasePass(InD3dDeviceContext, mPerFrameConstBuff);
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
