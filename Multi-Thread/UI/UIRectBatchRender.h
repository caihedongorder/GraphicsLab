#pragma once

#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11Effect.h>
#include "../Transform.h"
#include "UITransform.h"
#include <map>
#include <vector>
#include <memory>

#include "../CriticalSection.h"
#include "../LockFreeMemoryPool.h"

class UIRectBatchRender
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

	struct RectRenderElementInCPU
	{
		FVectex VertexData[6];
	};

	class RectRenderEffectInfo
	{
	public:
		RectRenderEffectInfo(ID3DX11EffectPass* InPass, int InIndex);
		~RectRenderEffectInfo();
		ID3DX11EffectPass* Pass;
		int VertexCount;
		ID3D11Buffer* mVB;
		ID3D11InputLayout* mInputLayout;
		int index;
		TLockFreeMemoryPool<RectRenderElementInCPU> RenderElementMemoryPool;
	};
	
public:
	UIRectBatchRender();
	~UIRectBatchRender();

	void OnRender();

	void DrawInCPU(int InEffectIdx, const RectRenderElementInCPU& InElement);

	void PostRender();

	void BeginFrame();
	void EndFrame();

	void RegisterEffect(ID3DX11EffectPass* InPass, int& OutEffectIndex);


private:
	void CreateInputLayout(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect, const std::string& InTechName, const std::string& InPassName, ID3D11InputLayout*& OutInputLayout);
private:

	TLockFreeMemoryPool<LONG> _RenderEffectIdxMemoryPool;
	std::vector<std::shared_ptr<RectRenderEffectInfo>> _EffectInfos;
};