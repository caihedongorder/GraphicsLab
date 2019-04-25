#pragma once
#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11Effect.h>
#include "../Transform.h"
#include "UITransform.h"

class UIImageRender
{
public:
	struct FVectex
	{
		glm::vec4 ClipRect;					//当前控件裁剪矩形
		glm::vec4 TranslateAndScale;		//xy : translate zw : Scale
		glm::vec4 CanvasSizeAndWidgetSize;	//xy : FrameSize zw : WidgetSize
		glm::vec4 LocationAndAnchor;		//xy :  Location zw : Anchor
		float RotateAngle;
	};
public:
	UIImageRender();
	~UIImageRender();

	void Init(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect,const UITransform& InTransform,
		const glm::vec2& InSize, const glm::vec2& InClipSize, const glm::vec2& InAnchor
		, const glm::vec2& InCanvasSize);

	void OnRender(ID3D11DeviceContext* InD3dDeviceContext,LPCSTR strPassName);

	void SetSRV(LPCSTR strVarName, ID3D11ShaderResourceView* InSRV);

	void ClearSRVs(ID3D11DeviceContext* InD3dDeviceContext, LPCSTR strPassName, LPCSTR strVarNames[], int Count);

	void PostRender(const UITransform& InTransform);

private:
	void GetVertexData(FVectex* OutVertexData);
private:
	ID3D11Buffer* mVB;
	ID3D11InputLayout* mInputLayout;
	ID3DX11Effect* Effect;
	ID3DX11EffectTechnique* mTech;

	UITransform _Transform;
	glm::vec2 _Size;
	glm::vec2 _ClipSize;
	glm::vec2 _Anchor;
	glm::vec2 _CanvasSize;
};