#pragma once

#include <xnamath.h>
#include <d3d11.h>
#include <d3dx11Effect.h>
#include "../Transform.h"
#include "UITransform.h"
#include <map>
#include <vector>

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

	class RectRenderElementInGPU
	{
	public:
		RectRenderElementInGPU();
		~RectRenderElementInGPU();
		int VectexCount = 0;
		ID3D11Buffer* mVB = nullptr;
		ID3D11InputLayout* mInputLayout = nullptr;
	};
public:
	UIRectBatchRender();
	~UIRectBatchRender();

	void OnRender();

	void DrawInCPU(ID3DX11Effect* InEffect, const std::string& InTechName, const std::string& InPassName, const RectRenderElementInCPU& InElement);

	void PostRender();

	void BeginDraw();
	void EndDraw();

private:
	void CreateInputLayout(ID3D11Device* Ind3dDevice, ID3DX11Effect* InEffect, const std::string& InTechName, const std::string& InPassName, ID3D11InputLayout*& OutInputLayout);
private:

	typedef std::multimap<std::string, std::vector<RectRenderElementInCPU>> PassName2RenderElementsInCPU;
	typedef std::multimap<std::string, PassName2RenderElementsInCPU> TechName2PassRenderCollectionsInCPU;
	typedef std::multimap<ID3DX11Effect*, TechName2PassRenderCollectionsInCPU> Effect2TechRenderCollectionsInCPU;

	Effect2TechRenderCollectionsInCPU AllBatchElementsInCPU;

	typedef std::multimap<std::string, RectRenderElementInGPU> PassName2RenderElementsInGPU;
	typedef std::multimap<std::string, PassName2RenderElementsInGPU> TechName2PassRenderCollectionsInGPU;
	typedef std::multimap<ID3DX11Effect*, TechName2PassRenderCollectionsInGPU> Effect2TechRenderCollectionsInGPU;

	Effect2TechRenderCollectionsInGPU AllBatchElementsInGPU;
};