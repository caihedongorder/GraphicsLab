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
	float4x4 modelMatInvT;
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float3 Normal : NORMAL;
	float2 UV : TEXCOORD0;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float3 Normal : NORMAL;
	float2 UV : TEXCOORD0;
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
	vout.Normal = mul(modelMatInvT,float4(vin.Normal,0)).xyz;
	vout.UV = vin.UV;
    
    return vout;
}

PixelOutputType DeferedBasePassPS(VertexOut pin)
{
	PixelOutputType output;
	float3 normalValue = EncodeNormal(pin.Normal);
	//normalValue = float3(1,0,0);
    output.color = float4(normalValue,1);
	output.normal = float4(normalValue,1);
	
	return output;
}


VertexOut ForwardBasePassVS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(modelMat,viewMat),projMat);
	vout.PosH = mul(float4(vin.PosL, 1.0f), mvp);
	
	// Just pass vertex color into the pixel shader.
	vout.Normal = mul(modelMatInvT,float4(vin.Normal,0)).xyz;
	vout.UV = vin.UV;
    
    return vout;
}

float4 ForwardBasePassPS(VertexOut pin):SV_Target
{
	return float4(pin.Normal,1);
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
