#pragma once
#include <list>
#include <memory>
#include "StaticMeshRenderData.h"

class World
{
public:
	World();
	~World();

	void CreateBox(ID3D11Device* Ind3dDevice,const Transform& InLocal2WorldTransform);

	void OnRender(ID3D11DeviceContext* InD3dDeviceContext);

private:
	std::list<std::shared_ptr<StaticMeshRenderData>> allRenderDatas;
};

