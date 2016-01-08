//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
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

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Change color of terrain based on height. Just an exercise for GPU stuff.
    if ( vin.PosL.y < -10.f ) {
        vout.Color = float4( 1.f, 0.96f, 0.62f, 1.f );
    }
    else if ( vin.PosL.y < 5.f ) {
        vout.Color = float4( 0.48f, 0.77f, 0.46f, 1.f );
    }
    else if ( vin.PosL.y < 12.f ) {
        vout.Color = float4( 0.1f, 0.48f, 0.19f, 1.f );
    }
    else if ( vin.PosL.y < 20.f ) {
        vout.Color = float4( 0.45f, 0.39f, 0.34f, 1.f );
    }
    else {
        vout.Color = float4( 1.f, 1.f, 1.f, 1.f );
    }
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
