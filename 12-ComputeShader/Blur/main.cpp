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

    void UpdateWaves();
    void DrawWrapper();
    void DrawScreenQuad( ID3D11ShaderResourceView* srv );
    float GetHillHeight( float x, float z ) const;
    XMFLOAT3 GetHillNormal( float x, float z ) const;
    void BuildLandGeometryBuffers();
    void BuildWaveGeometryBuffers();
    void BuildCrateGeometryBuffers();
    void BuildScreenQuadGeometryBuffers();
    void BuildOffscreenViews();

private:

    ID3D11Buffer* mLandVB;
    ID3D11Buffer* mLandIB;

    ID3D11Buffer* mWavesVB;
    ID3D11Buffer* mWavesIB;

    ID3D11Buffer* mBoxVB;
    ID3D11Buffer* mBoxIB;

    ID3D11Buffer* mScreenQuadVB;
    ID3D11Buffer* mScreenQuadIB;

    ID3D11ShaderResourceView* mGrassMapSRV;
    ID3D11ShaderResourceView* mWavesMapSRV;
    ID3D11ShaderResourceView* mCrateSRV;

    ID3D11ShaderResourceView* mOffscreenSRV;
    ID3D11UnorderedAccessView* mOffscreenUAV;
    ID3D11RenderTargetView* mOffscreenRTV;

    BlurFilter mBlur;
    Waves mWaves;

    DirectionalLight mDirLights[3];
    Material mLandMat;
    Material mWavesMat;
    Material mBoxMat;

    XMFLOAT4X4 mGrassTexTransform;
    XMFLOAT4X4 mWaterTexTransform;
    XMFLOAT4X4 mLandWorld;
    XMFLOAT4X4 mWavesWorld;
    XMFLOAT4X4 mBoxWorld;

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    UINT mLandIndexCount;
    UINT mWaveIndexCount;

    XMFLOAT2 mWaterTexOffset;

    RenderOptions mRenderOptions;

    int mBlurStrength;

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
    : D3DApp( hInstance ), mLandVB( 0 ), mLandIB( 0 ), mWavesVB( 0 ), mWavesIB( 0 ),
    mBoxVB( 0 ), mBoxIB( 0 ), mScreenQuadVB( 0 ), mScreenQuadIB( 0 ),
    mGrassMapSRV( 0 ), mWavesMapSRV( 0 ), mCrateSRV( 0 ), mOffscreenSRV( 0 ), mOffscreenUAV( 0 ), mOffscreenRTV( 0 ),
    mWaterTexOffset( 0.0f, 0.0f ), mEyePosW( 0.0f, 0.0f, 0.0f ), mLandIndexCount( 0 ), mWaveIndexCount( 0 ),
    mRenderOptions( RenderOptions::TexturesAndFog ),
    mTheta( 1.3f*MathHelper::Pi ), mPhi( 0.4f*MathHelper::Pi ), mRadius( 80.0f ), mBlurStrength( 8 )
{
    mMainWindowCaption = L"Blur Demo";

    ZeroMemory( &mLastMousePos, sizeof( POINT ) );

    CXMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4( &mLandWorld, I );
    XMStoreFloat4x4( &mWavesWorld, I );
    XMStoreFloat4x4( &mView, I );
    XMStoreFloat4x4( &mProj, I );

    XMMATRIX boxScale = XMMatrixScaling( 15.0f, 15.0f, 15.0f );
    XMMATRIX boxOffset = XMMatrixTranslation( 8.0f, 5.0f, -15.0f );
    XMStoreFloat4x4( &mBoxWorld, boxScale*boxOffset );

    XMMATRIX grassTexScale = XMMatrixScaling( 5.0f, 5.0f, 0.0f );
    XMStoreFloat4x4( &mGrassTexTransform, grassTexScale );

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

    mLandMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mLandMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mLandMat.specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 16.0f );

    mWavesMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mWavesMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 0.5f );
    mWavesMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 32.0f );

    mBoxMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mBoxMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.specular = XMFLOAT4( 0.4f, 0.4f, 0.4f, 16.0f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    mD3DImmediateContext->ClearState();
    
    ReleaseCOM( mLandVB );
    ReleaseCOM( mLandIB );
    ReleaseCOM( mWavesVB );
    ReleaseCOM( mWavesIB );
    ReleaseCOM( mBoxVB );
    ReleaseCOM( mBoxIB );
    ReleaseCOM( mScreenQuadVB );
    ReleaseCOM( mScreenQuadIB );

    ReleaseCOM( mGrassMapSRV );
    ReleaseCOM( mWavesMapSRV );
    ReleaseCOM( mCrateSRV );

    ReleaseCOM( mOffscreenSRV );
    ReleaseCOM( mOffscreenUAV );
    ReleaseCOM( mOffscreenRTV );

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
    HR( LoadFromDDSFile( L"Textures/grass.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mGrassMapSRV ) );
    image.reset( new ScratchImage() );
    ZeroMemory( &data, sizeof( data ) );
    HR( LoadFromDDSFile( L"Textures/water2.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mWavesMapSRV ) );
    image.reset( new ScratchImage() );
    ZeroMemory( &data, sizeof( data ) );
    HR( LoadFromDDSFile( L"Textures/WireFence.dds",
                         DDS_FLAGS_NONE,
                         &data,
                         *image ) );
    HR( CreateShaderResourceView( mD3DDevice,
                                  image->GetImages(),
                                  image->GetImageCount(),
                                  data,
                                  &mCrateSRV ) );

    mWaves.Init( 160, 160, 1.f, 0.03f, 5.f, 0.3f );

    BuildLandGeometryBuffers();
    BuildWaveGeometryBuffers();
    BuildCrateGeometryBuffers();
    BuildScreenQuadGeometryBuffers();
    BuildOffscreenViews();

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onResize( void )
{
    D3DApp::onResize();

    // Recreate the resources that depend on the client area size.
    BuildOffscreenViews();
    mBlur.Init( mD3DDevice, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM );

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

    //
    // Every quarter second, generate a random wave.
    //
    static float t_base = 0.0f;
    if ( ( mTimer.totalTime() - t_base ) >= 0.1f )
    {
        t_base += 0.1f;

        DWORD i = 5 + rand() % ( mWaves.RowCount() - 10 );
        DWORD j = 5 + rand() % ( mWaves.ColumnCount() - 10 );

        float r = MathHelper::RandF( 0.5f, 1.0f );

        mWaves.Disturb( i, j, r );
    }

    mWaves.Update( dt );

    // Update waves VB.
    D3D11_MAPPED_SUBRESOURCE md;
    HR( mD3DImmediateContext->Map( mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &md ) );
    Vertex::Basic32* v = reinterpret_cast<Vertex::Basic32*>( md.pData );
    for ( UINT i = 0; i < mWaves.VertexCount(); ++i ) {
        v[i].pos = mWaves[i];
        v[i].normal = mWaves.normal( i );

        // Derive tex-coords in [0,1] from position.
        v[i].tex.x = 0.5f + mWaves[i].x / mWaves.Width();
        v[i].tex.y = 0.5f - mWaves[i].z / mWaves.Depth();
    }

    mD3DImmediateContext->Unmap( mWavesVB, 0 );

    // Animate water texture.
    XMMATRIX wavesScale = XMMatrixScaling( 5.f, 5.f, 0.f );

    // Translate tex over time.
    mWaterTexOffset.x += 0.05f * dt;
    mWaterTexOffset.y += 0.1f * dt;
    XMMATRIX wavesOffset = XMMatrixTranslation( mWaterTexOffset.x, mWaterTexOffset.y, 0.f );

    // Combine scale and translation.
    XMStoreFloat4x4( &mWaterTexTransform, wavesScale * wavesOffset );

    // Switch the render mode based in key input.
    //
    if ( GetAsyncKeyState( '1' ) & 0x8000 )
        mRenderOptions = RenderOptions::Lighting;

    if ( GetAsyncKeyState( '2' ) & 0x8000 )
        mRenderOptions = RenderOptions::Textures;

    if ( GetAsyncKeyState( '3' ) & 0x8000 )
        mRenderOptions = RenderOptions::TexturesAndFog;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    // Render to offscreen buffer.
    ID3D11RenderTargetView* renderTargets[1] = { mOffscreenRTV };
    mD3DImmediateContext->OMSetRenderTargets( 1, renderTargets, mDepthStencilView );

    mD3DImmediateContext->ClearRenderTargetView( mOffscreenRTV,
                                                 reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView,
                                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                 1.f,
                                                 0 );

    // Draw scene to offscreen buffer.
    DrawWrapper();

    // Restore back buffer. The offscreen render target will serve as input to compute
    // shader for blurring, so we must unbind it from the OM stage before we can use
    // it as an input into the compute shader.
    renderTargets[0] = mRenderTargetView;
    mD3DImmediateContext->OMSetRenderTargets( 1, renderTargets, mDepthStencilView );

    // Blur offscreen buffer.
    //mBlur.BlurInPlace( mD3DImmediateContext, mOffscreenSRV, mOffscreenUAV, mBlurStrength );

    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    DrawScreenQuad( mOffscreenSRV );

    HR( mSwapChain->Present( 0, 0 ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::DrawWrapper( void )
{
    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    float blendFactor [] = { 0.0f, 0.0f, 0.0f, 0.0f };

    UINT stride = sizeof( Vertex::Basic32 );
    UINT offset = 0;

    XMMATRIX view = XMLoadFloat4x4( &mView );
    XMMATRIX proj = XMLoadFloat4x4( &mProj );
    XMMATRIX viewProj = view*proj;

    // Set per frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mEyePosW );
    Effects::BasicFX->SetFogColor( Colors::Silver );
    Effects::BasicFX->SetFogStart( 15.0f );
    Effects::BasicFX->SetFogRange( 175.0f );

    ID3DX11EffectTechnique* boxTech = Effects::BasicFX->Light3Tech;
    ID3DX11EffectTechnique* landAndWavesTech = Effects::BasicFX->Light3Tech;

    switch ( mRenderOptions )
    {
    case RenderOptions::Lighting:
        boxTech = Effects::BasicFX->Light3Tech;
        landAndWavesTech = Effects::BasicFX->Light3Tech;
        break;
    case RenderOptions::Textures:
        boxTech = Effects::BasicFX->Light3TexAlphaClipTech;
        landAndWavesTech = Effects::BasicFX->Light3TexTech;
        break;
    case RenderOptions::TexturesAndFog:
        boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
        landAndWavesTech = Effects::BasicFX->Light3TexFogTech;
        break;
    }

    D3DX11_TECHNIQUE_DESC techDesc;

    //
    // Draw the box with alpha clipping.
    // 

    boxTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mBoxVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mBoxIB, DXGI_FORMAT_R32_UINT, 0 );

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4( &mBoxWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMMatrixIdentity() );
        Effects::BasicFX->SetMaterial( mBoxMat );
        Effects::BasicFX->SetDiffuseMap( mCrateSRV );

        mD3DImmediateContext->RSSetState( RenderStates::NoCullRS );
        boxTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( 36, 0, 0 );

        // Restore default render state.
        mD3DImmediateContext->RSSetState( 0 );
    }

    //
    // Draw the hills and water with texture and fog (no alpha clipping needed).
    //

    landAndWavesTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        //
        // Draw the hills.
        //
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mLandVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mLandIB, DXGI_FORMAT_R32_UINT, 0 );

        // Set per object constants.
        XMMATRIX world = XMLoadFloat4x4( &mLandWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMLoadFloat4x4( &mGrassTexTransform ) );
        Effects::BasicFX->SetMaterial( mLandMat );
        Effects::BasicFX->SetDiffuseMap( mGrassMapSRV );

        landAndWavesTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mLandIndexCount, 0, 0 );

        //
        // Draw the waves.
        //
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mWavesVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mWavesIB, DXGI_FORMAT_R32_UINT, 0 );

        // Set per object constants.
        world = XMLoadFloat4x4( &mWavesWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetTexTransform( XMLoadFloat4x4( &mWaterTexTransform ) );
        Effects::BasicFX->SetMaterial( mWavesMat );
        Effects::BasicFX->SetDiffuseMap( mWavesMapSRV );

        mD3DImmediateContext->OMSetBlendState( RenderStates::TransparentBS, blendFactor, 0xffffffff );
        landAndWavesTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( 3 * mWaves.TriangleCount(), 0, 0 );

        // Restore default blend state
        mD3DImmediateContext->OMSetBlendState( 0, blendFactor, 0xffffffff );
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::DrawScreenQuad( ID3D11ShaderResourceView* srv )
{
    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT stride = sizeof( Vertex::Basic32 );
    UINT offset = 0;

    XMMATRIX identity = XMMatrixIdentity();
	identity = XMMatrixScaling( 0.5f, 0.5f, 1.f );

    ID3DX11EffectTechnique* texOnlyTech = Effects::BasicFX->Light0TexTech;
    D3DX11_TECHNIQUE_DESC techDesc;

    texOnlyTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mScreenQuadVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mScreenQuadIB, DXGI_FORMAT_R32_UINT, 0 );

        Effects::BasicFX->SetWorld( identity );
        Effects::BasicFX->SetWorldInvTranspose( identity );
        Effects::BasicFX->SetWorldViewProj( identity );
        Effects::BasicFX->SetTexTransform( identity );
        //Effects::BasicFX->SetDiffuseMap( mBlur.GetBlurredOutput() );
		Effects::BasicFX->SetDiffuseMap( srv );

        texOnlyTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( 6, 0, 0 );
    }
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

float App::GetHillHeight( float x, float z )const
{
    return 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );
}

XMFLOAT3 App::GetHillNormal( float x, float z )const
{
    // n = (-df/dx, 1, -df/dz)
    XMFLOAT3 n(
        -0.03f*z*cosf( 0.1f*x ) - 0.3f*cosf( 0.1f*z ),
        1.0f,
        -0.3f*sinf( 0.1f*x ) + 0.03f*x*sinf( 0.1f*z ) );

    XMVECTOR unitNormal = XMVector3Normalize( XMLoadFloat3( &n ) );
    XMStoreFloat3( &n, unitNormal );

    return n;
}

void App::BuildLandGeometryBuffers()
{
    GeometryGenerator::MeshData grid;

    GeometryGenerator geoGen;

    geoGen.createGrid( 160.0f, 160.0f, 50, 50, grid );

    mLandIndexCount = grid.indices.size();

    //
    // Extract the vertex elements we are interested and apply the height function to
    // each vertex.  
    //

    std::vector<Vertex::Basic32> vertices( grid.vertices.size() );
    for ( UINT i = 0; i < grid.vertices.size(); ++i )
    {
        XMFLOAT3 p = grid.vertices[i].position;

        p.y = GetHillHeight( p.x, p.z );

        vertices[i].pos = p;
        vertices[i].normal = GetHillNormal( p.x, p.z );
        vertices[i].tex = grid.vertices[i].texC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * grid.vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mLandVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * mLandIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &grid.indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mLandIB ) );
}

void App::BuildWaveGeometryBuffers()
{
    // Create the vertex buffer.  Note that we allocate space only, as
    // we will be updating the data every time step of the simulation.

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    HR( mD3DDevice->CreateBuffer( &vbd, 0, &mWavesVB ) );


    // Create the index buffer.  The index buffer is fixed, so we only 
    // need to create and set once.

    std::vector<UINT> indices( 3 * mWaves.TriangleCount() ); // 3 indices per face

                                                             // Iterate over each quad.
    UINT m = mWaves.RowCount();
    UINT n = mWaves.ColumnCount();
    int k = 0;
    for ( UINT i = 0; i < m - 1; ++i )
    {
        for ( DWORD j = 0; j < n - 1; ++j )
        {
            indices[k] = i*n + j;
            indices[k + 1] = i*n + j + 1;
            indices[k + 2] = ( i + 1 )*n + j;

            indices[k + 3] = ( i + 1 )*n + j;
            indices[k + 4] = i*n + j + 1;
            indices[k + 5] = ( i + 1 )*n + j + 1;

            k += 6; // next quad
        }
    }

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mWavesIB ) );
}

