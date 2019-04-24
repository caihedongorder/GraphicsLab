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
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

struct PixelOutputType
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
};

VertexOut DeferedBasePassVS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(modelMat,viewMat),projMat);
	vout.PosH = mul(float4(vin.PosL, 1.0f), mvp);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

PixelOutputType DeferedBasePassPS(VertexOut pin)
{
	PixelOutputType output;
    output.color = pin.Color;
	output.normal = pin.Color;
	
	return output;
}


VertexOut ForwardBasePassVS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(modelMat,viewMat),projMat);
	vout.PosH = mul(float4(vin.PosL, 1.0f), mvp);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 ForwardBasePassPS(VertexOut pin):SV_Target
{
	return pin.Color;
}

technique11 BaseTech
{
    pass DeferedBasePass
    {
		SetDepthStencilState(DSS_Less,0);
		SetRasterizerState(CullRS);
		
        SetVertexShader( CompileShader( vs_5_0, DeferedBasePassVS() ) );
        SetPixelShader( CompileShader( ps_5_0, DeferedBasePassPS() ) );
    }
	pass ForwardBasePass
    {
		SetDepthStencilState(DSS_Less,0);
		SetRasterizerState(CullRS);
		
        SetVertexShader( CompileShader( vs_5_0, ForwardBasePassVS() ) );
        SetPixelShader( CompileShader( ps_5_0, ForwardBasePassPS() ) );
    }
}
