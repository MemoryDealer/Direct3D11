// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Vertex.h"

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

using namespace DirectX;

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

class App : public D3DApp {

public:

    App( HINSTANCE hInst );
    virtual ~App( void ) override;

    virtual bool init( void ) override;
    virtual void onResize( void ) override;
    virtual void updateScene( const float dt ) override;
    virtual void drawScene( void ) override;

    virtual void onMouseDown( WPARAM btnState, int x, int y ) override;
    virtual void onMouseUp( WPARAM btnState, int x, int y ) override;
    virtual void onMouseMove( WPARAM btnState, int x, int y ) override;

private:

    void buildGeometryBuffers( void );

private:

    ID3D11Buffer* mBoxVB;
    ID3D11Buffer* mBoxIB;

    ID3D11ShaderResourceView* mDiffuseMapSRV;

    DirectionalLight mDirLights[3];
    Material mBoxMat;

    XMFLOAT4X4 mTexTransform;
    XMFLOAT4X4 mBoxWorld;

    XMFLOAT4X4 mView, mProj;

    int mBoxVertexOffset;
    UINT mBoxIndexOffset;
    UINT mBoxIndexCount;

    UINT mLightCount;

    XMFLOAT3 mEyePosW;

    float mTheta, mPhi, mRadius;

    POINT mLastMousePos;

};

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE prevInstance,
                    PSTR cmdLine, int showCmd )
{
#if defined( DEBUG ) || defined( _DEBUG )
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    App app( hInstance );

    if ( !app.init() ) {
        return 0;
    }

    return app.run();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::App( HINSTANCE hInstance )
    : D3DApp( hInstance )
    , mBoxVB( nullptr )
    , mBoxIB( nullptr )
    , mDiffuseMapSRV( nullptr )
    , mLightCount( 1 )
    , mEyePosW( 0.f, 0.f, 0.f )
    , mTheta( 1.5f * MathHelper::Pi )
    , mPhi( 0.1f * MathHelper::Pi )
    , mRadius( 15.0f )
{
    mMainWindowCaption = L"Crate Demo";

    ZeroMemory( &mLastMousePos, sizeof( POINT ) );

    CXMMATRIX I = DirectX::XMMatrixIdentity();
    XMStoreFloat4x4( &mBoxWorld, I );
    XMStoreFloat4x4( &mTexTransform, I );
    XMStoreFloat4x4( &mView, I );
    XMStoreFloat4x4( &mProj, I );

    mBoxMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mBoxMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.specular = XMFLOAT4( 0.6f, 0.6f, 0.6f, 16.0f );

    mDirLights[0].ambient = XMFLOAT4( 0.3f, 0.3f, 0.3f, 1.0f );
    mDirLights[0].diffuse = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mDirLights[0].specular = XMFLOAT4( 0.6f, 0.6f, 0.6f, 16.0f );
    mDirLights[0].direction = XMFLOAT3( 0.707f, -0.707f, 0.0f );

    mDirLights[1].ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[1].diffuse = XMFLOAT4( 1.4f, 1.4f, 1.4f, 1.0f );
    mDirLights[1].specular = XMFLOAT4( 0.3f, 0.3f, 0.3f, 16.0f );
    mDirLights[1].direction = XMFLOAT3( -0.707f, 0.0f, 0.707f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    ReleaseCOM( mBoxVB );
    ReleaseCOM( mBoxIB );   
    ReleaseCOM( mDiffuseMapSRV );

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() ) {
        return false;
    }

    Effects::InitAll( mD3DDevice );
    InputLayouts::InitAll( mD3DDevice );

    //HR (D3DX11CreateShader)
    TexMetadata data;
    std::unique_ptr<ScratchImage> image( new ScratchImage() );
    HR( LoadFromDDSFile( L"Textures/WoodCrate01.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mDiffuseMapSRV ) );

    buildGeometryBuffers();

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onResize( void )
{
    D3DApp::onResize();

    // Update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH( 0.25f * MathHelper::Pi,
                                           getAspectRatio(),
                                           1.f,
                                           1000.f );
    XMStoreFloat4x4( &mProj, P );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::updateScene( const float dt )
{
    // Convert spherical to cartesian.
    float x = mRadius * sinf( mPhi ) * cosf( mTheta );
    float y = mRadius * sinf( mPhi ) * sinf( mTheta );
    float z = mRadius * cosf( mPhi );

    mEyePosW = XMFLOAT3( x, y, z );

    // Build the view matrix.
    DirectX::XMVECTOR pos = DirectX::XMVectorSet( x, y, z, 1.f );
    DirectX::XMVECTOR target = DirectX::XMVectorZero();
    DirectX::XMVECTOR up = DirectX::XMVectorSet( 0.f, 1.f, 0.f, 0.f );

    DirectX::XMMATRIX V = DirectX::XMMatrixLookAtLH( pos, target, up );
    XMStoreFloat4x4( &mView, V );    

    /*static XMFLOAT2 offset { 0.f, 0.f };
    offset.x += 0.05f * dt;
    offset.y += 0.1f * dt;
    XMStoreFloat4x4( &mTexTransform, XMMatrixTranslation( offset.x, offset.y, 0.f ) );*/
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView,
                                                 reinterpret_cast<const float*>( &Colors::LightSteelBlue ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView,
                                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                 1.f,
                                                 0 );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Set constant buffers.
    XMMATRIX view = XMLoadFloat4x4( &mView );
    XMMATRIX proj = XMLoadFloat4x4( &mProj );
    XMMATRIX viewProj = view * proj;

    // Set per-frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mEyePosW );

    // Figure out which tech to use.
    ID3DX11EffectTechnique* tech = Effects::BasicFX->Light2TexTech;
    

    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );
    const UINT stride = sizeof( Vertex::Basic32 );
    const UINT offset = 0;
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        // Bind shape VBs.
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mBoxVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mBoxIB, DXGI_FORMAT_R32_UINT, 0 );

        // Draw box.
        XMMATRIX world = XMLoadFloat4x4( &mBoxWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMLoadFloat4x4( &mTexTransform ) );
        Effects::BasicFX->SetMaterial( mBoxMat );
        Effects::BasicFX->SetDiffuseMap( mDiffuseMapSRV );

        tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset );
    }

    HR( mSwapChain->Present( 0, 0 ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onMouseDown( WPARAM btnState, int x, int y )
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture( mMainWindow );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onMouseUp( WPARAM btnState, int x, int y )
{
    ReleaseCapture();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onMouseMove( WPARAM btnState, int x, int y )
{
    if ( ( btnState & MK_LBUTTON ) != 0 ) {
        // Correspond each pixel to a quarter of a degree.
        float dx = XMConvertToRadians( 0.25f * static_cast<float>( x - mLastMousePos.x ) );
        float dy = XMConvertToRadians( 0.25f * static_cast<float>( y - mLastMousePos.y ) );

        // Update spherical coordinates for angles.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle phi.
        mPhi = MathHelper::Clamp( mPhi, 0.1f, MathHelper::Pi - 0.1f );
    }
    else if ( ( btnState & MK_RBUTTON ) != 0 ) {
        // Correspond each pixel to 0.005 units in the scene.
        float dx = 0.01f * static_cast<float>( x - mLastMousePos.x );
        float dy = 0.01f * static_cast<float>( y - mLastMousePos.y );

        // Update camera radius.
        mRadius += dx - dy;

        // Restrict radius.
        mRadius = MathHelper::Clamp( mRadius, 3.f, 200.f );
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildGeometryBuffers( void )
{
    GeometryGenerator::MeshData box;

    GeometryGenerator geoGen;
    geoGen.createBox( 1.0f, 1.0f, 1.0f, box );

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;

    // Cache the index count of each object.
    mBoxIndexCount = box.indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;

    UINT totalVertexCount = box.vertices.size();

    UINT totalIndexCount = mBoxIndexCount;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<Vertex::Basic32> vertices( totalVertexCount );

    UINT k = 0;
    for ( size_t i = 0; i < box.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = box.vertices[i].position;
        vertices[k].normal = box.vertices[i].normal;
        vertices[k].tex = box.vertices[i].texC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mBoxVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    std::vector<UINT> indices;
    indices.insert( indices.end(), box.indices.begin(), box.indices.end() );

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mBoxIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //