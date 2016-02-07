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

    void drawScene( const Camera& camera, bool drawSkull );
    void BuildCubeFaceCamera( float x, float y, float z );
    void BuildDynamicCubeMapViews();
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

    ID3D11DepthStencilView* mDynamicCubeMapDSV;
    ID3D11RenderTargetView* mDynamicCubeMapRTV[6];
    ID3D11ShaderResourceView* mDynamicCubeMapSRV;
    D3D11_VIEWPORT mCubeMapViewport;
    static const int CubeMapSize = 256;

    DirectionalLight mDirLights[3];
    Material mGridMat;
    Material mBoxMat;
    Material mCylinderMat;
    Material mSphereMat;
    Material mSkullMat;
    Material mCenterSphereMat;

    // Define transformations from local spaces to world space.
    XMFLOAT4X4 mSphereWorld[10];
    XMFLOAT4X4 mCylWorld[10];
    XMFLOAT4X4 mBoxWorld;
    XMFLOAT4X4 mGridWorld;
    XMFLOAT4X4 mSkullWorld;
    XMFLOAT4X4 mCenterSphereWorld;

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
    Camera mCubeMapCamera[6];

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
    mDynamicCubeMapDSV( 0 ), mDynamicCubeMapSRV( 0 ),
    mSkullIndexCount( 0 ), mLightCount( 3 )
{
    mMainWindowCaption = L"Cube Mapping Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition( 0.0f, 2.0f, -15.0f );

    BuildCubeFaceCamera( 0.0f, 2.0f, 0.0f );

    for ( int i = 0; i < 6; ++i )
    {
        mDynamicCubeMapRTV[i] = 0;
    }

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4( &mGridWorld, I );

    XMMATRIX boxScale = XMMatrixScaling( 3.0f, 1.0f, 3.0f );
    XMMATRIX boxOffset = XMMatrixTranslation( 0.0f, 0.5f, 0.0f );
    XMStoreFloat4x4( &mBoxWorld, XMMatrixMultiply( boxScale, boxOffset ) );

    XMMATRIX centerSphereScale = XMMatrixScaling( 2.0f, 2.0f, 2.0f );
    XMMATRIX centerSphereOffset = XMMatrixTranslation( 0.0f, 2.0f, 0.0f );
    XMStoreFloat4x4( &mCenterSphereWorld, XMMatrixMultiply( centerSphereScale, centerSphereOffset ) );

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

    mSphereMat.ambient = XMFLOAT4( 0.6f, 0.8f, 1.0f, 1.0f );
    mSphereMat.diffuse = XMFLOAT4( 0.6f, 0.8f, 1.0f, 1.0f );
    mSphereMat.specular = XMFLOAT4( 0.9f, 0.9f, 0.9f, 16.0f );
    mSphereMat.reflect = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );

    mBoxMat.ambient = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mBoxMat.reflect = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );

    mSkullMat.ambient = XMFLOAT4( 0.4f, 0.4f, 0.4f, 1.0f );
    mSkullMat.diffuse = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mSkullMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mSkullMat.reflect = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );

    mCenterSphereMat.ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mCenterSphereMat.diffuse = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mCenterSphereMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
    mCenterSphereMat.reflect = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
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
    ReleaseCOM( mDynamicCubeMapDSV );
    ReleaseCOM( mDynamicCubeMapSRV );
    for ( int i = 0; i < 6; ++i )
        ReleaseCOM( mDynamicCubeMapRTV[i] );

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

    mSky = new Sky( mD3DDevice, L"Textures/sunsetcube1024.dds", 5000.0f );

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

    BuildDynamicCubeMapViews();

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

    XMMATRIX skullScale = XMMatrixScaling( 0.2f, 0.2f, 0.2f );
    XMMATRIX skullOffset = XMMatrixTranslation( 3.0f, 2.0f, 0.0f );
    XMMATRIX skullLocalRotate = XMMatrixRotationY( 2.0f*mTimer.totalTime() );
    XMMATRIX skullGlobalRotate = XMMatrixRotationY( 0.5f*mTimer.totalTime() );
    XMStoreFloat4x4( &mSkullWorld, skullScale*skullLocalRotate*skullOffset*skullGlobalRotate );

    mCam.UpdateViewMatrix();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    ID3D11RenderTargetView* renderTargets[1];

    // Generate the cube map.
    mD3DImmediateContext->RSSetViewports( 1, &mCubeMapViewport );
    for ( int i = 0; i < 6; ++i )
    {
        // Clear cube map face and depth buffer.
        mD3DImmediateContext->ClearRenderTargetView( mDynamicCubeMapRTV[i], reinterpret_cast<const float*>( &Colors::Silver ) );
        mD3DImmediateContext->ClearDepthStencilView( mDynamicCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

        // Bind cube map face as render target.
        renderTargets[0] = mDynamicCubeMapRTV[i];
        mD3DImmediateContext->OMSetRenderTargets( 1, renderTargets, mDynamicCubeMapDSV );

        // Draw the scene with the exception of the center sphere to this cube map face.
        drawScene( mCubeMapCamera[i], false );
    }

    // Restore old viewport and render targets.
    mD3DImmediateContext->RSSetViewports( 1, &mViewport );
    renderTargets[0] = mRenderTargetView;
    mD3DImmediateContext->OMSetRenderTargets( 1, renderTargets, mDepthStencilView );

    // Have hardware generate lower mipmap levels of cube map.
    mD3DImmediateContext->GenerateMips( mDynamicCubeMapSRV );

    // Now draw the scene as normal, but with the center sphere.
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    drawScene( mCam, true );

    HR( mSwapChain->Present( 0, 0 ) );
}

