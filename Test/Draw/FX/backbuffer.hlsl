#pragma pack_matrix( row_major )

Texture2D gTexture : register( t0 );

cbuffer cbPerObject : register( b0 )
{
	float4x4 gWorldViewProj;
};

SamplerState samLinear {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn {
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut {
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	matrix Identity =
{
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

	vout.PosH = mul( float4( vin.PosL, 1.0f ), Identity );
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS( VertexOut pin ) : SV_Target
{
	float4 pixel = gTexture.Sample( samLinear, pin.Tex );

	// B/W
	/*float gs = dot( pixel.rgb, float3( 0.3, 0.59, 0.11 ) );
	pixel.r = gs;
	pixel.g = gs;
	pixel.b = gs;*/

	// Sepia
	/*pixel.r = ( pixel.r * .393 ) + ( pixel.g * .769 ) + ( pixel.b * .189 );
	pixel.g = ( pixel.r * .349 ) + ( pixel.g * .686 ) + ( pixel.b * .168 );
	pixel.b = ( pixel.r * .272 ) + ( pixel.g * .534 ) + ( pixel.b * .131 );*/

	// Night vision
	/*float c = dot( pixel, float4( 0.2, 0.6, 0.1, 0.1 ) );
	return float4( 0.0, c, 0.0, 0.0 );*/

	return pixel;
}