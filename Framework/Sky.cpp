//***************************************************************************************
// Sky.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Sky.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "Vertex.h"
#include "Effects.h"

using namespace DirectX;

Sky::Sky( ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius )
{    
    TexMetadata data;
    std::unique_ptr<ScratchImage> image( new ScratchImage() );
    HR( LoadFromDDSFile( cubemapFilename.c_str(), 
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( device,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mCubeMapSRV ) );

    GeometryGenerator::MeshData sphere;
    GeometryGenerator geoGen;
    geoGen.createSphere( skySphereRadius, 30, 30, sphere );

    std::vector<XMFLOAT3> vertices( sphere.vertices.size() );

    for ( size_t i = 0; i < sphere.vertices.size(); ++i )
    {
        vertices[i] = sphere.vertices[i].position;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( XMFLOAT3 ) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];

    HR( device->CreateBuffer( &vbd, &vinitData, &mVB ) );


    mIndexCount = sphere.indices.size();

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( USHORT ) * mIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;

    std::vector<USHORT> indices16;
    indices16.assign( sphere.indices.begin(), sphere.indices.end() );

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices16[0];

    HR( device->CreateBuffer( &ibd, &iinitData, &mIB ) );
}

Sky::~Sky()
{
    ReleaseCOM( mVB );
    ReleaseCOM( mIB );
    ReleaseCOM( mCubeMapSRV );
}

ID3D11ShaderResourceView* Sky::CubeMapSRV()
{
    return mCubeMapSRV;
}

void Sky::Draw( ID3D11DeviceContext* dc, const Camera& camera )
{
    // center Sky about eye in world space
    XMFLOAT3 eyePos = camera.GetPosition();
    XMMATRIX T = XMMatrixTranslation( eyePos.x, eyePos.y, eyePos.z );


    XMMATRIX WVP = XMMatrixMultiply( T, camera.ViewProj() );

    Effects::SkyFX->SetWorldViewProj( WVP );
    Effects::SkyFX->SetCubeMap( mCubeMapSRV );


    UINT stride = sizeof( XMFLOAT3 );
    UINT offset = 0;
    dc->IASetVertexBuffers( 0, 1, &mVB, &stride, &offset );
    dc->IASetIndexBuffer( mIB, DXGI_FORMAT_R16_UINT, 0 );
    dc->IASetInputLayout( InputLayouts::Pos );
    dc->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    D3DX11_TECHNIQUE_DESC techDesc;
    Effects::SkyFX->SkyTech->GetDesc( &techDesc );

    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        ID3DX11EffectPass* pass = Effects::SkyFX->SkyTech->GetPassByIndex( p );

        pass->Apply( 0, dc );

        dc->DrawIndexed( mIndexCount, 0, 0 );
    }
}