void App::drawScene( const Camera& camera, bool drawCenterSphere )
{
    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT stride = sizeof( Vertex::Basic32 );
    UINT offset = 0;

    XMMATRIX view = camera.View();
    XMMATRIX proj = camera.Proj();
    XMMATRIX viewProj = camera.ViewProj();

    float blendFactor [] = { 0.0f, 0.0f, 0.0f, 0.0f };

    // Set per frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mCam.GetPosition() );
    Effects::BasicFX->SetCubeMap( mSky->CubeMapSRV() );

    // Figure out which technique to use.   

    ID3DX11EffectTechnique* activeTexTech = Effects::BasicFX->Light1TexTech;
    ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light1Tech;
    ID3DX11EffectTechnique* activeRTech = Effects::BasicFX->Light1TexReflectTech;
    ID3DX11EffectTechnique* activeReflectTech = Effects::BasicFX->Light1ReflectTech;
    switch ( mLightCount )
    {
    case 1:
        activeTexTech = Effects::BasicFX->Light1TexTech;
        activeSkullTech = Effects::BasicFX->Light1Tech;
        activeReflectTech = Effects::BasicFX->Light1ReflectTech;
        break;
    case 2:
        activeTexTech = Effects::BasicFX->Light2TexTech;
        activeSkullTech = Effects::BasicFX->Light2Tech;
        activeReflectTech = Effects::BasicFX->Light2ReflectTech;
        break;
    case 3:
        activeTexTech = Effects::BasicFX->Light3TexTech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        activeReflectTech = Effects::BasicFX->Light3ReflectTech;
        break;
    }

    XMMATRIX world;
    XMMATRIX worldInvTranspose;
    XMMATRIX worldViewProj;

    //
    // Draw the skull.
    //
    D3DX11_TECHNIQUE_DESC techDesc;
    activeSkullTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mSkullVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        world = XMLoadFloat4x4( &mSkullWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
        Effects::BasicFX->SetMaterial( mSkullMat );

        activeSkullTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSkullIndexCount, 0, 0 );
    }

    mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mShapesVB, &stride, &offset );
    mD3DImmediateContext->IASetIndexBuffer( mShapesIB, DXGI_FORMAT_R32_UINT, 0 );

    //
    // Draw the grid, cylinders, spheres and box without any cubemap reflection.
    // 

    activeTexTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
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

    activeRTech->GetDesc( &techDesc );
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

            activeRTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );
        }
    }

    //
    // Draw the center sphere with the dynamic cube map.
    //
    if ( drawCenterSphere )
    {
        activeReflectTech->GetDesc( &techDesc );
        for ( UINT p = 0; p < techDesc.Passes; ++p )
        {
            // Draw the center sphere.

            world = XMLoadFloat4x4( &mCenterSphereWorld );
            worldInvTranspose = MathHelper::InverseTranspose( world );
            worldViewProj = world*view*proj;

            Effects::BasicFX->SetWorld( world );
            Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
            Effects::BasicFX->SetWorldViewProj( worldViewProj );
            Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
            Effects::BasicFX->SetMaterial( mCenterSphereMat );
            Effects::BasicFX->SetDiffuseMap( mStoneTexSRV );
            Effects::BasicFX->SetCubeMap( mDynamicCubeMapSRV );

            activeReflectTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );
        }
    }

    mSky->Draw( mD3DImmediateContext, camera );

    // restore default states, as the SkyFX changes them in the effect file.
    mD3DImmediateContext->RSSetState( 0 );
    mD3DImmediateContext->OMSetDepthStencilState( 0, 0 );
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

