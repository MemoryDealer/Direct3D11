// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file D3DUtil.h
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#pragma once

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#if defined( DEBUG ) || defined( _DEBUG )
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <Windows.h>

#include <D3D11.h>
#include <DXGI.h>
#include <dxgiformat.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h> // HALF
#include <DxErr.h>
#include <d3dx11effect.h>
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DDSTextureLoader/DDSTextureLoader.h>

#include "LightHelper.h"

#include <cassert>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <vector>

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

// Error checker.
#if defined( DEBUG ) || defined( _DEBUG )
#ifndef HR
#define HR( x )\
{\
HRESULT hr = ( x );\
if( FAILED( hr ) ){\
std::cout << "Fatal error: " << __FILE__ << ", line " << __LINE__;\
DXTrace( __FILEW__, static_cast<DWORD>( __LINE__ ), hr, L"Unknown", true );\
}\
}
#endif

#else
#ifndef HR
#define HR( x ) ( x )
#endif
#endif

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#define ReleaseCOM( x ) { if( x ){ x->Release(); x = nullptr; } }
#define SafeDelete( x ) { delete x; x = nullptr; }

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

class TextHelper {
public:

    template<typename T>
    static inline std::wstring ToString( const T& s )
    {
        std::wostringstream oss;
        oss << s;

        return oss.str();
    }

    template<typename T>
    static inline T FromString( const std::wstring& s )
    {
        T x;
        std::wistringstream iss( s );
        iss >> x;

        return x;
    }
};

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

static void ExtractFrustumPlanes( DirectX::XMFLOAT4 planes[6], DirectX::CXMMATRIX M )
{
    //
    // Left
    //
    planes[0].x = M( 0, 3 ) + M( 0, 0 );
    planes[0].y = M( 1, 3 ) + M( 1, 0 );
    planes[0].z = M( 2, 3 ) + M( 2, 0 );
    planes[0].w = M( 3, 3 ) + M( 3, 0 );

    //
    // Right
    //
    planes[1].x = M( 0, 3 ) - M( 0, 0 );
    planes[1].y = M( 1, 3 ) - M( 1, 0 );
    planes[1].z = M( 2, 3 ) - M( 2, 0 );
    planes[1].w = M( 3, 3 ) - M( 3, 0 );

    //
    // Bottom
    //
    planes[2].x = M( 0, 3 ) + M( 0, 1 );
    planes[2].y = M( 1, 3 ) + M( 1, 1 );
    planes[2].z = M( 2, 3 ) + M( 2, 1 );
    planes[2].w = M( 3, 3 ) + M( 3, 1 );

    //
    // Top
    //
    planes[3].x = M( 0, 3 ) - M( 0, 1 );
    planes[3].y = M( 1, 3 ) - M( 1, 1 );
    planes[3].z = M( 2, 3 ) - M( 2, 1 );
    planes[3].w = M( 3, 3 ) - M( 3, 1 );

    //
    // Near
    //
    planes[4].x = M( 0, 2 );
    planes[4].y = M( 1, 2 );
    planes[4].z = M( 2, 2 );
    planes[4].w = M( 3, 2 );

    //
    // Far
    //
    planes[5].x = M( 0, 3 ) - M( 0, 2 );
    planes[5].y = M( 1, 3 ) - M( 1, 2 );
    planes[5].z = M( 2, 3 ) - M( 2, 2 );
    planes[5].w = M( 3, 3 ) - M( 3, 2 );

    // Normalize the plane equations.
    for ( int i = 0; i < 6; ++i )
    {
        DirectX::XMVECTOR v = DirectX::XMPlaneNormalize( DirectX::XMLoadFloat4( &planes[i] ) );
        DirectX::XMStoreFloat4( &planes[i], v );
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

namespace Colors {

    XMGLOBALCONST DirectX::XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

    XMGLOBALCONST DirectX::XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
    XMGLOBALCONST DirectX::XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };

}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //