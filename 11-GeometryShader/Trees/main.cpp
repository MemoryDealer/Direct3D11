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

    float GetHillHeight( float x, float z )const;
    XMFLOAT3 GetHillNormal( float x, float z )const;
    void BuildLandGeometryBuffers();
    void BuildWaveGeometryBuffers();
    void BuildCrateGeometryBuffers();
    void BuildTreeSpritesBuffer();
    void DrawTreeSprites( CXMMATRIX viewProj );

private:

    ID3D11Buffer* mLandVB;
    ID3D11Buffer* mLandIB;

    ID3D11Buffer* mWavesVB;
    ID3D11Buffer* mWavesIB;

    ID3D11Buffer* mBoxVB;
    ID3D11Buffer* mBoxIB;

    ID3D11Buffer* mTreeSpritesVB;

    ID3D11ShaderResourceView* mGrassMapSRV;
    ID3D11ShaderResourceView* mWavesMapSRV;
    ID3D11ShaderResourceView* mBoxMapSRV;
    ID3D11ShaderResourceView* mTreeTextureMapArraySRV;

    Waves mWaves;

    DirectionalLight mDirLights[3];
    Material mLandMat;
    Material mWavesMat;
    Material mBoxMat;
    Material mTreeMat;

    XMFLOAT4X4 mGrassTexTransform;
    XMFLOAT4X4 mWaterTexTransform;
    XMFLOAT4X4 mLandWorld;
    XMFLOAT4X4 mWavesWorld;
    XMFLOAT4X4 mBoxWorld;

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

    UINT mLandIndexCount;

    XMFLOAT2 mWaterTexOffset;

    static const UINT TreeCount = 16;

    bool mAlphaToCoverageOn;    

    RenderOptions mRenderOptions;

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
    : D3DApp( hInstance ), mLandVB( 0 ), mLandIB( 0 ), mWavesVB( 0 ), mWavesIB( 0 ), mBoxVB( 0 ), mBoxIB( 0 ), mTreeSpritesVB( 0 ),
    mGrassMapSRV( 0 ), mWavesMapSRV( 0 ), mBoxMapSRV( 0 ), mAlphaToCoverageOn( true ),
    mWaterTexOffset( 0.0f, 0.0f ), mEyePosW( 0.0f, 0.0f, 0.0f ), mLandIndexCount( 0 ), mRenderOptions( RenderOptions::TexturesAndFog ),
    mTheta( 1.3f*MathHelper::Pi ), mPhi( 0.4f*MathHelper::Pi ), mRadius( 80.0f )
{
    mMainWindowCaption = L"Blending Demo";

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

    mTreeMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mTreeMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mTreeMat.specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 16.0f );
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
    ReleaseCOM( mTreeSpritesVB );
    ReleaseCOM( mGrassMapSRV );
    ReleaseCOM( mWavesMapSRV );
    ReleaseCOM( mBoxMapSRV );
    ReleaseCOM( mTreeTextureMapArraySRV );

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

    mWaves.Init( 160, 160, 1.f, 0.03f, 5.f, 0.3f );

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
                                  &mBoxMapSRV ) );

    std::vector<std::wstring> trees;
    trees.push_back( L"Textures/tree0.dds" );
    trees.push_back( L"Textures/tree1.dds" );
    trees.push_back( L"Textures/tree2.dds" );
    trees.push_back( L"Textures/tree3.dds" );    

    std::vector<ID3D11Texture2D*> textures;
    textures.resize( trees.size() );
    for ( auto& i : trees ) {     
        static int j = 0;

        HR( CreateDDSTextureFromFileEx( mD3DDevice,
                                        i.c_str(),
                                        0,
                                        D3D11_USAGE_STAGING,
                                        0,
                                        D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ,
                                        0,
                                        0,
                                        reinterpret_cast<ID3D11Resource**>( &textures[j++] ),
                                        nullptr ) );


    }

    // Create texture array.
    D3D11_TEXTURE2D_DESC desc;
    textures[0]->GetDesc( &desc );

    D3D11_TEXTURE2D_DESC arrayDesc;
    arrayDesc.Width = desc.Width;
    arrayDesc.Height = desc.Height;
    arrayDesc.MipLevels = desc.MipLevels;
    arrayDesc.ArraySize = textures.size();
    arrayDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    arrayDesc.SampleDesc.Count = 1;
    arrayDesc.SampleDesc.Quality = 0;
    arrayDesc.Usage = D3D11_USAGE_DEFAULT;
    arrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    arrayDesc.CPUAccessFlags = 0;
    arrayDesc.MiscFlags = 0;

    ID3D11Texture2D* texArray = nullptr;
    HR( mD3DDevice->CreateTexture2D( &arrayDesc, 0, &texArray ) );

    // Copy individual texture elements into array.
    for ( UINT texElement = 0; texElement < textures.size(); ++texElement ) {
        for ( UINT mipLevel = 0; mipLevel < desc.MipLevels; ++mipLevel ) {
            D3D11_MAPPED_SUBRESOURCE mappedTex2D;
            HR( mD3DImmediateContext->Map( textures[texElement], 
                                           mipLevel,
                                           D3D11_MAP_READ,
                                           0,
                                           &mappedTex2D ) );

            mD3DImmediateContext->UpdateSubresource( texArray,
                                                     D3D11CalcSubresource( mipLevel,
                                                                           texElement,
                                                                           desc.MipLevels ),
                                                     0,
                                                     mappedTex2D.pData,
                                                     mappedTex2D.RowPitch,
                                                     mappedTex2D.DepthPitch );
            mD3DImmediateContext->Unmap( textures[texElement], mipLevel );
        }
    }

    // Create resource view to texture array.
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
    viewDesc.Format = arrayDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    viewDesc.Texture2DArray.MostDetailedMip = 0;
    viewDesc.Texture2DArray.MipLevels = arrayDesc.MipLevels;
    viewDesc.Texture2DArray.FirstArraySlice = 0;
    viewDesc.Texture2DArray.ArraySize = textures.size();

    HR( mD3DDevice->CreateShaderResourceView( texArray,
                                              &viewDesc,
                                              &mTreeTextureMapArraySRV ) );

    // Cleanup textures.
    ReleaseCOM( texArray );
    for ( auto& i : textures ) {
        ReleaseCOM( i );
    }

    BuildLandGeometryBuffers();
    BuildWaveGeometryBuffers();
    BuildCrateGeometryBuffers();
    BuildTreeSpritesBuffer();

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

    if ( GetAsyncKeyState( 'R' ) & 0x8000 )
        mAlphaToCoverageOn = true;

    if ( GetAsyncKeyState( 'T' ) & 0x8000 )
        mAlphaToCoverageOn = false;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView,
                                                 reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView,
                                                 D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                 1.f,
                                                 0 );

    const UINT stride = sizeof( Vertex::Basic32 );
    const UINT offset = 0;

    // Set constant buffers.
    XMMATRIX view = XMLoadFloat4x4( &mView );
    XMMATRIX proj = XMLoadFloat4x4( &mProj );
    XMMATRIX viewProj = view * proj;

    DrawTreeSprites( viewProj );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    float blendFactor [] = { 0.f, 0.f, 0.f, 0.f };    

    // Set per frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mEyePosW );
    Effects::BasicFX->SetFogColor( Colors::Silver );
    Effects::BasicFX->SetFogStart( 15.0f );
    Effects::BasicFX->SetFogRange( 175.0f );

    ID3DX11EffectTechnique* boxTech = Effects::BasicFX->Light1Tech;
    ID3DX11EffectTechnique* landAndWavesTech = Effects::BasicFX->Light1Tech;

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
        Effects::BasicFX->SetDiffuseMap( mBoxMapSRV );

        mD3DImmediateContext->RSSetState( RenderStates::NoCullRS );
        boxTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( 36, 0, 0 );

        // Restore default render state.
        mD3DImmediateContext->RSSetState( 0 );
    }

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
        mD3DImmediateContext->OMSetBlendState( nullptr, blendFactor, 0xffffffff );
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