void App::BuildCubeFaceCamera( float x, float y, float z )
{
    // Generate the cube map about the given position.
    XMFLOAT3 center( x, y, z );
    XMFLOAT3 worldUp( 0.0f, 1.0f, 0.0f );

    // Look along each coordinate axis.
    XMFLOAT3 targets[6] =
    {
        XMFLOAT3( x + 1.0f, y, z ), // +X
        XMFLOAT3( x - 1.0f, y, z ), // -X
        XMFLOAT3( x, y + 1.0f, z ), // +Y
        XMFLOAT3( x, y - 1.0f, z ), // -Y
        XMFLOAT3( x, y, z + 1.0f ), // +Z
        XMFLOAT3( x, y, z - 1.0f )  // -Z
    };

    // Use world up vector (0,1,0) for all directions except +Y/-Y.  In these cases, we
    // are looking down +Y or -Y, so we need a different "up" vector.
    XMFLOAT3 ups[6] =
    {
        XMFLOAT3( 0.0f, 1.0f, 0.0f ),  // +X
        XMFLOAT3( 0.0f, 1.0f, 0.0f ),  // -X
        XMFLOAT3( 0.0f, 0.0f, -1.0f ), // +Y
        XMFLOAT3( 0.0f, 0.0f, +1.0f ), // -Y
        XMFLOAT3( 0.0f, 1.0f, 0.0f ),	 // +Z
        XMFLOAT3( 0.0f, 1.0f, 0.0f )	 // -Z
    };

    for ( int i = 0; i < 6; ++i )
    {
        mCubeMapCamera[i].LookAt( center, targets[i], ups[i] );
        mCubeMapCamera[i].SetLens( 0.5f*XM_PI, 1.0f, 0.1f, 1000.0f );
        mCubeMapCamera[i].UpdateViewMatrix();
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildDynamicCubeMapViews()
{
    //
    // Cubemap is a special texture array with 6 elements.
    //

    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = CubeMapSize;
    texDesc.Height = CubeMapSize;
    texDesc.MipLevels = 0;
    texDesc.ArraySize = 6;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

    ID3D11Texture2D* cubeTex = 0;
    HR( mD3DDevice->CreateTexture2D( &texDesc, 0, &cubeTex ) );

    //
    // Create a render target view to each cube map face 
    // (i.e., each element in the texture array).
    // 

    D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
    viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY; 
    viewDesc.Texture2DArray.FirstArraySlice = 0;
    viewDesc.Texture2DArray.ArraySize = 1;
    viewDesc.Texture2DArray.MipSlice = 0;

    for ( int i = 0; i < 6; ++i )
    {
        viewDesc.Texture2DArray.FirstArraySlice = i;
        HR( mD3DDevice->CreateRenderTargetView( cubeTex, &viewDesc, &mDynamicCubeMapRTV[i] ) );
    }

    //
    // Create a shader resource view to the cube map.
    //

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = -1;

    HR( mD3DDevice->CreateShaderResourceView( cubeTex, &srvDesc, &mDynamicCubeMapSRV ) );

    ReleaseCOM( cubeTex );

    //
    // We need a depth texture for rendering the scene into the cubemap
    // that has the same resolution as the cubemap faces.  
    //

    D3D11_TEXTURE2D_DESC depthTexDesc;
    depthTexDesc.Width = CubeMapSize;
    depthTexDesc.Height = CubeMapSize;
    depthTexDesc.MipLevels = 1;
    depthTexDesc.ArraySize = 1;
    depthTexDesc.SampleDesc.Count = 1;
    depthTexDesc.SampleDesc.Quality = 0;
    depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
    depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthTexDesc.CPUAccessFlags = 0;
    depthTexDesc.MiscFlags = 0;

    ID3D11Texture2D* depthTex = 0;
    HR( mD3DDevice->CreateTexture2D( &depthTexDesc, 0, &depthTex ) );

    // Create the depth stencil view for the entire cube
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format = depthTexDesc.Format;
    dsvDesc.Flags = 0;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    HR( mD3DDevice->CreateDepthStencilView( depthTex, &dsvDesc, &mDynamicCubeMapDSV ) );

    ReleaseCOM( depthTex );

    //
    // Viewport for drawing into cubemap.
    // 

    mCubeMapViewport.TopLeftX = 0.0f;
    mCubeMapViewport.TopLeftY = 0.0f;
    mCubeMapViewport.Width = (float)CubeMapSize;
    mCubeMapViewport.Height = (float)CubeMapSize;
    mCubeMapViewport.MinDepth = 0.0f;
    mCubeMapViewport.MaxDepth = 1.0f;
}

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