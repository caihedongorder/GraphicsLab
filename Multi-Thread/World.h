#pragma once
#include <list>
#include <memory>
#include "StaticMeshRenderData.h"

struct PerFrameData 
{
	glm::mat4 viewMat;
	glm::mat4 projMat;
};

class World
{
public:
	World();
	~World();

	void Init(ID3D11Device* Ind3dDevice);

	void CreateBox(ID3D11Device* Ind3dDevice,const Transform& InLocal2WorldTransform);

	void OnRenderBasePass(ID3D11DeviceContext* InD3dDeviceContext);

	void UpdatePerFrameCBuffer(ID3D11DeviceContext* InD3dDeviceContext);

private:
	std::list<std::shared_ptr<StaticMeshRenderData>> allRenderDatas;

	PerFrameData mPerFrameData;
	ID3D11Buffer* mPerFrameConstBuff;

};

