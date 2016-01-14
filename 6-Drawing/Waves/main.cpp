// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "Waves.h"

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

struct Vertex {
    DirectX::XMFLOAT3 pos;
    DirectX::XMFLOAT4 color;
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

    const float getHeight( const float x, const float z ) const;
    void buildLandBuffers( void );
    void buildWaveBuffers( void );

private:

    ID3D11Buffer* mLandVB;
    ID3D11Buffer* mLandIB;
    ID3D11Buffer* mWavesVB;
    ID3D11Buffer* mWavesIB;

    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;

    ID3D11InputLayout* mInputLayout;

    ID3D11RasterizerState* mWireframeRS;

    DirectX::XMFLOAT4X4 mGridWorld, mWavesWorld, mView, mProj;

    UINT mGridIndexCount;

    Waves mWaves;

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
    , mInputLayout( nullptr )
    , mWireframeRS( nullptr )
    , mTheta( 1.5f * MathHelper::Pi )
    , mPhi( 0.1f * MathHelper::Pi )
    , mRadius( 200.0f )
{
    mMainWindowCaption = L"Waves Demo";

    ZeroMemory( &mLastMousePos, sizeof( POINT ) );

    DirectX::CXMMATRIX I = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4( &mGridWorld, I );
    DirectX::XMStoreFloat4x4( &mWavesWorld, I );
    DirectX::XMStoreFloat4x4( &mView, I );
    DirectX::XMStoreFloat4x4( &mProj, I );
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
    ReleaseCOM( mWireframeRS );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() ) {
        return false;
    }

    mWaves.Init( 200, 200, 0.8f, 0.03f, 3.25f, 0.4f );

    buildLandBuffers();
    buildWaveBuffers();
    buildFX();
    buildVertexLayout();

    D3D11_RASTERIZER_DESC wireframeDesc;
    ZeroMemory( &wireframeDesc, sizeof( D3D11_RASTERIZER_DESC ) );
    wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireframeDesc.CullMode = D3D11_CULL_BACK;
    wireframeDesc.FrontCounterClockwise = false;
    wireframeDesc.DepthClipEnable = true;

    HR( mD3DDevice->CreateRasterizerState( &wireframeDesc, &mWireframeRS ) );

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onResize( void )
{
    D3DApp::onResize();

    // Update the aspect ratio and recompute the projection matrix.
    DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH( 0.25f * MathHelper::Pi,
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

        DWORD i = 5 + rand() % 190;
        DWORD j = 5 + rand() % 190;

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
        v[i].color = DirectX::XMFLOAT4( 0.f, 0.f, 0.f, 1.f );
    }

    mD3DImmediateContext->Unmap( mWavesVB, 0 );
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

    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        // Draw land.
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mLandVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mLandIB, DXGI_FORMAT_R32_UINT, 0 );

        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4( &mGridWorld );
        DirectX::XMMATRIX worldViewProj = world * view * proj;

        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &worldViewProj ) );

        mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mGridIndexCount, 0, 0 );

        // Draw waves.
        mD3DImmediateContext->RSSetState( mWireframeRS );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mWavesVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mWavesIB, DXGI_FORMAT_R32_UINT, 0 );

        world = DirectX::XMLoadFloat4x4( &mWavesWorld );
        worldViewProj = world * view * proj;
        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &worldViewProj ) );

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

const float App::getHeight( const float x, const float z ) const
{
    return 0.3f*( z*sinf( 0.1f*x ) + x*cosf( 0.1f*z ) );
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
    HRESULT hr = D3DCompileFromFile( L"FX/color.fx",
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
        if ( std::strstr( static_cast<const char*>( compilationMsgs->GetBufferPointer() ), 
                          "X4717" ) == 0 ) {
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

    mTech = mFX->GetTechniqueByName( "ColorTech" );
    // Store a pointer to constant buffer gWorldViewProj.
    mfxWorldViewProj = mFX->GetVariableByName( "gWorldViewProj" )->AsMatrix();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildVertexLayout( void )
{
    // Create vertex input layout.
    D3D11_INPUT_ELEMENT_DESC vertexDesc [] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

        p.y = getHeight( p.x, p.z );

        vertices[i].pos = p;

        // Color the vertex based on its height.
        if ( p.y < -10.0f )
        {
            // Sandy beach color.
            vertices[i].color = DirectX::XMFLOAT4( 1.0f, 0.96f, 0.62f, 1.0f );
        }
        else if ( p.y < 5.0f )
        {
            // Light yellow-green.
            vertices[i].color = DirectX::XMFLOAT4( 0.48f, 0.77f, 0.46f, 1.0f );
        }
        else if ( p.y < 12.0f )
        {
            // Dark yellow-green.
            vertices[i].color = DirectX::XMFLOAT4( 0.1f, 0.48f, 0.19f, 1.0f );
        }
        else if ( p.y < 20.0f )
        {
            // Dark brown.
            vertices[i].color = DirectX::XMFLOAT4( 0.45f, 0.39f, 0.34f, 1.0f );
        }
        else
        {
            // White snow.
            vertices[i].color = DirectX::XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
        }
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