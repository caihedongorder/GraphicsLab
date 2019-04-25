#include "stdafx.h"
#include "GraphSystem.h"
#include <assert.h>
#include "ShaderManager.h"
#include "UI/UISystem.h"

int (WINAPIV * __vsnwprintf)(wchar_t *, size_t, const wchar_t*, va_list) = _vsnwprintf;

glm::vec4 Colors::White = { 1.0f, 1.0f, 1.0f, 1.0f };
glm::vec4 Colors::Black = { 0.0f, 0.0f, 0.0f, 1.0f };
glm::vec4 Colors::Red = { 1.0f, 0.0f, 0.0f, 1.0f };
glm::vec4 Colors::Green = { 0.0f, 1.0f, 0.0f, 1.0f };
glm::vec4 Colors::Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
glm::vec4 Colors::Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
glm::vec4 Colors::Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
glm::vec4 Colors::Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

glm::vec4 Colors::Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
glm::vec4 Colors::LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };

bool GDeferedRender = true;

GraphSystem::GraphSystem(World *InWorld)
	:
	mWorld(InWorld),
	md3dDriverType(D3D_DRIVER_TYPE_HARDWARE),
	mClientWidth(800),
	mClientHeight(600),
	mEnable4xMsaa(false),
	m4xMsaaQuality(0),
	md3dDevice(0),
	md3dImmediateContext(0),
	mSwapChain(0),
	mDepthStencilBuffer(0),
	mRenderTargetView(0),
	mDepthStencilView(0)
	,hRenderThreadHandle(NULL)
	, hEventBeginRender(NULL)
	, hEventFinishRender(NULL)
{
	ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));

	ZeroMemory(mGBufferTexture, sizeof(mGBufferTexture));
	ZeroMemory(mGBufferRTVs, sizeof(mGBufferRTVs));
	ZeroMemory(mGBufferSRVs, sizeof(mGBufferSRVs));

}


GraphSystem::~GraphSystem()
{
	ReleaseGBuffer();

	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(mDepthStencilBuffer);

	// Restore all default settings.
	if (md3dImmediateContext)
		md3dImmediateContext->ClearState();

	ReleaseCOM(md3dImmediateContext);
	ReleaseCOM(md3dDevice);
}

void GraphSystem::ReleaseGBuffer()
{
	for (int iGBuffIdx = 0; iGBuffIdx < GBUFFER_NUM; ++iGBuffIdx)
	{
		ReleaseCOM(mGBufferRTVs[iGBuffIdx]);
		ReleaseCOM(mGBufferSRVs[iGBuffIdx]);
		ReleaseCOM(mGBufferTexture[iGBuffIdx]);
	}

}

void GraphSystem::OnRenderThreadProc(void * InGraphSystem)
{
	GraphSystem* graphSystem = (GraphSystem*)(InGraphSystem);
	while (TRUE)
	{
		::WaitForSingleObject(graphSystem->hEventBeginRender, INFINITE);

		graphSystem->OnRender();

		::SetEvent(graphSystem->hEventFinishRender);

	}
}

void GraphSystem::BeginRender()
{
	::SetEvent(hEventBeginRender);
}

void GraphSystem::WaitforFinishRender()
{
	::WaitForSingleObject(hEventFinishRender, INFINITE);
}

bool GraphSystem::InitDirect3D(HWND InhWnd, int InClientWidth, int InClientHeight)
{
	mClientWidth = InClientWidth;
	mClientHeight = InClientHeight;
	mhWnd = InhWnd;

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = D3D11CreateDevice(
		0,                 // default adapter
		md3dDriverType,
		0,                 // no software device
		createDeviceFlags,
		0, 0,              // default feature level array
		D3D11_SDK_VERSION,
		&md3dDevice,
		&featureLevel,
		&md3dImmediateContext);

	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
		return false;
	}

	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return false;
	}

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	HR(md3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
	assert(m4xMsaaQuality > 0);

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Use 4X MSAA? 
	if (mEnable4xMsaa)
	{
		sd.SampleDesc.Count = 4;
		sd.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
	}

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = mhWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice* dxgiDevice = 0;
	HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));

	IDXGIAdapter* dxgiAdapter = 0;
	HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

	IDXGIFactory* dxgiFactory = 0;
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

	HR(dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain));

	ReleaseCOM(dxgiDevice);
	ReleaseCOM(dxgiAdapter);
	ReleaseCOM(dxgiFactory);

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.

	
	DWORD ThreadID;
	hRenderThreadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&GraphSystem::OnRenderThreadProc, (LPVOID)this, 0, &ThreadID);

	hEventBeginRender = ::CreateEvent(NULL, FALSE, FALSE, TEXT("BeginRender"));
	hEventFinishRender = ::CreateEvent(NULL, FALSE, FALSE, TEXT("FinishRender"));

	CreateCompositeQuad();

	return true;
}


