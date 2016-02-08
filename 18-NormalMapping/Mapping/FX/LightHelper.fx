// ================================================= //
// LightHelper.fx
// ================================================= //

struct DirectionalLight {
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};

struct PointLight {
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 att;
    float pad;
};

struct SpotLight {
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 direction;
    float spot;

    float3 att;
    float pad;
};

struct Material {
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = specPower
    float4 reflect;
};

// ================================================= //

void ComputeDirectionalLight( Material mat,
                              DirectionalLight L,
                              float3 normal,
                              float3 toEye,
                              out float4 ambient,
                              out float4 diffuse,
                              out float4 spec )
{
    // Init outputs.
    ambient = float4( 0.f, 0.f, 0.f, 0.f );
    diffuse = float4( 0.f, 0.f, 0.f, 0.f );
    spec = float4( 0.f, 0.f, 0.f, 0.f );

    // Light vector aims opposite the direction the light rays travel.
    float3 lightVec = -L.direction;

    // Apply ambient.
    ambient = mat.ambient * L.ambient;

    // Apply diffuse/spec, provided surface is in line of sight with light.
    float diffuseFactor = dot( lightVec, normal );

    // Flatten to avoid dynamic branching.
    [flatten]
    if ( diffuseFactor > 0.f ) {
        // Get a reflection vector to calculate specular lighting.
        float3 v = reflect( -lightVec, normal );
        // If the cosine of the angle between the reflection vector and the toEye vector is positive,
        // a specular effect is generated, the intensity determine by the specular.w exponent.
        float specFactor = pow( max( dot( v, toEye ), 0.f ), mat.specular.w );

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }
}

// ================================================= //

void ComputePointLight( Material mat,
                        PointLight L,
                        float3 pos,
                        float3 normal,
                        float3 toEye,
                        out float4 ambient,
                        out float4 diffuse,
                        out float4 spec )
{
    // Init outputs.
    ambient = float4( 0.f, 0.f, 0.f, 0.f );
    diffuse = float4( 0.f, 0.f, 0.f, 0.f );
    spec = float4( 0.f, 0.f, 0.f, 0.f );

    // The vector from surface to the light.
    float3 lightVec = L.position - pos;

    // Distance from surface point to light.
    float d = length( lightVec );

    // Range test.
    if ( d > L.range )
        return;

    // Normalize light vector.
    lightVec /= d;

    // Apply ambient.
    ambient = mat.ambient * L.ambient;

    // Apply diffuse/spec.
    float diffuseFactor = dot( lightVec, normal );

    [flatten]
    if ( diffuseFactor > 0.f ) {
        float3 v = reflect( -lightVec, normal );
        float specFactor = pow( max( dot( v, toEye ), 0.f ), mat.specular.w );

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // Attenuate. The light's attenuation value is a float3, so the attenuation
    // parameters are controlled by changing its values.
    // e.g., L.att of (0.f, 1.f, 0.f) calculates attenuation by inverse distance.
    float att = 1.f / dot( L.att, float3( 1.f, d, d * d ) );    

    diffuse *= att;
    spec *= att;
}

// ================================================= //

void ComputeSpotLight( Material mat,
                       SpotLight L,
                       float3 pos,
                       float3 normal,
                       float3 toEye,
                       out float4 ambient,
                       out float4 diffuse,
                       out float4 spec )
{
    // Init outputs.
    ambient = float4( 0.0f, 0.0f, 0.0f, 0.0f );
    diffuse = float4( 0.0f, 0.0f, 0.0f, 0.0f );
    spec = float4( 0.0f, 0.0f, 0.0f, 0.0f );

    // Vector from surface to light.
    float3 lightVec = L.position - pos;

    float d = length( lightVec );

    if ( d > L.range )
        return;

    lightVec /= d;

    // Apply ambient.
    ambient = mat.ambient * L.ambient;

    // Apply diffuse/spec.
    float diffuseFactor = dot( lightVec, normal );

    [flatten]
    if ( diffuseFactor > 0.f ) {
        float3 v = reflect( -lightVec, normal );
        float specFactor = pow ( max( dot( v, toEye ), 0.f ), mat.specular.w );

        diffuse = diffuseFactor * mat.diffuse * L.diffuse;
        spec = specFactor * mat.specular * L.specular;
    }

    // Scale by spotlight factor and attenuate.
    float spot = pow( max( dot( -lightVec, L.direction ), 0.f ), L.spot );
    float att = spot / dot( L.att, float3( 1.f, d, d * d ) );

    ambient *= spot;
    diffuse *= att;
    spec *= att;
}

// ================================================= //

float3 NormalSampleToWorldSpace( float3 normalMapSample, float3 unitNormalW, float3 tangentW )
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f*normalMapSample - 1.0f;

    // Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize( tangentW - dot( tangentW, N )*N );
    float3 B = cross( N, T );

    float3x3 TBN = float3x3( T, B, N );

    // Transform from tangent space to world space.
    float3 bumpedNormalW = mul( normalT, TBN );

    return bumpedNormalW;
}

// ================================================= //
