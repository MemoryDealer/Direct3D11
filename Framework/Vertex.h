//***************************************************************************************
// Vertex.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines vertex structures and input layouts.
//***************************************************************************************

#ifndef VERTEX_H
#define VERTEX_H

#include "d3dUtil.h"

namespace Vertex
{
	struct Basic32
	{
		DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 tex;

        Basic32() : pos( 0.0f, 0.0f, 0.0f ), normal( 0.0f, 0.0f, 0.0f ), tex( 0.0f, 0.0f ) { }
        Basic32( const DirectX::XMFLOAT3& p, const DirectX::XMFLOAT3& n, const DirectX::XMFLOAT2& uv )
            : pos( p ), normal( n ), tex( uv )
        {}
        Basic32( float px, float py, float pz, float nx, float ny, float nz, float u, float v )
            : pos( px, py, pz ), normal( nx, ny, nz ), tex( u, v )
        {}
	};

    struct Pos {
        float x;
        float y;
        float z;
    };

    struct TreePointSprite {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT2 size;
    };
}

class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
    static const D3D11_INPUT_ELEMENT_DESC Pos[1];
    static const D3D11_INPUT_ELEMENT_DESC TreePointSprite[2];
    static const D3D11_INPUT_ELEMENT_DESC InstancedBasic32[8];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Basic32;
    static ID3D11InputLayout* Pos;
    static ID3D11InputLayout* TreePointSprite;
    static ID3D11InputLayout* InstancedBasic32;
};

#endif // VERTEX_H