void App::BuildCrateGeometryBuffers()
{
    GeometryGenerator::MeshData box;

    GeometryGenerator geoGen;
    geoGen.createBox( 1.0f, 1.0f, 1.0f, box );

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<Vertex::Basic32> vertices( box.vertices.size() );

    for ( UINT i = 0; i < box.vertices.size(); ++i )
    {
        vertices[i].pos = box.vertices[i].position;
        vertices[i].normal = box.vertices[i].normal;
        vertices[i].tex = box.vertices[i].texC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * box.vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mBoxVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * box.indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &box.indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mBoxIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildScreenQuadGeometryBuffers( void )
{
    GeometryGenerator::MeshData quad;

    GeometryGenerator geoGen;
    geoGen.createFullscreenQuad( quad );

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<Vertex::Basic32> vertices( quad.vertices.size() );

    for ( UINT i = 0; i < quad.vertices.size(); ++i )
    {
        vertices[i].pos = quad.vertices[i].position;
        vertices[i].normal = quad.vertices[i].normal;
        vertices[i].tex = quad.vertices[i].texC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * quad.vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mScreenQuadVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * quad.indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &quad.indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mScreenQuadIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildOffscreenViews( void )
{
    // We call this function everytime the window is resized so that the render target is a quarter
    // the client area dimensions.  So Release the previous views before we create new ones.
    ReleaseCOM( mOffscreenSRV );
    ReleaseCOM( mOffscreenRTV );
    ReleaseCOM( mOffscreenUAV );

    D3D11_TEXTURE2D_DESC texDesc;

    texDesc.Width = mClientWidth;
    texDesc.Height = mClientHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;

    ID3D11Texture2D* offscreenTex = 0;
    HR( mD3DDevice->CreateTexture2D( &texDesc, 0, &offscreenTex ) );

    // Null description means to create a view to all mipmap levels using 
    // the format the texture was created with.
    HR( mD3DDevice->CreateShaderResourceView( offscreenTex, 0, &mOffscreenSRV ) );
    HR( mD3DDevice->CreateRenderTargetView( offscreenTex, 0, &mOffscreenRTV ) );
    HR( mD3DDevice->CreateUnorderedAccessView( offscreenTex, 0, &mOffscreenUAV ) );

    // View saves a reference to the texture so we can release our reference.
    ReleaseCOM( offscreenTex );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //