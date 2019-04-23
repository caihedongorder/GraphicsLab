#include "stdafx.h"
#include "ShaderManager.h"
#include <d3dx11effect.h>
#include <d3dx11async.h>
#include "GraphSystem.h"


ShaderManager ShaderManager::msInstance;

ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
	for (auto it = ShadersCache.begin() ; it != ShadersCache.end() ; ++it)
	{
		ReleaseCOM(it->second);
	}
	ShadersCache.clear();
}

ID3DX11Effect* ShaderManager::GetShader(ID3D11Device* Ind3dDevice, const TCHAR* InShaderName)
{
	ID3DX11Effect* Effect = nullptr;

	auto it = ShadersCache.find(InShaderName);

	if (it == ShadersCache.end())
	{
		DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3D10_SHADER_DEBUG;
		shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

		ID3D10Blob* compiledShader = 0;
		ID3D10Blob* compilationMsgs = 0;
		HRESULT hr = D3DX11CompileFromFile(InShaderName, 0, 0, 0, "fx_5_0", shaderFlags,
			0, 0, &compiledShader, &compilationMsgs, 0);

		// compilationMsgs can store errors or warnings.
		if (compilationMsgs != 0)
		{
			MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
			ReleaseCOM(compilationMsgs);
		}

		// Even if there are no compilationMsgs, check to make sure there were no other errors.
		if (FAILED(hr))
		{
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX11CompileFromFile", true);
		}

		HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
			0, Ind3dDevice, &Effect));

		ShadersCache[InShaderName] = Effect;
	}
	else
	{
		Effect = it->second;
	}

	return Effect;
}

ShaderManager* ShaderManager::GetInstance()
{
	return &msInstance;
}
