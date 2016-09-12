#pragma pack_matrix( row_major )

Texture2D gTexture;

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

	vout.PosH = mul( float4( vin.PosL, 1.0f ), gWorldViewProj );
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS( VertexOut pin ) : SV_Target
{
	return gTexture.Sample( samLinear, pin.Tex );
}