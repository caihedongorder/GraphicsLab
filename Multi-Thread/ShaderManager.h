#pragma once
#include "d3dx11Effect.h"
#include <map>
#include <string>

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	ID3DX11Effect* GetShader(ID3D11Device* Ind3dDevice, const TCHAR* InShaderName);
	static ShaderManager* GetInstance();

private:
	static ShaderManager msInstance;

	std::map<std::wstring, ID3DX11Effect*> ShadersCache;
};

