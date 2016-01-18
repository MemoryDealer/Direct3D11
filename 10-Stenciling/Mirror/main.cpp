// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
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

    void BuildRoomGeometryBuffers();
    void BuildSkullGeometryBuffers();

private:

    ID3D11Buffer* mRoomVB;

    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;

    ID3D11ShaderResourceView* mFloorDiffuseMapSRV;
    ID3D11ShaderResourceView* mWallDiffuseMapSRV;
    ID3D11ShaderResourceView* mMirrorDiffuseMapSRV;

    DirectionalLight mDirLights[3];
    Material mRoomMat;
    Material mSkullMat;
    Material mMirrorMat;
    Material mShadowMat;

    XMFLOAT4X4 mRoomWorld;
    XMFLOAT4X4 mSkullWorld;

    UINT mSkullIndexCount;
    XMFLOAT3 mSkullTranslation;

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    RenderOptions mRenderOptions;

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
    : D3DApp( hInstance ), mRoomVB( 0 ), mSkullVB( 0 ), mSkullIB( 0 ), mSkullIndexCount( 0 ), mSkullTranslation( 0.0f, 1.0f, -5.0f ),
    mFloorDiffuseMapSRV( 0 ), mWallDiffuseMapSRV( 0 ), mMirrorDiffuseMapSRV( 0 ),
    mEyePosW( 0.0f, 0.0f, 0.0f ), mRenderOptions( RenderOptions::Textures ),
    mTheta( 1.24f*MathHelper::Pi ), mPhi( 0.42f*MathHelper::Pi ), mRadius( 12.0f )
{
    mMainWindowCaption = L"Mirror Demo";
    mEnable4xMsaa = false;

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4( &mRoomWorld, I );
    XMStoreFloat4x4( &mSkullWorld, I );
    XMStoreFloat4x4( &mView, I );
    XMStoreFloat4x4( &mProj, I );

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

    mRoomMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mRoomMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mRoomMat.specular = XMFLOAT4( 0.4f, 0.4f, 0.4f, 16.0f );

    mSkullMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mSkullMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mSkullMat.specular = XMFLOAT4( 0.4f, 0.4f, 0.4f, 16.0f );

    // Reflected material is transparent so it blends into mirror.
    mMirrorMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mMirrorMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 0.5f );
    mMirrorMat.specular = XMFLOAT4( 0.4f, 0.4f, 0.4f, 16.0f );

    mShadowMat.ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mShadowMat.diffuse = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.5f );
    mShadowMat.specular = XMFLOAT4( 0.0f, 0.0f, 0.0f, 16.0f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    mD3DImmediateContext->ClearState();
    ReleaseCOM( mRoomVB );
    ReleaseCOM( mSkullVB );
    ReleaseCOM( mSkullIB );
    ReleaseCOM( mFloorDiffuseMapSRV );
    ReleaseCOM( mWallDiffuseMapSRV );
    ReleaseCOM( mMirrorDiffuseMapSRV );

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

    Effects::InitAll( mD3DDevice );
    InputLayouts::InitAll( mD3DDevice );
    RenderStates::InitAll( mD3DDevice );

    //HR (D3DX11CreateShader)
    TexMetadata data;
    std::unique_ptr<ScratchImage> image( new ScratchImage() );
    HR( LoadFromDDSFile( L"Textures/checkboard.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mFloorDiffuseMapSRV ) );
    HR( LoadFromDDSFile( L"Textures/brick01.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mWallDiffuseMapSRV ) );
    HR( LoadFromDDSFile( L"Textures/ice.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mMirrorDiffuseMapSRV ) );

    

    BuildRoomGeometryBuffers();
    BuildSkullGeometryBuffers();

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

    // Switch the render mode based in key input.
    if ( GetAsyncKeyState( '1' ) & 0x8000 )
        mRenderOptions = RenderOptions::Lighting;

    if ( GetAsyncKeyState( '2' ) & 0x8000 )
        mRenderOptions = RenderOptions::Textures;

    if ( GetAsyncKeyState( '3' ) & 0x8000 )
        mRenderOptions = RenderOptions::TexturesAndFog;

    // Allow user to move box.
    if ( GetAsyncKeyState( 'A' ) & 0x8000 ) {
        mSkullTranslation.x -= 1.f * dt;
    }
    else if ( GetAsyncKeyState( 'D' ) & 0x8000 ) {
        mSkullTranslation.x += 1.f * dt;
    }
    else if ( GetAsyncKeyState( 'W' ) & 0x8000 ) {
        mSkullTranslation.y += 1.f * dt;
    }
    else if ( GetAsyncKeyState( 'S' ) & 0x8000 ) {
        mSkullTranslation.y -= 1.f * dt;
    }

    // Keep skull above ground.
    mSkullTranslation.y = MathHelper::Max( mSkullTranslation.y, 0.f );

    // Update skull world matrix.
    XMMATRIX skullRotate = XMMatrixRotationY( 0.5f * MathHelper::Pi );
    XMMATRIX skullScale = XMMatrixScaling( 0.45f, 0.45f, 0.45f );
    XMMATRIX skullOffset = XMMatrixTranslation( mSkullTranslation.x,
                                                mSkullTranslation.y,
                                                mSkullTranslation.z );
    XMStoreFloat4x4( &mSkullWorld, skullRotate * skullScale * skullOffset );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView,
                                                 reinterpret_cast<const float*>( &Colors::Black ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView,
                                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                 1.f,
                                                 0 );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    float blendFactor [] = { 0.f, 0.f, 0.f, 0.f };
    const UINT stride = sizeof( Vertex::Basic32 );
    const UINT offset = 0;

    XMMATRIX view = XMLoadFloat4x4( &mView );
    XMMATRIX proj = XMLoadFloat4x4( &mProj );
    const XMMATRIX viewProj = view * proj;

    // Set per-frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mEyePosW );
    Effects::BasicFX->SetFogColor( Colors::Black );
    Effects::BasicFX->SetFogStart( 2.f );
    Effects::BasicFX->SetFogRange( 40.f );

    ID3DX11EffectTechnique* tech = Effects::BasicFX->Light0TexTech;
    ID3DX11EffectTechnique* activeSkullTech = Effects::BasicFX->Light0TexTech;

    switch ( mRenderOptions )
    {
    case RenderOptions::Lighting:
        tech = Effects::BasicFX->Light3Tech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::Textures:
        tech = Effects::BasicFX->Light3TexTech;
        activeSkullTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::TexturesAndFog:
        tech = Effects::BasicFX->Light3TexFogTech;
        activeSkullTech = Effects::BasicFX->Light3FogTech;
        break;
    }

    D3DX11_TECHNIQUE_DESC techDesc;

    // 
    // Draw floor and walls to back buffer as normal.

    tech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        ID3DX11EffectPass* pass = tech->GetPassByIndex( p );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mRoomVB, &stride, &offset );
        // ???

        // Set per-object constants.
        XMMATRIX world = XMLoadFloat4x4( &mRoomWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world * viewProj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
        Effects::BasicFX->SetMaterial( mRoomMat );

        // Floor.
        Effects::BasicFX->SetDiffuseMap( mFloorDiffuseMapSRV );
        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->Draw( 6, 0 );

        // Wall.
        Effects::BasicFX->SetDiffuseMap( mWallDiffuseMapSRV );
        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->Draw( 18, 6 );
    }

    //
    // Draw the skull to the back buffer as normal.

    activeSkullTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex( p );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mSkullVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        XMMATRIX world = XMLoadFloat4x4( &mSkullWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mSkullMat );

        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSkullIndexCount, 0, 0 );
    }

    //
    // Draw the mirror into the stencil buffer only.

    tech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        ID3DX11EffectPass* pass = tech->GetPassByIndex( 0 );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mRoomVB, &stride, &offset );

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4( &mRoomWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );

        // Do not write to render target.
        mD3DImmediateContext->OMSetBlendState( RenderStates::NoRenderTargetWritesBS,
                                               blendFactor,
                                               0xffffffff );

        // Render visible mirror pixels to stencil buffer.
        // Do not write mirror depth to depth buffer to avoid occluding the reflection.
        mD3DImmediateContext->OMSetDepthStencilState( RenderStates::MarkMirrorDSS,
                                                      1 );

        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->Draw( 6, 24 );

        // Restore states.
        mD3DImmediateContext->OMSetBlendState( nullptr, blendFactor, 0xffffffff );
        mD3DImmediateContext->OMSetDepthStencilState( nullptr, 0 );
    }

    //
    // Draw skull reflection to back buffer.

    activeSkullTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex( p );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mSkullVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        XMVECTOR mirrorPlane = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f ); // xy plane
        XMMATRIX R = XMMatrixReflect( mirrorPlane );
        //XMMATRIX world = XMMatrixRotationY( MathHelper::Pi ) * XMLoadFloat4x4( &mSkullWorld )  * R;
        XMMATRIX world = XMLoadFloat4x4( &mSkullWorld ) * R;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mSkullMat );

        // Cache old light directions, then reflect them.
        XMFLOAT3 oldLightDirs[3];
        for ( int i = 0; i < 3; ++i ) {
            oldLightDirs[i] = mDirLights[i].direction;

            XMVECTOR dir = XMLoadFloat3( &mDirLights[i].direction );
            XMVECTOR reflection = XMVector3TransformNormal( dir, R );
            XMStoreFloat3( &mDirLights[i].direction, reflection );
        }

        Effects::BasicFX->SetDirLights( mDirLights );

        // Cull clockwise triangles for reflection.
        mD3DImmediateContext->RSSetState( RenderStates::CullClockwiseRS );

        // Only draw reflection into visible mirror pixels as marked by stencil buffer.
        mD3DImmediateContext->OMSetDepthStencilState( RenderStates::DrawReflectionDSS,
                                                      1 );
        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSkullIndexCount, 0, 0 );

        // Restore default states.
        mD3DImmediateContext->RSSetState( nullptr );
        mD3DImmediateContext->OMSetDepthStencilState( nullptr, 0 );

        // Restore light directions.
        for ( int i = 0; i < 3; ++i )
        {
            mDirLights[i].direction = oldLightDirs[i];
        }

        Effects::BasicFX->SetDirLights( mDirLights );
    }

    // Draw mirror to back buffer.
    tech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        ID3DX11EffectPass* pass = tech->GetPassByIndex( p );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mRoomVB, &stride, &offset );

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4( &mRoomWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
        Effects::BasicFX->SetMaterial( mMirrorMat );
        Effects::BasicFX->SetDiffuseMap( mMirrorDiffuseMapSRV );

        // Mirror
        mD3DImmediateContext->OMSetBlendState( RenderStates::TransparentBS, blendFactor, 0xffffffff );
        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->Draw( 6, 24 );
    }

    // Draw skull shadow.
    activeSkullTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        ID3DX11EffectPass* pass = activeSkullTech->GetPassByIndex( p );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mSkullVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        XMVECTOR shadowPlane = XMVectorSet( 0.f, 1.f, 0.f, 0.f );
        XMVECTOR toMainLight = -XMLoadFloat3( &mDirLights[0].direction );
        XMMATRIX S = XMMatrixShadow( shadowPlane, toMainLight );
        XMMATRIX shadowOffsetY = XMMatrixTranslation( 0.f, 0.001f, 0.f );

        // Set per-object constants.
        XMMATRIX world = XMLoadFloat4x4( &mSkullWorld ) * S * shadowOffsetY;
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mShadowMat );

        mD3DImmediateContext->OMSetDepthStencilState( RenderStates::NoDoubleBlendDSS,
                                                      0 );
        pass->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSkullIndexCount, 0, 0 );

        // Restore default states.
        mD3DImmediateContext->OMSetBlendState( 0, blendFactor, 0xffffffff );
        mD3DImmediateContext->OMSetDepthStencilState( 0, 0 );
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
    if ( ( btnState & MK_LBUTTON ) != 0 )
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians( 0.25f*static_cast<float>( x - mLastMousePos.x ) );
        float dy = XMConvertToRadians( 0.25f*static_cast<float>( y - mLastMousePos.y ) );

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp( mPhi, 0.1f, MathHelper::Pi - 0.1f );
    }
    else if ( ( btnState & MK_RBUTTON ) != 0 )
    {
        // Make each pixel correspond to 0.01 unit in the scene.
        float dx = 0.01f*static_cast<float>( x - mLastMousePos.x );
        float dy = 0.01f*static_cast<float>( y - mLastMousePos.y );

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp( mRadius, 3.0f, 50.0f );
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildRoomGeometryBuffers()
{
    // Create and specify geometry.  For this sample we draw a floor
    // and a wall with a mirror on it.  We put the floor, wall, and
    // mirror geometry in one vertex buffer.
    //
    //   |--------------|
    //   |              |
    //   |----|----|----|
    //   |Wall|Mirr|Wall|
    //   |    | or |    |
    //   /--------------/
    //  /   Floor      /
    // /--------------/


    Vertex::Basic32 v[30];

    // Floor: Observe we tile texture coordinates.
    v[0] = Vertex::Basic32( -3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f );
    v[1] = Vertex::Basic32( -3.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f );
    v[2] = Vertex::Basic32( 7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f );

    v[3] = Vertex::Basic32( -3.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f );
    v[4] = Vertex::Basic32( 7.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 4.0f, 0.0f );
    v[5] = Vertex::Basic32( 7.5f, 0.0f, -10.0f, 0.0f, 1.0f, 0.0f, 4.0f, 4.0f );

    // Wall: Observe we tile texture coordinates, and that we
    // leave a gap in the middle for the mirror.
    v[6] = Vertex::Basic32( -3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f );
    v[7] = Vertex::Basic32( -3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f );
    v[8] = Vertex::Basic32( -2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f );

    v[9] = Vertex::Basic32( -3.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f );
    v[10] = Vertex::Basic32( -2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 0.0f );
    v[11] = Vertex::Basic32( -2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.5f, 2.0f );

    v[12] = Vertex::Basic32( 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f );
    v[13] = Vertex::Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f );
    v[14] = Vertex::Basic32( 7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f );

    v[15] = Vertex::Basic32( 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 2.0f );
    v[16] = Vertex::Basic32( 7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 0.0f );
    v[17] = Vertex::Basic32( 7.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 2.0f, 2.0f );

    v[18] = Vertex::Basic32( -3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f );
    v[19] = Vertex::Basic32( -3.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f );
    v[20] = Vertex::Basic32( 7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f );

    v[21] = Vertex::Basic32( -3.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f );
    v[22] = Vertex::Basic32( 7.5f, 6.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 0.0f );
    v[23] = Vertex::Basic32( 7.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 6.0f, 1.0f );

    // Mirror
    v[24] = Vertex::Basic32( -2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f );
    v[25] = Vertex::Basic32( -2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f );
    v[26] = Vertex::Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f );

    v[27] = Vertex::Basic32( -2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f );
    v[28] = Vertex::Basic32( 2.5f, 4.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f );
    v[29] = Vertex::Basic32( 2.5f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f );

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * 30;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mRoomVB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildSkullGeometryBuffers( void )
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