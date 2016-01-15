// ================================================= //
// Lighting.fx
// ================================================= //

#include "LightHelper.fx"

// ================================================= //

cbuffer cbPerFrame {
    DirectionalLight gDirLight;
    PointLight gPointLight;
    SpotLight gSpotLight;
    float3 gEyePosW;
};

cbuffer cbPerObject {
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    Material gMaterial;
};

// ================================================= //

struct VertexIn {
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
};

struct VertexOut {
    float4 PosH : SV_POSITION; // "System value" - denotes the vertex shader output element that holds the vertex position.
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
};

// ================================================= //

VertexOut VS( VertexIn vin )
{
    VertexOut vout;

    // Transform to world space.
    vout.PosW = mul( float4( vin.PosL, 1.f ), gWorld ).xyz; // Only pull out xyz components for PosW since it's a float3.
    // (See p.276) A non-uniform scaling factor distorts the normal, applying the inverse tranpose world matrix
    // transforms the normal so it's orthogonal to its associated tangent vector.
    vout.NormalW = mul( vin.NormalL, ( float3x3 )gWorldInvTranspose );

    // Transform to homogeneous clip space.
    vout.PosH = mul( float4( vin.PosL, 1.f ), gWorldViewProj );

    return vout;
}

// ================================================= //

float4 PS( VertexOut pin ) : SV_Target // SV_Target indicates the PS return value type should match the render target format.
{
    // Interpolating normal (happens during rasterization) can unnormalize it, so normalize it.
    pin.NormalW = normalize( pin.NormalW );

    float3 toEyeW = normalize( gEyePosW - pin.PosW );

    // Begin with zero.
    float4 ambient = float4( 0.0f, 0.0f, 0.0f, 0.0f );
    float4 diffuse = float4( 0.0f, 0.0f, 0.0f, 0.0f );
    float4 spec = float4( 0.0f, 0.0f, 0.0f, 0.0f );

    // Sum the light contribution from each light source.
    float4 A, D, S;

    ComputeDirectionalLight( gMaterial,
                             gDirLight,
                             pin.NormalW,
                             toEyeW,
                             A,
                             D,
                             S );
    ambient += A;
    diffuse += D;
    spec += S;

    ComputePointLight( gMaterial,
                       gPointLight,
                       pin.PosW,
                       pin.NormalW,
                       toEyeW,
                       A,
                       D,
                       S );
    ambient += A;
    diffuse += D;
    spec += S;

    ComputeSpotLight( gMaterial,
                      gSpotLight,
                      pin.PosW,
                      pin.NormalW,
                      toEyeW,
                      A,
                      D,
                      S );
    ambient += A;
    diffuse += D;
    spec += S;

    float4 litColor = ambient + diffuse + spec;

    // Common to take alpha from diffuse material.
    litColor.a = gMaterial.diffuse.a;

    return litColor;
}

// ================================================= //

technique11 LightTech {
    pass P0 {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

// ================================================= //
