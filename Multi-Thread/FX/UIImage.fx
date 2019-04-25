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
	float4 ClipRect         :CLIP;			//当前控件裁剪矩形
	float4 TranslateAndScale :TS;			//xy : translate zw : Scale
	float4 CanvasSizeAndWidgetSize :CW;		//xy : FrameSize zw : WidgetSize
	float4 LocationAndAnchor :LA;			//xy :  Location zw : Anchor
	float RotateAngle :ROTATE;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 ClipRect         :CLIP;
    float2 CanvasUV : TEXCOORD0;
};




VertexOut VS(VertexIn vin)
{
    VertexOut output;
    
	// Store the texture coordinates for the pixel shader.
	float2 VertexLocation = vin.LocationAndAnchor.xy;
	float3x3 tansformMat = MakeUITransform(vin.TranslateAndScale.xy,vin.TranslateAndScale.zw,vin.RotateAngle);
	float2 transformedLocation = mul(tansformMat,float3(VertexLocation,1)).xy;
	//transformedLocation = VertexLocation;
	float2 UV = transformedLocation / vin.CanvasSizeAndWidgetSize.xy;

    output.CanvasUV = UV;

	float2 screenPos = (UV * 2.0f - 1.0f) * float2(1.0f,-1.0f) ;
	output.PosH = float4(screenPos,0.0f,1.0f);

	output.ClipRect.xy = vin.ClipRect.xy / vin.CanvasSizeAndWidgetSize.xy;
	output.ClipRect.zw = vin.ClipRect.zw / vin.CanvasSizeAndWidgetSize.xy;
    
	return output;
}


float4 PS(VertexOut pin) : SV_Target
{
	float4 outputColor = float4(1,0,0,1);
	if(pin.CanvasUV.x < pin.ClipRect.x ||
	  pin.CanvasUV.x > pin.ClipRect.z ||
	  pin.CanvasUV.y < pin.ClipRect.y ||
	  pin.CanvasUV.y > pin.ClipRect.w)
		discard;
	return outputColor;

}

technique11 BaseTech
{
    pass UI
    {
		SetDepthStencilState(DSS_AwaysPass,0);
		SetRasterizerState(NoCullRS);
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
