//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

RasterizerState WireframeRS
{
    FillMode = Wireframe;
    CullMode = Back;
    FrontCounterClockwise = false;
    AntialiasedLineEnable = true;
};


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
	float2 PosL  : TEXCOORD0;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	float4x4 mvp = mul(mul(modelMat,viewMat),projMat);
	vout.PosH = mul(float4(vin.PosL,1.0f, 1.0f), mvp);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = float4(1,0,0,0);
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return float4(1,0,0,0);
}

technique11 ColorTech
{
    pass BasePass
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
