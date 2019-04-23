#include "stdafx.h"
#include "World.h"
#include "GraphSystem.h"
#include "ShaderManager.h"


World::World()
{
}


World::~World()
{
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

void World::OnRender(ID3D11DeviceContext* InD3dDeviceContext)
{
	for (auto It = allRenderDatas.begin() ; It != allRenderDatas.end() ; ++It)
	{
		(*It)->OnRender(InD3dDeviceContext);
	}
}
