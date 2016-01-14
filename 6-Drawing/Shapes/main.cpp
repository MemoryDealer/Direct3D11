// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"

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

    const float getHeight( const float x, const float z ) const;
    void buildGeometryBuffers( void );
    void buildFX( void );
    void buildVertexLayout( void );

private:

    ID3D11Buffer* mVB;
    ID3D11Buffer* mIB;

    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;

    ID3D11InputLayout* mInputLayout;

    ID3D11RasterizerState* mWireframeRS;

    DirectX::XMFLOAT4X4 mSphereWorld[10],
        mCylWorld[10],
        mBoxWorld,
        mGridWorld,
        mCenterSphere;
    DirectX::XMFLOAT4X4 mView, mProj;

    int mBoxVertexOffset,
        mGridVertexOffset,
        mSphereVertexOffset,
        mCylinderVertexOffset;

    UINT mBoxIndexOffset,
        mGridIndexOffset,
        mSphereIndexOffset,
        mCylinderIndexOffset;

    UINT mBoxIndexCount,
        mGridIndexCount,
        mSphereIndexCount,
        mCylinderIndexCount;

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
    , mVB( nullptr )
    , mIB( nullptr )
    , mFX( nullptr )
    , mTech( nullptr )
    , mfxWorldViewProj( nullptr )
    , mInputLayout( nullptr )
    , mWireframeRS( nullptr )
    , mTheta( 1.5f * MathHelper::Pi )
    , mPhi( 0.1f * MathHelper::Pi )
    , mRadius( 15.f )
{
    mMainWindowCaption = L"Shapes Demo";

    ZeroMemory( &mLastMousePos, sizeof( POINT ) );

    DirectX::XMMATRIX I = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4( &mGridWorld, I );
    DirectX::XMStoreFloat4x4( &mView, I );
    DirectX::XMStoreFloat4x4( &mProj, I );

    // Setup shape matrices.
    DirectX::XMMATRIX boxScale = DirectX::XMMatrixScaling( 2.f, 1.f, 2.f );
    DirectX::XMMATRIX boxOffset = DirectX::XMMatrixTranslation( 0.f, 0.5f, 0.f );
    DirectX::XMStoreFloat4x4( &mBoxWorld, DirectX::XMMatrixMultiply( boxScale, boxOffset ) );

    DirectX::XMMATRIX centerSphereScale = DirectX::XMMatrixScaling( 2.f, 2.f, 2.f );
    DirectX::XMMATRIX centerSphereOffset = DirectX::XMMatrixTranslation( 0.f, 2.f, 0.f );
    DirectX::XMStoreFloat4x4( &mCenterSphere, DirectX::XMMatrixMultiply( centerSphereScale, centerSphereOffset ) );

    for ( int i = 0; i < 5; ++i ) {
        DirectX::XMStoreFloat4x4( &mCylWorld[i * 2 + 0], DirectX::XMMatrixTranslation( -5.f, 1.5f, -10.f + i*5.f ) );
        DirectX::XMStoreFloat4x4( &mCylWorld[i * 2 + 1], DirectX::XMMatrixTranslation( +5.f, 1.5f, -10.f + i*5.f ) );

        DirectX::XMStoreFloat4x4( &mSphereWorld[i * 2 + 0], DirectX::XMMatrixTranslation( -5.f, 3.5f, -10.f + i * 5.f ) );
        DirectX::XMStoreFloat4x4( &mSphereWorld[i * 2 + 1], DirectX::XMMatrixTranslation( +5.f, 3.5f, -10.f + i * 5.f ) );
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    ReleaseCOM( mVB );
    ReleaseCOM( mIB );
    ReleaseCOM( mFX );
    ReleaseCOM( mInputLayout );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() ) {
        return false;
    }

    buildGeometryBuffers();
    buildFX();
    buildVertexLayout();

    D3D11_RASTERIZER_DESC wireframeDesc;
    ZeroMemory( &wireframeDesc, sizeof( wireframeDesc ) );
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

    mD3DImmediateContext->RSSetState( mWireframeRS );

    UINT stride = sizeof( Vertex );
    UINT offset = 0;
    mD3DImmediateContext->IASetVertexBuffers( 0,
                                              1,
                                              &mVB,
                                              &stride,
                                              &offset );
    mD3DImmediateContext->IASetIndexBuffer( mIB,
                                            DXGI_FORMAT_R32_UINT,
                                            0 );

    // Get a view-projection matrix for the scene to later get the 
    // world-view-projection matrix of each object.
    DirectX::XMMATRIX view = XMLoadFloat4x4( &mView );
    DirectX::XMMATRIX proj = XMLoadFloat4x4( &mProj );
    DirectX::XMMATRIX viewProj = view * proj;
    
    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        // Draw grid.
        DirectX::XMMATRIX world = XMLoadFloat4x4( &mGridWorld );
        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &( world * viewProj ) ) );
        mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mGridIndexCount, mGridIndexOffset, mGridVertexOffset );

        // Draw box.
        world = XMLoadFloat4x4( &mBoxWorld );
        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &( world * viewProj ) ) );
        mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset );

        // Draw sphere.
        world = XMLoadFloat4x4( &mCenterSphere );
        mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &( world * viewProj ) ) );
        mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );

        // Cylinders and spheres.
        for ( int i = 0; i < 10; ++i ) {
            world = XMLoadFloat4x4( &mCylWorld[i] );
            mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &( world * viewProj ) ) );
            mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset );
        }

        for ( int i = 0; i < 10; ++i ) {
            world = XMLoadFloat4x4( &mSphereWorld[i] );
            mfxWorldViewProj->SetMatrix( reinterpret_cast<float*>( &( world * viewProj ) ) );
            mTech->GetPassByIndex( 0 )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );
        }
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

