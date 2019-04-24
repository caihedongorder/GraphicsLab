

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