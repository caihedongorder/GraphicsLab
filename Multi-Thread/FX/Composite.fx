//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

#include "Common.fx"

cbuffer cbPerFrame: register(b0)
{
	float4x4 viewMat;
	float4x4 projMat;
};

cbuffer cbPerObject: register(b1)
{
	float4x4 modelMat;
};

struct VertexIn
{
	float2 UV  : TEXCOORD0;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float2 tex : TEXCOORD0;
};


/////////////
// GLOBALS //
/////////////
Texture2D colorTexture;
Texture2D normalTexture;
Texture2D depthTexture;




VertexOut VS(VertexIn vin)
{
    VertexOut output;
    
	// Store the texture coordinates for the pixel shader.
    output.tex = vin.UV;
	float2 screenPos = (vin.UV * 2.0f - 1.0f) * float2(1.0f,-1.0f) ;
	output.PosH = float4(screenPos,0.0f,1.0f);
    
	return output;
}


float4 PS(VertexOut pin) : SV_Target
{
	float3 normalValue = normalTexture.Sample(SampleTypePoint, pin.tex).rgb;
	float3 normals = normalValue * 2.0f - 1.0f;
	
	float4 outputColor;
	outputColor.rgb = normalValue;
	//outputColor.rgb = float3(0,0,0);
	outputColor.a = 1.0f;
	
	return outputColor;
}

technique11 BaseTech
{
    pass Composite
    {
		SetDepthStencilState(DSS_AwaysPass,0);
		SetRasterizerState(NoCullRS);
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
