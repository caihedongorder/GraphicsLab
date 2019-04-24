#pragma once
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11Effect.h>
#include "Transform.h"



class StaticMeshRenderData
{
public:
	struct FVectex 
	{
		glm::vec3 Pos;
		glm::vec3 Normal;
		glm::vec2 uv;
	};
public:
	StaticMeshRenderData(const Transform& InLocal2WorldTransform);
	~StaticMeshRenderData();

	void Init(ID3D11Device* Ind3dDevice,const FVectex* InVectexBasePtr, int InVertexCount, const UINT* InIndicesBasePtr, int InIndicesCount,
		ID3DX11Effect* InEffect);

	void OnRenderDeferedBasePass(ID3D11DeviceContext* InD3dDeviceContext, ID3D11Buffer* InPerFrameConstBuff);
	void OnRenderForwardBasePass(ID3D11DeviceContext* InD3dDeviceContext, ID3D11Buffer* InPerFrameConstBuff);


private:
	ID3D11Buffer* mIB;
	ID3D11Buffer* mVB;
	ID3DX11Effect* Effect;
	ID3DX11EffectTechnique* mTech;
	ID3D11InputLayout* mInputLayout;


	Transform local2World;
};