void GraphSystem::CreateCompositeQuad()
{
	CompositeQuad = std::shared_ptr<FullScreenRenderData>(new FullScreenRenderData);
	CompositeQuad->Init(md3dDevice, ShaderManager::GetInstance()->GetShader(md3dDevice, TEXT("FX/Composite.fx")));
}

bool GraphSystem::CreateGBuffer()
{
	ReleaseGBuffer();

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = mClientWidth;
	textureDesc.Height = mClientHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;


	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	for (int iGBuffIdx = 0; iGBuffIdx < GBUFFER_NUM; ++iGBuffIdx)
	{
		HR(md3dDevice->CreateTexture2D(&textureDesc, NULL, &mGBufferTexture[iGBuffIdx]));

		HR(md3dDevice->CreateRenderTargetView(mGBufferTexture[iGBuffIdx], &renderTargetViewDesc, &mGBufferRTVs[iGBuffIdx]));

		HR(md3dDevice->CreateShaderResourceView(mGBufferTexture[iGBuffIdx], &shaderResourceViewDesc, &mGBufferSRVs[iGBuffIdx]));
	}

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	return true;
}

void GraphSystem::OnResize()
{
	assert(md3dImmediateContext);
	assert(md3dDevice);
	assert(mSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mDepthStencilBuffer);


	// Resize the swap chain and recreate the render target view.

	HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
	ReleaseCOM(backBuffer);

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));


	// Bind the render target view and depth/stencil view to the pipeline.

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	// Set the viewport transform.

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mClientWidth);
	mScreenViewport.Height = static_cast<float>(mClientHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);


	//创建GBuffer
	CreateGBuffer();
}

void GraphSystem::OnRender()
{
	mWorld->UpdatePerFrameCBuffer(md3dImmediateContext);

	if (GDeferedRender)
	{
		OnDeferedRender();
	}
	else
	{
		OnForwardRender();
	}

	UISystem::GetInstance()->OnRender(md3dImmediateContext);


	HR(mSwapChain->Present(0, 0));

}

void GraphSystem::OnForwardRender()
{
	OnRenderForwardBasePass();
}

void GraphSystem::OnRenderForwardBasePass()
{
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);

	mWorld->OnRenderForwardBasePass(md3dImmediateContext);
}

void GraphSystem::OnDeferedRender()
{
	OnRenderDeferedBasePass();
	OnCompositeGraphics();
}

void GraphSystem::OnRenderDeferedBasePass()
{
	md3dImmediateContext->OMSetRenderTargets(GBUFFER_NUM, mGBufferRTVs, mDepthStencilView);

	for (int iGBufferIdx = 0; iGBufferIdx < GBUFFER_NUM; iGBufferIdx++)
	{
		md3dImmediateContext->ClearRenderTargetView(mGBufferRTVs[iGBufferIdx], reinterpret_cast<const float*>(&Colors::Black));
	}
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);


	mWorld->OnRenderDeferedBasePass(md3dImmediateContext);

}

void GraphSystem::OnCompositeGraphics()
{
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_STENCIL, 1.0f, 0);


	CompositeQuad->SetSRV("colorTexture", mGBufferSRVs[0]);
	CompositeQuad->SetSRV("normalTexture", mGBufferSRVs[1]);

	CompositeQuad->OnRender(md3dImmediateContext, mWorld->GetPerFrameCBuffer(),"Composite");

	LPCSTR strVarNames[] = { "colorTexture", "normalTexture" };

	CompositeQuad->ClearSRVs(md3dImmediateContext, "Composite", strVarNames, 2);

}

