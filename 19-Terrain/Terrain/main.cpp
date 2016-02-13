// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "Camera.h"
#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "RenderStates.h"
#include "Sky.h"
#include "Terrain.h"
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

    Sky* mSky;
    Terrain mTerrain;

    DirectionalLight mDirLights[3];

    Camera mCam;

    bool mWalkCamMode;

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
    : D3DApp( hInstance ), mSky( nullptr ), mWalkCamMode( false )
{
    mMainWindowCaption = L"Terrain Demo";
    mEnable4xMsaa = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition( 0.0f, 2.0f, 100.0f );

    mDirLights[0].ambient = XMFLOAT4( 0.3f, 0.3f, 0.3f, 1.0f );
    mDirLights[0].diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mDirLights[0].specular = XMFLOAT4( 0.8f, 0.8f, 0.7f, 1.0f );
    mDirLights[0].direction = XMFLOAT3( 0.707f, -0.707f, 0.0f );

    mDirLights[1].ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mDirLights[1].diffuse = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[1].specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[1].direction = XMFLOAT3( 0.57735f, -0.57735f, 0.57735f );

    mDirLights[2].ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mDirLights[2].diffuse = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[2].specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[2].direction = XMFLOAT3( -0.57735f, -0.57735f, -0.57735f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    mD3DImmediateContext->ClearState();

    SafeDelete( mSky );

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
    RenderStates::DestroyAll();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() ) {
        return false;
    }

    // Must init Effects first since InputLayouts depend on shader signatures.
    Effects::InitAll( mD3DDevice );
    InputLayouts::InitAll( mD3DDevice );
    RenderStates::InitAll( mD3DDevice );

    mSky = new Sky( mD3DDevice, L"Textures/grasscube1024.dds", 5000.0f );

    Terrain::InitInfo tii;
    tii.HeightMapFilename = L"Textures/terrain.raw";
    tii.LayerMapFilename0 = L"Textures/grass.dds";
    tii.LayerMapFilename1 = L"Textures/darkdirt.dds";
    tii.LayerMapFilename2 = L"Textures/stone.dds";
    tii.LayerMapFilename3 = L"Textures/lightdirt.dds";
    tii.LayerMapFilename4 = L"Textures/snow.dds";
    tii.BlendMapFilename = L"Textures/blend.dds";
    tii.HeightScale = 50.0f;
    tii.HeightmapWidth = 2049;
    tii.HeightmapHeight = 2049;
    tii.CellSpacing = 0.5f;

    mTerrain.Init( mD3DDevice, mD3DImmediateContext, tii );
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

    //
    // Walk/fly mode

    if ( GetAsyncKeyState( '2' ) & 0x8000 )
        mWalkCamMode = true;
    if ( GetAsyncKeyState( '3' ) & 0x8000 )
        mWalkCamMode = false;

    // 
    // Clamp camera to terrain surface in walk mode.
    if ( mWalkCamMode )
    {
        XMFLOAT3 camPos = mCam.GetPosition();
        float y = mTerrain.GetHeight( camPos.x, camPos.z );
        mCam.SetPosition( camPos.x, y + 2.0f, camPos.z );
    }

    mCam.UpdateViewMatrix();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    float blendFactor [] = { 0.0f, 0.0f, 0.0f, 0.0f };

    if ( GetAsyncKeyState( '1' ) & 0x8000 )
        mD3DImmediateContext->RSSetState( RenderStates::WireframeRS );

    mTerrain.Draw( mD3DImmediateContext, mCam, mDirLights );

    mD3DImmediateContext->RSSetState( 0 );

    mSky->Draw( mD3DImmediateContext, mCam );

    // restore default states, as the SkyFX changes them in the effect file.
    mD3DImmediateContext->RSSetState( 0 );
    mD3DImmediateContext->OMSetDepthStencilState( 0, 0 );

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