const float App::getHeight( const float x, const float z ) const
{
    return 0.3f * ( z * std::sinf( 0.1f * x ) + x * std::cosf( 0.1f * z ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::buildGeometryBuffers( void )
{
    GeometryGenerator::MeshData box, grid, sphere, cylinder;
    GeometryGenerator geoGen;

    geoGen.createBox( 1.f, 1.f, 1.f, box );
    geoGen.createGrid( 20.f, 30.f, 60, 40, grid );
    geoGen.createSphere( 0.5f, 20, 20, sphere );
    geoGen.createCylinder( 0.5f, 0.3f, 3.f, 20, 20, cylinder );

    mBoxVertexOffset = 0;
    mGridVertexOffset = box.vertices.size();
    mSphereVertexOffset = mGridVertexOffset + grid.vertices.size();
    mCylinderVertexOffset = mSphereVertexOffset + sphere.vertices.size();

    mBoxIndexCount = static_cast<UINT>( box.indices.size() );
    mGridIndexCount = static_cast<UINT>( grid.indices.size() );
    mSphereIndexCount = static_cast<UINT>( sphere.indices.size() );
    mCylinderIndexCount = static_cast<UINT>( cylinder.indices.size() );

    mBoxIndexOffset = 0;
    mGridIndexOffset = mBoxIndexCount;
    mSphereIndexOffset = mGridIndexOffset + mGridIndexCount;
    mCylinderIndexOffset = mSphereIndexOffset + mSphereIndexCount;

    const UINT totalVertexCount = box.vertices.size() +
        grid.vertices.size() +
        sphere.vertices.size() +
        cylinder.vertices.size();
    const UINT totalIndexCount = mBoxIndexCount +
        mGridIndexCount +
        mSphereIndexCount +
        mCylinderIndexCount;

    // Pack all vertices into one vertex buffer.
    std::vector<Vertex> vertices( totalVertexCount );

    const DirectX::XMFLOAT4 black( 0.f, 0.f, 0.f, 1.f );
    UINT k = 0;
    for ( size_t i = 0; i < box.vertices.size(); ++i, ++k ) {
        vertices[k].pos = box.vertices[i].position;
        vertices[k].color = black;
    }
    for ( size_t i = 0; i < grid.vertices.size(); ++i, ++k ) {
        vertices[k].pos = grid.vertices[i].position;
        vertices[k].color = black;
    }
    for ( size_t i = 0; i < sphere.vertices.size(); ++i, ++k ) {
        vertices[k].pos = sphere.vertices[i].position;
        vertices[k].color = black;
    }
    for ( size_t i = 0; i < cylinder.vertices.size(); ++i, ++k ) {
        vertices[k].pos = cylinder.vertices[i].position;
        vertices[k].color = black;
    }
    

    D3D11_BUFFER_DESC bd;
    ZeroMemory( &bd, sizeof( bd ) );
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof( Vertex ) * totalVertexCount;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    bd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory( &initData, sizeof( initData ) );
    initData.pSysMem = &vertices[0];
    HR( mD3DDevice->CreateBuffer( &bd, &initData, &mVB ) );

    std::vector<UINT> indices;
    indices.insert( indices.end(), box.indices.begin(), box.indices.end() );
    indices.insert( indices.end(), grid.indices.begin(), grid.indices.end() );
    indices.insert( indices.end(), sphere.indices.begin(), sphere.indices.end() );
    indices.insert( indices.end(), cylinder.indices.begin(), cylinder.indices.end() );

    D3D11_BUFFER_DESC ibd;
    ZeroMemory( &ibd, sizeof( ibd ) );
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    ibd.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mIB ) );
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