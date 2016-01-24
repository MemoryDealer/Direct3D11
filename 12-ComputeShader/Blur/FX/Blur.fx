// ================================================= //
// Blur.fx
// ================================================= //

cbuffer cbSettings {
    float gWeights[11] = {
        0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f,
    };
};

cbuffer cbFixed {
    static const int gBlurRadius = 5;
};

// ================================================= //

Texture2D gInput;
RWTexture2D<float4> gOutput;

#define N 256
#define CacheSize (N + 2 * gBlurRadius )
groupshared float4 gCache[CacheSize]; // Shared memory for thread group.

// ================================================= //

[numthreads(N, 1, 1)]
void HorzBlurCS( int3 groupThreadID : SV_GroupThreadID,
                 int3 dispatchThreadID : SV_DispatchThreadID )
{
    //
    // Fill local thread storage to reduce bandwidth. To blur 
    // N pixels, we need to load N + 2 * blurRadius pixels.

    // This thread group runs N threads. To get the extra 2*blurRadius pixels,
    // have 2 * blurRadius threads sample an extra pixel.
    if ( groupThreadID.x < gBlurRadius ) {
        // Clamp out of bounds samples.
        int x = max( dispatchThreadID.x - gBlurRadius, 0 );
        gCache[groupThreadID.x] = gInput[int2( x, dispatchThreadID.y )];
    }
    if ( groupThreadID.x >= N - gBlurRadius ) {
        int x = min( dispatchThreadID.x + gBlurRadius, gInput.Length.x - 1 );
        gCache[groupThreadID.x + 2 * gBlurRadius] = gInput[int2( x, dispatchThreadID.y )];
    }

    // Clamp out of bounds sample that occur at image borders.
    gCache[groupThreadID.x + gBlurRadius] = gInput[min( dispatchThreadID.xy, gInput.Length.xy - 1 )];

    // Wait for all threads.
    GroupMemoryBarrierWithGroupSync();

    //
    // Blur each pixel.

    float4 blurColor = float4( 0.f, 0.f, 0.f, 0.f );

    [unroll]
    for ( int i = -gBlurRadius; i <= gBlurRadius; ++i ) {
        int k = groupThreadID.x + gBlurRadius + i;

        blurColor += gWeights[i + gBlurRadius] * gCache[k];
    }

    gOutput[dispatchThreadID.xy] = blurColor;
}

// ================================================= //

[numthreads( 1, N, 1 )]
void VertBlurCS( int3 groupThreadID : SV_GroupThreadID,
                 int3 dispatchThreadID : SV_DispatchThreadID )
{
    //
    // Fill local thread storage to reduce bandwidth.  To blur 
    // N pixels, we will need to load N + 2*BlurRadius pixels
    // due to the blur radius.
    //

    // This thread group runs N threads.  To get the extra 2*BlurRadius pixels, 
    // have 2*BlurRadius threads sample an extra pixel.
    if ( groupThreadID.y < gBlurRadius )
    {
        // Clamp out of bound samples that occur at image borders.
        int y = max( dispatchThreadID.y - gBlurRadius, 0 );
        gCache[groupThreadID.y] = gInput[int2( dispatchThreadID.x, y )];
    }
    if ( groupThreadID.y >= N - gBlurRadius )
    {
        // Clamp out of bound samples that occur at image borders.
        int y = min( dispatchThreadID.y + gBlurRadius, gInput.Length.y - 1 );
        gCache[groupThreadID.y + 2 * gBlurRadius] = gInput[int2( dispatchThreadID.x, y )];
    }

    // Clamp out of bound samples that occur at image borders.
    gCache[groupThreadID.y + gBlurRadius] = gInput[min( dispatchThreadID.xy, gInput.Length.xy - 1 )];


    // Wait for all threads to finish.
    GroupMemoryBarrierWithGroupSync();

    //
    // Now blur each pixel.
    //

    float4 blurColor = float4( 0, 0, 0, 0 );

    [unroll]
    for ( int i = -gBlurRadius; i <= gBlurRadius; ++i )
    {
        int k = groupThreadID.y + gBlurRadius + i;

        blurColor += gWeights[i + gBlurRadius] * gCache[k];
    }

    gOutput[dispatchThreadID.xy] = blurColor;
}

// ================================================= //

technique11 HorzBlur {
    pass P0 {
        SetVertexShader( NULL );
        SetPixelShader( NULL );
        SetComputeShader( CompileShader( cs_5_0, HorzBlurCS() ) );
    }
}

technique11 VertBlur {
    pass P0 {
        SetVertexShader( NULL );
        SetPixelShader( NULL );
        SetComputeShader( CompileShader( cs_5_0, VertBlurCS() ) );
    }
}

// ================================================= //
