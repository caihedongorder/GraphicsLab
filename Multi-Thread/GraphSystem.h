#pragma once
#ifndef GRAPH_SYSTEM_H
#define GRAPH_SYSTEM_H
#include <D3D11.h>
#include <DxErr.h>
#include <xnamath.h>
#include "World.h"
#include "FullScreenRenderData.h"


#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                      \
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

//---------------------------------------------------------------------------------------
// Convenience macro for releasing COM objects.
//---------------------------------------------------------------------------------------

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

//---------------------------------------------------------------------------------------
// Convenience macro for deleting objects.
//---------------------------------------------------------------------------------------

#define SafeDelete(x) { delete x; x = 0; }

//---------------------------------------------------------------------------------------
// Utility classes.
//---------------------------------------------------------------------------------------

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/gtc/constants.hpp> // glm::pi
#include <glm/gtc/quaternion.hpp>

namespace Colors
{
	extern glm::vec4 White;
	extern glm::vec4 Black;
	extern glm::vec4 Red;
	extern glm::vec4 Green;
	extern glm::vec4 Blue;
	extern glm::vec4 Yellow;
	extern glm::vec4 Cyan;
	extern glm::vec4 Magenta;

	extern glm::vec4 Silver;
	extern glm::vec4 LightSteelBlue;
}

#define GBUFFER_NUM 2

class GraphSystem
{
public:
	GraphSystem(World *InWorld);
	~GraphSystem();


	bool InitDirect3D(HWND InhWnd,int InClientWidth,int InClientHeight);



	void OnResize();


	void BeginRender();

	void WaitforFinishRender();

	ID3D11Device* GetD3dDevice() { return md3dDevice; }

private:
	static void WINAPI OnRenderThreadProc(void * InGraphSystem);
	void OnRender();

	void OnForwardRender();
	void OnRenderForwardBasePass();


	void OnDeferedRender();
	void OnRenderDeferedBasePass();
	void OnCompositeGraphics();

private:
	bool CreateGBuffer();
	void ReleaseGBuffer();

	void CreateCompositeQuad();

private:
	D3D_DRIVER_TYPE md3dDriverType;

	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;
	IDXGISwapChain* mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;

	ID3D11Texture2D* mGBufferTexture[GBUFFER_NUM];
	ID3D11RenderTargetView* mGBufferRTVs[GBUFFER_NUM];
	ID3D11ShaderResourceView* mGBufferSRVs[GBUFFER_NUM];


	std::shared_ptr<FullScreenRenderData> CompositeQuad;

	D3D11_VIEWPORT mScreenViewport;

	UINT		m4xMsaaQuality;
	int			mClientWidth;
	int			mClientHeight;
	HWND		mhWnd;
	bool mEnable4xMsaa;

	HANDLE hRenderThreadHandle;
	HANDLE hEventBeginRender;
	HANDLE hEventFinishRender;

	World *mWorld;
};

#endif
