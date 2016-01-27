// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"

#include "BlurFilter.h"
#include "d3dx11Effect.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "RenderStates.h"
#include "Vertex.h"
#include "Waves.h"

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

using namespace DirectX;

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

enum RenderOptions {
    Lighting = 0,
    Textures = 1,
    TexturesAndFog = 2
};

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

    void BuildQuadPatchBuffer();

private:

    ID3D11Buffer* mQuadPatchVB;

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    XMFLOAT3 mEyePosW;

    float mTheta;
    float mPhi;
    float mRadius;

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
    : D3DApp( hInstance ), mQuadPatchVB( 0 ),
    mEyePosW( 0.0f, 0.0f, 0.0f ), mTheta( 1.3f*MathHelper::Pi ), mPhi( 0.4f*MathHelper::Pi ), mRadius( 80.0f )
{
    mMainWindowCaption = L"Bezier Surface Demo";
    mEnable4xMsaa = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4( &mView, I );
    XMStoreFloat4x4( &mProj, I );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    mD3DImmediateContext->ClearState();
    ReleaseCOM( mQuadPatchVB );

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

    BuildQuadPatchBuffer();

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
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    float blendFactor [] = { 0.0f, 0.0f, 0.0f, 0.0f };

    XMMATRIX view = XMLoadFloat4x4( &mView );
    XMMATRIX proj = XMLoadFloat4x4( &mProj );
    XMMATRIX viewProj = view*proj;

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Pos );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST );

    UINT stride = sizeof( Vertex::Pos );
    UINT offset = 0;

    // Set per frame constants.
    Effects::BezierTessellationFX->SetEyePosW( mEyePosW );
    Effects::BezierTessellationFX->SetFogColor( Colors::Silver );
    Effects::BezierTessellationFX->SetFogStart( 15.0f );
    Effects::BezierTessellationFX->SetFogRange( 175.0f );

    D3DX11_TECHNIQUE_DESC techDesc;
    Effects::BezierTessellationFX->TessTech->GetDesc( &techDesc );

    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mQuadPatchVB, &stride, &offset );

        // Set per object constants.
        XMMATRIX world = XMMatrixIdentity();
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BezierTessellationFX->SetWorld( world );
        Effects::BezierTessellationFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BezierTessellationFX->SetWorldViewProj( worldViewProj );
        Effects::BezierTessellationFX->SetTexTransform( XMMatrixIdentity() );
        //Effects::BezierTessellationFX->SetMaterial(0);
        Effects::BezierTessellationFX->SetDiffuseMap( 0 );

        Effects::BezierTessellationFX->TessTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );

        mD3DImmediateContext->RSSetState( RenderStates::WireframeRS );
        mD3DImmediateContext->Draw( 16, 0 );
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
        float dx = 0.1f * static_cast<float>( x - mLastMousePos.x );
        float dy = 0.1f * static_cast<float>( y - mLastMousePos.y );

        // Update camera radius.
        mRadius += dx - dy;

        // Restrict radius.
        mRadius = MathHelper::Clamp( mRadius, 3.f, 500.f );
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildQuadPatchBuffer()
{
    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( XMFLOAT3 ) * 16;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;

    XMFLOAT3 vertices[16] =
    {
        // Row 0
        XMFLOAT3( -10.0f, -10.0f, +15.0f ),
        XMFLOAT3( -5.0f,  0.0f, +15.0f ),
        XMFLOAT3( +5.0f,  0.0f, +15.0f ),
        XMFLOAT3( +10.0f, 0.0f, +15.0f ),

        // Row 1
        XMFLOAT3( -15.0f, 0.0f, +5.0f ),
        XMFLOAT3( -5.0f,  0.0f, +5.0f ),
        XMFLOAT3( +5.0f,  20.0f, +5.0f ),
        XMFLOAT3( +15.0f, 0.0f, +5.0f ),

        // Row 2
        XMFLOAT3( -15.0f, 0.0f, -5.0f ),
        XMFLOAT3( -5.0f,  0.0f, -5.0f ),
        XMFLOAT3( +5.0f,  0.0f, -5.0f ),
        XMFLOAT3( +15.0f, 0.0f, -5.0f ),

        // Row 3
        XMFLOAT3( -10.0f, 10.0f, -15.0f ),
        XMFLOAT3( -5.0f,  0.0f, -15.0f ),
        XMFLOAT3( +5.0f,  0.0f, -15.0f ),
        XMFLOAT3( +25.0f, 10.0f, -15.0f )
    };

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = vertices;
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mQuadPatchVB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