float App::GetHillHeight( float x, float z )const
{
    return 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

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

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

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

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

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

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

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

void App::BuildTreeSpritesBuffer()
{
    Vertex::TreePointSprite v[TreeCount];

    for ( UINT i = 0; i < TreeCount; ++i )
    {
        float x = MathHelper::RandF( -35.0f, 35.0f );
        float z = MathHelper::RandF( -35.0f, 35.0f );
        float y = GetHillHeight( x, z );

        // Move tree slightly above land height.
        y += 10.0f;

        v[i].pos = XMFLOAT3( x, y, z );
        v[i].size = XMFLOAT2( 24.0f, 24.0f );
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::TreePointSprite ) * TreeCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = v;
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mTreeSpritesVB ) );
}

void App::DrawTreeSprites( CXMMATRIX viewProj )
{
    Effects::TreeSpriteFX->SetDirLights( mDirLights );
    Effects::TreeSpriteFX->SetEyePosW( mEyePosW );
    Effects::TreeSpriteFX->SetFogColor( Colors::Silver );
    Effects::TreeSpriteFX->SetFogStart( 15.0f );
    Effects::TreeSpriteFX->SetFogRange( 175.0f );
    Effects::TreeSpriteFX->SetViewProj( viewProj );
    Effects::TreeSpriteFX->SetMaterial( mTreeMat );
    Effects::TreeSpriteFX->SetTreeTextureMapArray( mTreeTextureMapArraySRV );

    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    mD3DImmediateContext->IASetInputLayout( InputLayouts::TreePointSprite );
    UINT stride = sizeof( Vertex::TreePointSprite );
    UINT offset = 0;

    ID3DX11EffectTechnique* tech = Effects::TreeSpriteFX->Light3Tech;
    switch ( mRenderOptions )
    {
    case RenderOptions::Lighting:
        tech = Effects::TreeSpriteFX->Light3Tech;
        break;
    case RenderOptions::Textures:
        tech = Effects::TreeSpriteFX->Light3TexAlphaClipTech;
        break;
    case RenderOptions::TexturesAndFog:
        tech = Effects::TreeSpriteFX->Light3TexAlphaClipFogTech;
        break;
    }

    D3DX11_TECHNIQUE_DESC desc;
    tech->GetDesc( &desc );
    for ( UINT p = 0; p < desc.Passes; ++p ) {
        mD3DImmediateContext->IASetVertexBuffers( 0,
                                                  1,
                                                  &mTreeSpritesVB,
                                                  &stride,
                                                  &offset );

        float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };

        if ( mAlphaToCoverageOn ) {
            mD3DImmediateContext->OMSetBlendState( RenderStates::AlphaToCoverageBS,
                                                   blendFactor,
                                                   0xffffffff );
        }

        tech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->Draw( TreeCount, 0 );

        mD3DImmediateContext->OMSetBlendState( nullptr, blendFactor, 0xffffffff );
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //