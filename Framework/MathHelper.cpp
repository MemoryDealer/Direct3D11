//***************************************************************************************
// MathHelper.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "MathHelper.h"
#include <float.h>
#include <cmath>

const float MathHelper::Infinity = FLT_MAX;
const float MathHelper::Pi = 3.1415926535f;

float MathHelper::AngleFromXY( float x, float y )
{
    float theta = 0.0f;

    // Quadrant I or IV
    if ( x >= 0.0f )
    {
        // If x = 0, then atanf(y/x) = +pi/2 if y > 0
        //                atanf(y/x) = -pi/2 if y < 0
        theta = atanf( y / x ); // in [-pi/2, +pi/2]

        if ( theta < 0.0f )
            theta += 2.0f*Pi; // in [0, 2*pi).
    }

    // Quadrant II or III
    else
        theta = atanf( y / x ) + Pi; // in [0, 2*pi).

    return theta;
}

DirectX::XMVECTOR MathHelper::RandUnitVec3()
{
    DirectX::XMVECTOR One = DirectX::XMVectorSet( 1.0f, 1.0f, 1.0f, 1.0f );
    DirectX::XMVECTOR Zero = DirectX::XMVectorZero();

    // Keep trying until we get a point on/in the hemisphere.
    while ( true )
    {
        // Generate random point in the cube [-1,1]^3.
        DirectX::XMVECTOR v = DirectX::XMVectorSet( MathHelper::RandF( -1.0f, 1.0f ), MathHelper::RandF( -1.0f, 1.0f ), MathHelper::RandF( -1.0f, 1.0f ), 0.0f );

        // Ignore points outside the unit sphere in order to get an even distribution 
        // over the unit sphere.  Otherwise points will clump more on the sphere near 
        // the corners of the cube.

        if ( DirectX::XMVector3Greater( DirectX::XMVector3LengthSq( v ), One ) )
            continue;

        return DirectX::XMVector3Normalize( v );
    }
}

DirectX::XMVECTOR MathHelper::RandHemisphereUnitVec3( DirectX::XMVECTOR n )
{
    DirectX::XMVECTOR One = DirectX::XMVectorSet( 1.0f, 1.0f, 1.0f, 1.0f );
    DirectX::XMVECTOR Zero = DirectX::XMVectorZero();

    // Keep trying until we get a point on/in the hemisphere.
    while ( true )
    {
        // Generate random point in the cube [-1,1]^3.
        DirectX::XMVECTOR v = DirectX::XMVectorSet( MathHelper::RandF( -1.0f, 1.0f ), MathHelper::RandF( -1.0f, 1.0f ), MathHelper::RandF( -1.0f, 1.0f ), 0.0f );

        // Ignore points outside the unit sphere in order to get an even distribution 
        // over the unit sphere.  Otherwise points will clump more on the sphere near 
        // the corners of the cube.

        if ( DirectX::XMVector3Greater( DirectX::XMVector3LengthSq( v ), One ) )
            continue;

        // Ignore points in the bottom hemisphere.
        if ( DirectX::XMVector3Less( DirectX::XMVector3Dot( n, v ), Zero ) )
            continue;

        return DirectX::XMVector3Normalize( v );
    }
}