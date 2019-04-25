

RasterizerState WireframeRS
{
    FillMode = Wireframe;
    CullMode = Back;
    FrontCounterClockwise = false;
    AntialiasedLineEnable = true;
};

RasterizerState NoCullRS
{
    FillMode = Solid;
    CullMode = None;
    FrontCounterClockwise = false;
    AntialiasedLineEnable = true;
};

RasterizerState CullRS
{
    FillMode = Solid;
    CullMode = BACK;
    FrontCounterClockwise = false;
    AntialiasedLineEnable = true;
};

///////////////////
// SAMPLE STATES //
///////////////////

SamplerState SampleTypePoint
{
	Filter = MIN_MAG_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};


///////////////////
// DS STATES //
///////////////////
DepthStencilState DSS_AwaysPass
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
	DepthFunc = ALWAYS;
};

DepthStencilState DSS_Less
{
	DepthEnable = TRUE;
	DepthWriteMask = All;
	DepthFunc = LESS;
};


float3 EncodeNormal(float3 InNormalValue){
    return (InNormalValue + 1.0f)*0.5f;
}

float3 DecodeNormal(float3 InNormalRGB){
    return InNormalRGB * 2.0f - 1.0f;
}

float3x3 MakeUITransform(float2 InTranslate,float2 InScale,float InRotateAngle){

    //float3x3 ToLocal
    float cosTheta = cos(InRotateAngle);
    float sinTheta = sin(InRotateAngle);
    float3x3 transform = {
        cosTheta * InScale.x,   -sinTheta * InScale.x, 0,
        sinTheta * InScale.y,   cosTheta * InScale.y,  0,
        InTranslate.x,          InTranslate.y,          1
    };

    return transpose(transform);
}