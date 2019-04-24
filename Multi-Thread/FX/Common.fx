

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