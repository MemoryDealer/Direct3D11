// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "Waves.h"

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

using namespace DirectX;

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT3 normal;
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

    void buildFX( void );
    void buildVertexLayout( void );

    const float getHillHeight( const float x, const float z ) const;
    const DirectX::XMFLOAT3 getHillNormal( const float x, const float z ) const;
    void buildLandBuffers( void );
    void buildWaveBuffers( void );

private:

    ID3D11Buffer* mLandVB;
    ID3D11Buffer* mLandIB;
    ID3D11Buffer* mWavesVB;
    ID3D11Buffer* mWavesIB;

    Waves mWaves;
    DirectionalLight mDirLight;
    PointLight mPointLight;
    SpotLight mSpotLight;
    Material mLandMat, mWavesMat;

    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;
    ID3DX11EffectMatrixVariable* mfxWorld;
    ID3DX11EffectMatrixVariable* mfxWorldInvTranspose;
    ID3DX11EffectVectorVariable* mfxEyePosW;
    ID3DX11EffectVariable* mfxDirLight;
    ID3DX11EffectVariable* mfxPointLight;
    ID3DX11EffectVariable* mfxSpotLight;
    ID3DX11EffectVariable* mfxMaterial;

    ID3D11InputLayout* mInputLayout;

    DirectX::XMFLOAT4X4 mLandWorld, mWavesWorld, mView, mProj;

    UINT mGridIndexCount;

    DirectX::XMFLOAT3 mEyePosW;

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
    , mLandVB( nullptr )
    , mLandIB( nullptr )
    , mWavesVB( nullptr )
    , mWavesIB( nullptr )
    , mFX( nullptr )
    , mTech( nullptr )
    , mfxWorldViewProj( nullptr )
    , mfxWorld( nullptr )
    , mfxWorldInvTranspose( nullptr )
    , mfxEyePosW( nullptr )
    , mfxDirLight( nullptr )
    , mfxPointLight( nullptr )
    , mfxSpotLight( nullptr )
    , mfxMaterial( nullptr )
    , mInputLayout( nullptr )
    , mEyePosW( 0.f, 0.f, 0.f )
    , mTheta( 1.5f * MathHelper::Pi )
    , mPhi( 0.1f * MathHelper::Pi )
    , mRadius( 200.0f )
{
    mMainWindowCaption = L"Waves Demo";

    ZeroMemory( &mLastMousePos, sizeof( POINT ) );

    DirectX::CXMMATRIX I = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4( &mLandWorld, I );
    DirectX::XMStoreFloat4x4( &mWavesWorld, I );
    DirectX::XMStoreFloat4x4( &mView, I );
    DirectX::XMStoreFloat4x4( &mProj, I );

    XMMATRIX wavesOffset = XMMatrixTranslation( 0.0f, -3.0f, 0.0f );
    XMStoreFloat4x4( &mWavesWorld, wavesOffset );

    // Directional light.
    mDirLight.ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLight.diffuse = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mDirLight.specular = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mDirLight.direction = XMFLOAT3( 0.57735f, -0.57735f, 0.57735f );

    // Point light--position is changed every frame to animate in UpdateScene function.
    mPointLight.ambient = XMFLOAT4( 0.3f, 0.3f, 0.3f, 1.0f );
    mPointLight.diffuse = XMFLOAT4( 0.7f, 0.7f, 0.7f, 1.0f );
    mPointLight.specular = XMFLOAT4( 0.7f, 0.7f, 0.7f, 1.0f );
    mPointLight.att = XMFLOAT3( 0.0f, 0.1f, 0.0f );
    mPointLight.range = 25.0f;

    // Spot light--position and direction changed every frame to animate in UpdateScene function.
    mSpotLight.ambient = XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f );
    mSpotLight.diffuse = XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f );
    mSpotLight.specular = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mSpotLight.att = XMFLOAT3( 1.0f, 0.0f, 0.0f );
    mSpotLight.spot = 96.0f;
    mSpotLight.range = 10000.0f;

    mLandMat.ambient = XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
    mLandMat.diffuse = XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
    mLandMat.specular = XMFLOAT4( 0.2f, 0.2f, 0.2f, 16.0f );

    mWavesMat.ambient = XMFLOAT4( 0.137f, 0.42f, 0.556f, 1.0f );
    mWavesMat.diffuse = XMFLOAT4( 0.137f, 0.42f, 0.556f, 1.0f );
    mWavesMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 96.0f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    ReleaseCOM( mLandVB );
    ReleaseCOM( mLandIB );
    ReleaseCOM( mWavesVB );
    ReleaseCOM( mWavesIB );
    ReleaseCOM( mFX );
    ReleaseCOM( mInputLayout );    
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() ) {
        return false;
    }

    mWaves.Init( 160, 160, 1.f, 0.03f, 3.25f, 0.4f );

    buildLandBuffers();
    buildWaveBuffers();
    buildFX();
    buildVertexLayout();

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

    // Generate a random wave every 0.25 seconds.
    static float t_base = 0.f;
    if ( ( mTimer.totalTime() - t_base ) >= 0.25f ) {
        t_base += 0.25f;

        DWORD i = 5 + rand() % ( mWaves.RowCount() - 10 );
        DWORD j = 5 + rand() % ( mWaves.ColumnCount() - 10 );

        float r = MathHelper::RandF( 1.f, 2.f );

        mWaves.Disturb( i, j, r );
    }

    mWaves.Update( dt );

    // Update wave vertex buffer.
    D3D11_MAPPED_SUBRESOURCE mappedData;
    HR( mD3DImmediateContext->Map( mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData ) );

    Vertex* v = reinterpret_cast<Vertex*>( mappedData.pData );
    for ( UINT i = 0; i < mWaves.VertexCount(); ++i ) {
        v[i].pos = mWaves[i];
        v[i].normal = mWaves.Normal( i );
    }

    mD3DImmediateContext->Unmap( mWavesVB, 0 );

    // Animate lights.
    mPointLight.position.x = 70.f * cosf( 0.2f * mTimer.totalTime() );
    mPointLight.position.z = 70.f * sinf( 0.2f * mTimer.totalTime() );
    mPointLight.position.y = MathHelper::Max( getHillHeight( mPointLight.position.x,
                                                             mPointLight.position.z ),
                                              -3.f ) + 10.f;

    mSpotLight.position = mEyePosW;
    XMStoreFloat3( &mSpotLight.direction, XMVector3Normalize( target - pos ) );
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

    mD3DImmediateContext->IASetInputLayout( mInputLayout );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT stride = sizeof( Vertex );
    UINT offset = 0;

    // Set constant buffers.
    DirectX::XMMATRIX view = XMLoadFloat4x4( &mView );
    DirectX::XMMATRIX proj = XMLoadFloat4x4( &mProj );

    mfxDirLight->SetRawValue( &mDirLight, 0, sizeof( mDirLight ) );
    mfxPointLight->SetRawValue( &mPointLight, 0, sizeof( mPointLight ) );
    mfxSpotLight->SetRawValue( &mSpotLight, 0, sizeof( mSpotLight ) );
    mfxEyePosW->SetRawValue( &mEyePosW, 0, sizeof( mEyePosW ) );

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        // Draw land.
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mLandVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mLandIB, DXGI_FORMAT_R32_UINT, 0 );

        XMMATRIX world = DirectX::XMLoadFloat4x4( &mLandWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world * view * proj;

        mfxWorld->SetMatrix( reinterpret_cast<float*>( &world ) );
        mfxWorldInvTranspose->SetMatrix( reinterpret_cast<float*>( &worldInvTranspose ) );
        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &worldViewProj ) );
        mfxMaterial->SetRawValue( &mLandMat, 0, sizeof( mLandMat ) );

        mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mGridIndexCount, 0, 0 );

        // Draw waves.

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mWavesVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mWavesIB, DXGI_FORMAT_R32_UINT, 0 );

        world = DirectX::XMLoadFloat4x4( &mWavesWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world * view * proj;

        mfxWorld->SetMatrix( reinterpret_cast<float*>( &world ) );
        mfxWorldInvTranspose->SetMatrix( reinterpret_cast<float*>( &worldInvTranspose ) );
        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &worldViewProj ) );
        mfxMaterial->SetRawValue( &mWavesMat, 0, sizeof( mWavesMat ) );

        mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( 3 * mWaves.TriangleCount(), 0, 0 );

        // Restore default.
        mD3DImmediateContext->RSSetState( nullptr );
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
        float dx = DirectX::XMConvertToRadians( 0.25f * static_cast<float>( x - mLastMousePos.x ) );
        float dy = DirectX::XMConvertToRadians( 0.25f * static_cast<float>( y - mLastMousePos.y ) );

        // Update spherical coordinates for angles.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle phi.
        mPhi = MathHelper::Clamp( mPhi, 0.1f, MathHelper::Pi - 0.1f );
    }
    else if ( ( btnState & MK_RBUTTON ) != 0 ) {
        // Correspond each pixel to 0.005 units in the scene.
        float dx = 0.2f * static_cast<float>( x - mLastMousePos.x );
        float dy = 0.2f * static_cast<float>( y - mLastMousePos.y );

        // Update camera radius.
        mRadius += dx - dy;

        // Restrict radius.
        mRadius = MathHelper::Clamp( mRadius, 50.f, 500.f );
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

const float App::getHillHeight( const float x, const float z ) const
{
    return 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

const XMFLOAT3 App::getHillNormal( const float x, const float z ) const
{
    // n = ( -df/dx, 1, -df/dz )
    XMFLOAT3 n(
        -0.03f * z * cosf( 0.1f*x ) - 0.3f*cosf( 0.1f*z ),
        1.f,
        -0.3f*sinf( 0.1f*x ) + -0.03f*x*sinf( 0.1f*z ) );

    XMVECTOR unitNormal = XMVector3Normalize( XMLoadFloat3( &n ) );
    XMStoreFloat3( &n, unitNormal );
    
    return n;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildFX( void )
{
    DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined ( _DEBUG )   
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* compiledShader = nullptr;
    ID3DBlob* compilationMsgs = nullptr;
    /*HRESULT hr = D3DX11CompileEffectFromFile( L"FX/color.fx",
                                        nullptr,
                                        D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                        shaderFlags,
                                        0,
                                        mD3DDevice,
                                        &mFX,
                                        &compilationMsgs );*/
    HRESULT hr = D3DCompileFromFile( L"FX/Lighting.fx",
                                     nullptr,
                                     D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                     nullptr,
                                     "fx_5_0",
                                     shaderFlags,
                                     0,
                                     &compiledShader,
                                     &compilationMsgs );

    if ( compilationMsgs != nullptr ) {
        // Filter out warning about deprecated compiler since effects are being used.
        if ( std::string("warning X4717: Effects deprecated for D3DCompiler_47").compare( 
            static_cast<const char*>( compilationMsgs->GetBufferPointer() ) ) == 0 ) {
            MessageBoxA( 0,
                         static_cast<char*>( compilationMsgs->GetBufferPointer() ),
                         nullptr,
                         0 );
        }
        ReleaseCOM( compilationMsgs );
    }

    // Check for other errors.
    if ( FAILED( hr ) ) {
        DXTrace( __FILEW__,
        static_cast<DWORD>( __LINE__ ),
        hr,
        L"D3DX11CompileFromFile",
        true );
    }

    hr = D3DX11CreateEffectFromMemory( compiledShader->GetBufferPointer(),
                                      compiledShader->GetBufferSize(),
                                      0,
                                      mD3DDevice,
                                      &mFX );
    if ( FAILED( hr ) ) {
        DXTrace( __FILEW__,
                 static_cast<DWORD>( __LINE__ ),
                 hr,
                 L"D3DX11CreateEffectFromMemory",
                 true );
    }

    ReleaseCOM( compiledShader );

    mTech = mFX->GetTechniqueByName( "LightTech" );
    // Store a pointer to constant buffer gWorldViewProj.
    mfxWorldViewProj = mFX->GetVariableByName( "gWorldViewProj" )->AsMatrix();
    mfxWorld = mFX->GetVariableByName( "gWorld" )->AsMatrix();
    mfxWorldInvTranspose = mFX->GetVariableByName( "gWorldInvTranspose" )->AsMatrix();
    mfxEyePosW = mFX->GetVariableByName( "gEyePosW" )->AsVector();
    mfxDirLight = mFX->GetVariableByName( "gDirLight" );
    mfxPointLight = mFX->GetVariableByName( "gPointLight" );
    mfxSpotLight = mFX->GetVariableByName( "gSpotLight" );
    mfxMaterial = mFX->GetVariableByName( "gMaterial" );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildVertexLayout( void )
{
    // Create vertex input layout.
    D3D11_INPUT_ELEMENT_DESC vertexDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3DX11_PASS_DESC passDesc;
    mTech->GetPassByIndex( 0 )->GetDesc( &passDesc );
    HR( mD3DDevice->CreateInputLayout( vertexDesc,
                                       2,
                                       passDesc.pIAInputSignature,
                                       passDesc.IAInputSignatureSize,
                                       &mInputLayout ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildLandBuffers( void )
{
    GeometryGenerator::MeshData grid;

    GeometryGenerator gen;
    gen.createGrid( 160.f, 160.f, 50, 50, grid );
    mGridIndexCount = grid.indices.size();

    // Build land.
    std::vector<Vertex> vertices( grid.vertices.size() );
    for ( size_t i = 0; i < grid.vertices.size(); ++i )
    {
        DirectX::XMFLOAT3 p = grid.vertices[i].position;

        p.y = getHillHeight( p.x, p.z );

        vertices[i].pos = p;
        vertices[i].normal = getHillNormal( p.x, p.z );
    }

    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof( Vertex ) * grid.vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory( &initData, sizeof( initData ) );
    initData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &bd, &initData, &mLandVB ) );

    D3D11_BUFFER_DESC ibd;
    ZeroMemory( &ibd, sizeof( ibd ) );
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * mGridIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &grid.indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mLandIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildWaveBuffers( void )
{
    D3D11_BUFFER_DESC vbd;
    ZeroMemory( &vbd, sizeof( vbd ) );
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof( Vertex ) * mWaves.VertexCount();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HR( mD3DDevice->CreateBuffer( &vbd, nullptr, &mWavesVB ) );

    std::vector<UINT> indices( 3 * mWaves.TriangleCount() );

    // Iterate over each quad.
    UINT m = mWaves.RowCount();
    UINT n = mWaves.ColumnCount();
    int k = 0;
    for ( UINT i = 0; i < m - 1; ++i ) {
        for ( DWORD j = 0; j < n - 1; ++j ) {
            indices[k] = i * n + j;
            indices[k + 1] = i * n + j + 1;
            indices[k + 2] = ( i + 1 ) * n + j;

            indices[k + 3] = ( i + 1 )*n + j;
            indices[k + 4] = i * n + j + 1;
            indices[k + 5] = ( i + 1 ) * n + j + 1;

            k += 6;
        }
    }

    D3D11_BUFFER_DESC ibd;
    ZeroMemory( &ibd, sizeof( ibd ) );
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