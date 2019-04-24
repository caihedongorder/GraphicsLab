#pragma once
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11Effect.h>
#include "Transform.h"

class FullScreenRenderData
{
public:
	struct FVectex
	{
		glm::vec2 UV;
	};
public:
	FullScreenRenderData();
	~FullScreenRenderData();

	void Init(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect);

	void OnRender(ID3D11DeviceContext* InD3dDeviceContext, ID3D11Buffer* InPerFrameConstBuff, LPCSTR strPassName);

	void SetSRV(LPCSTR strVarName, ID3D11ShaderResourceView* InSRV);

	void ClearSRVs(ID3D11DeviceContext* InD3dDeviceContext, LPCSTR strPassName,LPCSTR strVarNames[],int Count);


private:
	ID3D11Buffer* mVB;
	ID3D11InputLayout* mInputLayout;
	ID3DX11Effect* Effect;
	ID3DX11EffectTechnique* mTech;

};

