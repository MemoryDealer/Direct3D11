// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "Camera.h"
#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "DirectXCollision.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "RenderStates.h"
#include "Sky.h"
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

    void BuildShapeGeometryBuffers();
    void BuildSkullGeometryBuffers();

private:

    Sky* mSky;

    ID3D11Buffer* mShapesVB;
    ID3D11Buffer* mShapesIB;

    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;

    ID3D11Buffer* mSkySphereVB;
    ID3D11Buffer* mSkySphereIB;

    ID3D11ShaderResourceView* mFloorTexSRV;
    ID3D11ShaderResourceView* mStoneTexSRV;
    ID3D11ShaderResourceView* mBrickTexSRV;

    DirectionalLight mDirLights[3];
    Material mGridMat;
    Material mBoxMat;
    Material mCylinderMat;
    Material mSphereMat;
    Material mSkullMat;

    // Define transformations from local spaces to world space.
    XMFLOAT4X4 mSphereWorld[10];
    XMFLOAT4X4 mCylWorld[10];
    XMFLOAT4X4 mBoxWorld;
    XMFLOAT4X4 mGridWorld;
    XMFLOAT4X4 mSkullWorld;

    int mBoxVertexOffset;
    int mGridVertexOffset;
    int mSphereVertexOffset;
    int mCylinderVertexOffset;

    UINT mBoxIndexOffset;
    UINT mGridIndexOffset;
    UINT mSphereIndexOffset;
    UINT mCylinderIndexOffset;

    UINT mBoxIndexCount;
    UINT mGridIndexCount;
    UINT mSphereIndexCount;
    UINT mCylinderIndexCount;

    UINT mSkullIndexCount;

    UINT mLightCount;

    Camera mCam;

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
    : D3DApp( hInstance ), mSky( 0 ),
    mShapesVB( 0 ), mShapesIB( 0 ), mSkullVB( 0 ), mSkullIB( 0 ),
    mFloorTexSRV( 0 ), mStoneTexSRV( 0 ), mBrickTexSRV( 0 ),
    mSkullIndexCount( 0 ), mLightCount( 3 )
{
    mMainWindowCaption = L"Cube Mapping Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition( 0.0f, 2.0f, -15.0f );

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4( &mGridWorld, I );

    XMMATRIX boxScale = XMMatrixScaling( 3.0f, 1.0f, 3.0f );
    XMMATRIX boxOffset = XMMatrixTranslation( 0.0f, 0.5f, 0.0f );
    XMStoreFloat4x4( &mBoxWorld, XMMatrixMultiply( boxScale, boxOffset ) );

    XMMATRIX skullScale = XMMatrixScaling( 0.5f, 0.5f, 0.5f );
    XMMATRIX skullOffset = XMMatrixTranslation( 0.0f, 1.0f, 0.0f );
    XMStoreFloat4x4( &mSkullWorld, XMMatrixMultiply( skullScale, skullOffset ) );

    for ( int i = 0; i < 5; ++i )
    {
        XMStoreFloat4x4( &mCylWorld[i * 2 + 0], XMMatrixTranslation( -5.0f, 1.5f, -10.0f + i*5.0f ) );
        XMStoreFloat4x4( &mCylWorld[i * 2 + 1], XMMatrixTranslation( +5.0f, 1.5f, -10.0f + i*5.0f ) );

        XMStoreFloat4x4( &mSphereWorld[i * 2 + 0], XMMatrixTranslation( -5.0f, 3.5f, -10.0f + i*5.0f ) );
        XMStoreFloat4x4( &mSphereWorld[i * 2 + 1], XMMatrixTranslation( +5.0f, 3.5f, -10.0f + i*5.0f ) );
    }

    mDirLights[0].ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[0].diffuse = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mDirLights[0].specular = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mDirLights[0].direction = XMFLOAT3( 0.57735f, -0.57735f, 0.57735f );

    mDirLights[1].ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mDirLights[1].diffuse = XMFLOAT4( 0.20f, 0.20f, 0.20f, 1.0f );
    mDirLights[1].specular = XMFLOAT4( 0.25f, 0.25f, 0.25f, 1.0f );
    mDirLights[1].direction = XMFLOAT3( -0.57735f, -0.57735f, 0.57735f );

    mDirLights[2].ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mDirLights[2].diffuse = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[2].specular = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mDirLights[2].direction = XMFLOAT3( 0.0f, -0.707f, -0.707f );

    mGridMat.ambient = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mGridMat.diffuse = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mGridMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mGridMat.reflect = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );

    mCylinderMat.ambient = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mCylinderMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mCylinderMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mCylinderMat.reflect = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );

    mSphereMat.ambient = XMFLOAT4( 0.2f, 0.3f, 0.4f, 1.0f );
    mSphereMat.diffuse = XMFLOAT4( 0.2f, 0.3f, 0.4f, 1.0f );
    mSphereMat.specular = XMFLOAT4( 0.9f, 0.9f, 0.9f, 16.0f );
    mSphereMat.reflect = XMFLOAT4( 0.4f, 0.4f, 0.4f, 1.0f );

    mBoxMat.ambient = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mBoxMat.reflect = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );

    mSkullMat.ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mSkullMat.diffuse = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mSkullMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mSkullMat.reflect = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    SafeDelete( mSky );

    ReleaseCOM( mShapesVB );
    ReleaseCOM( mShapesIB );
    ReleaseCOM( mSkullVB );
    ReleaseCOM( mSkullIB );
    ReleaseCOM( mFloorTexSRV );
    ReleaseCOM( mStoneTexSRV );
    ReleaseCOM( mBrickTexSRV );

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() )
        return false;

    Effects::InitAll( mD3DDevice );
    InputLayouts::InitAll( mD3DDevice );

    mSky = new Sky( mD3DDevice, L"Textures/grasscube1024.dds", 5000.0f );

    TexMetadata data;
    std::unique_ptr<ScratchImage> image( new ScratchImage() );
    HR( LoadFromDDSFile( L"Textures/floor.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mFloorTexSRV ) );

    HR( LoadFromDDSFile( L"Textures/stone.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mStoneTexSRV ) );

    HR( LoadFromDDSFile( L"Textures/bricks.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mBrickTexSRV ) );

    BuildShapeGeometryBuffers();
    BuildSkullGeometryBuffers();

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onResize( void )
{
    D3DApp::onResize();

    mCam.SetLens( 0.25f * MathHelper::Pi, getAspectRatio(), 1.f, 1000.f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::updateScene( const float dt )
{
    //
    // Camera.

    if ( GetAsyncKeyState( 'W' ) & 0x8000 ) {
        mCam.Walk( 10.f * dt );
    }
    else if ( GetAsyncKeyState( 'S' ) & 0x8000 ) {
        mCam.Walk( -10.f * dt );
    }
    if ( GetAsyncKeyState( 'D' ) & 0x8000 ) {
        mCam.Strafe( 10.f * dt );
    }
    else if ( GetAsyncKeyState( 'A' ) & 0x8000 ) {
        mCam.Strafe( -10.f * dt );
    }
    if ( GetAsyncKeyState( 'E' ) & 0x8000 ) {
        mCam.Climb( 10.f * dt );
    }
    else if ( GetAsyncKeyState( 'Q' ) & 0x8000 ) {
        mCam.Climb( -10.f * dt );
    }

    if ( GetAsyncKeyState( '0' ) & 0x8000 )
        mLightCount = 0;

    if ( GetAsyncKeyState( '1' ) & 0x8000 )
        mLightCount = 1;

    if ( GetAsyncKeyState( '2' ) & 0x8000 )
        mLightCount = 2;

    if ( GetAsyncKeyState( '3' ) & 0x8000 )
        mLightCount = 3;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT stride = sizeof( Vertex::Basic32 );
    UINT offset = 0;

    mCam.UpdateViewMatrix();

    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    XMMATRIX viewProj = mCam.ViewProj();

    float blendFactor [] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Set per frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mCam.GetPosition() );
    Effects::BasicFX->SetCubeMap( mSky->CubeMapSRV() );

    // Figure out which technique to use.  Skull does not have texture coordinates,
    // so we need a separate technique for it, and not every surface is reflective,
    // so don't pay for cubemap look up.

    ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;
    ID3DX11EffectTechnique* activeReflectTech = Effects::BasicFX->Light1TexReflectTech;
    ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light1ReflectTech;
    switch ( mLightCount )
    {
    case 1:
        activeTexTech = Effects::BasicFX->Light1TexTech;
        activeReflectTech = Effects::BasicFX->Light1TexReflectTech;
        activeSkullTech = Effects::BasicFX->Light1ReflectTech;
        break;
    case 2:
        activeTexTech = Effects::BasicFX->Light2TexTech;
        activeReflectTech = Effects::BasicFX->Light2TexReflectTech;
        activeSkullTech = Effects::BasicFX->Light2ReflectTech;
        break;
    case 3:
        activeTexTech = Effects::BasicFX->Light3TexTech;
        activeReflectTech = Effects::BasicFX->Light3TexReflectTech;
        activeSkullTech = Effects::BasicFX->Light3ReflectTech;
        break;
    }

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldViewProj;

    //
    // Draw the grid, cylinders, and box without any cubemap reflection.
    // 
    D3DX11_TECHNIQUE_DESC techDesc;
    activeTexTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mShapesVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mShapesIB, DXGI_FORMAT_R32_UINT, 0 );

        // Draw the grid.
        world = XMLoadFloat4x4( &mGridWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixScaling( 6.0f, 8.0f, 1.0f ) );
        Effects::BasicFX->SetMaterial( mGridMat );
        Effects::BasicFX->SetDiffuseMap( mFloorTexSRV );

        activeTexTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mGridIndexCount, mGridIndexOffset, mGridVertexOffset );

        // Draw the box.
        world = XMLoadFloat4x4( &mBoxWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
        Effects::BasicFX->SetMaterial( mBoxMat );
        Effects::BasicFX->SetDiffuseMap( mStoneTexSRV );

        activeTexTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset );

        // Draw the cylinders.
        for ( int i = 0; i < 10; ++i )
        {
            world = XMLoadFloat4x4( &mCylWorld[i] );
            worldInvTranspose = MathHelper::InverseTranspose( world );
            worldViewProj = world*view*proj;

            Effects::BasicFX->SetWorld( world );
            Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
            Effects::BasicFX->SetWorldViewProj( worldViewProj );
            Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
            Effects::BasicFX->SetMaterial( mCylinderMat );
            Effects::BasicFX->SetDiffuseMap( mBrickTexSRV );

            activeTexTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset );
        }
    }

    //
    // Draw the spheres with cubemap reflection.
    //
    activeReflectTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        // Draw the spheres.
        for ( int i = 0; i < 10; ++i )
        {
            world = XMLoadFloat4x4( &mSphereWorld[i] );
            worldInvTranspose = MathHelper::InverseTranspose( world );
            worldViewProj = world*view*proj;

            Effects::BasicFX->SetWorld( world );
            Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
            Effects::BasicFX->SetWorldViewProj( worldViewProj );
            Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
            Effects::BasicFX->SetMaterial( mSphereMat );
            Effects::BasicFX->SetDiffuseMap( mStoneTexSRV );

            activeReflectTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );
        }
    }

    activeSkullTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        // Draw the skull.

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mSkullVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        world = XMLoadFloat4x4( &mSkullWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mSkullMat );

        activeSkullTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSkullIndexCount, 0, 0 );
    }

    mSky->Draw( mD3DImmediateContext, mCam );

    // restore default states, as the SkyFX changes them in the effect file.
    mD3DImmediateContext->RSSetState( 0 );
    mD3DImmediateContext->OMSetDepthStencilState( 0, 0 );

    HR( mSwapChain->Present( 0, 0 ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onMouseDown( WPARAM btnState, int x, int y )
{
    if ( ( btnState & MK_LBUTTON ) != 0 ) {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture( mMainWindow );
    }
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
        // Each pixel corresponds to quarter of degree.
        const float dx = XMConvertToRadians( 0.25f * static_cast<float>( x - mLastMousePos.x ) );
        const float dy = XMConvertToRadians( 0.25f * static_cast<float>( y - mLastMousePos.y ) );

        mCam.Pitch( dy );
        mCam.RotateY( dx );
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildShapeGeometryBuffers()
{
    GeometryGenerator::MeshData box;
    GeometryGenerator::MeshData grid;
    GeometryGenerator::MeshData sphere;
    GeometryGenerator::MeshData cylinder;

    GeometryGenerator geoGen;
    geoGen.createBox( 1.0f, 1.0f, 1.0f, box );
    geoGen.createGrid( 20.0f, 30.0f, 60, 40, grid );
    geoGen.createSphere( 0.5f, 20, 20, sphere );
    geoGen.createCylinder( 0.5f, 0.3f, 3.0f, 20, 20, cylinder );

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;
    mGridVertexOffset = box.vertices.size();
    mSphereVertexOffset = mGridVertexOffset + grid.vertices.size();
    mCylinderVertexOffset = mSphereVertexOffset + sphere.vertices.size();

    // Cache the index count of each object.
    mBoxIndexCount = box.indices.size();
    mGridIndexCount = grid.indices.size();
    mSphereIndexCount = sphere.indices.size();
    mCylinderIndexCount = cylinder.indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;
    mGridIndexOffset = mBoxIndexCount;
    mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
    mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

    UINT totalVertexCount =
        box.vertices.size() +
        grid.vertices.size() +
        sphere.vertices.size() +
        cylinder.vertices.size();

    UINT totalIndexCount =
        mBoxIndexCount +
        mGridIndexCount +
        mSphereIndexCount +
        mCylinderIndexCount;

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

    for ( size_t i = 0; i < grid.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = grid.vertices[i].position;
        vertices[k].normal = grid.vertices[i].normal;
        vertices[k].tex = grid.vertices[i].texC;
    }

    for ( size_t i = 0; i < sphere.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = sphere.vertices[i].position;
        vertices[k].normal = sphere.vertices[i].normal;
        vertices[k].tex = sphere.vertices[i].texC;
    }

    for ( size_t i = 0; i < cylinder.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = cylinder.vertices[i].position;
        vertices[k].normal = cylinder.vertices[i].normal;
        vertices[k].tex = cylinder.vertices[i].texC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mShapesVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    std::vector<UINT> indices;
    indices.insert( indices.end(), box.indices.begin(), box.indices.end() );
    indices.insert( indices.end(), grid.indices.begin(), grid.indices.end() );
    indices.insert( indices.end(), sphere.indices.begin(), sphere.indices.end() );
    indices.insert( indices.end(), cylinder.indices.begin(), cylinder.indices.end() );

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mShapesIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildSkullGeometryBuffers()
{
    std::ifstream fin( "Models/skull.txt" );

    if ( !fin )
    {
        MessageBox( 0, L"Models/skull.txt not found.", 0, 0 );
        return;
    }

    UINT vcount = 0;
    UINT tcount = 0;
    std::string ignore;

    fin >> ignore >> vcount;
    fin >> ignore >> tcount;
    fin >> ignore >> ignore >> ignore >> ignore;

    std::vector<Vertex::Basic32> vertices( vcount );
    for ( UINT i = 0; i < vcount; ++i )
    {
        fin >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
        fin >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;
    }

    fin >> ignore;
    fin >> ignore;
    fin >> ignore;

    mSkullIndexCount = 3 * tcount;
    std::vector<UINT> indices( mSkullIndexCount );
    for ( UINT i = 0; i < tcount; ++i )
    {
        fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mSkullVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * mSkullIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mSkullIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //