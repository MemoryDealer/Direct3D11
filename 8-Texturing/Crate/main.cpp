// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
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

    void buildGeometryBuffers( void );

private:

    ID3D11Buffer* mBoxVB;
    ID3D11Buffer* mBoxIB;

    ID3D11ShaderResourceView* mDiffuseMapSRV;

    DirectionalLight mDirLights[3];
    Material mBoxMat;

    XMFLOAT4X4 mTexTransform;
    XMFLOAT4X4 mBoxWorld;

    XMFLOAT4X4 mView, mProj;

    int mBoxVertexOffset;
    UINT mBoxIndexOffset;
    UINT mBoxIndexCount;

    UINT mLightCount;

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
    : D3DApp( hInstance )
    , mBoxVB( nullptr )
    , mBoxIB( nullptr )
    , mDiffuseMapSRV( nullptr )
    , mLightCount( 1 )
    , mEyePosW( 0.f, 0.f, 0.f )
    , mTheta( 1.5f * MathHelper::Pi )
    , mPhi( 0.1f * MathHelper::Pi )
    , mRadius( 15.0f )
{
    mMainWindowCaption = L"Crate Demo";

    ZeroMemory( &mLastMousePos, sizeof( POINT ) );

    CXMMATRIX I = DirectX::XMMatrixIdentity();
    XMStoreFloat4x4( &mBoxWorld, I );
    XMStoreFloat4x4( &mTexTransform, I );
    XMStoreFloat4x4( &mView, I );
    XMStoreFloat4x4( &mProj, I );

    mBoxMat.ambient = XMFLOAT4( 0.5f, 0.5f, 0.5f, 1.0f );
    mBoxMat.diffuse = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
    mBoxMat.specular = XMFLOAT4( 0.6f, 0.6f, 0.6f, 16.0f );

    mDirLights[0].ambient = XMFLOAT4( 0.3f, 0.3f, 0.3f, 1.0f );
    mDirLights[0].diffuse = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mDirLights[0].specular = XMFLOAT4( 0.6f, 0.6f, 0.6f, 16.0f );
    mDirLights[0].direction = XMFLOAT3( 0.707f, -0.707f, 0.0f );

    mDirLights[1].ambient = XMFLOAT4( 0.2f, 0.2f, 0.2f, 1.0f );
    mDirLights[1].diffuse = XMFLOAT4( 1.4f, 1.4f, 1.4f, 1.0f );
    mDirLights[1].specular = XMFLOAT4( 0.3f, 0.3f, 0.3f, 16.0f );
    mDirLights[1].direction = XMFLOAT3( -0.707f, 0.0f, 0.707f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    ReleaseCOM( mBoxVB );
    ReleaseCOM( mBoxIB );   
    ReleaseCOM( mDiffuseMapSRV );

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() ) {
        return false;
    }

    Effects::InitAll( mD3DDevice );
    InputLayouts::InitAll( mD3DDevice );

    //HR (D3DX11CreateShader)

    buildGeometryBuffers();

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

    if ( GetAsyncKeyState( '0' ) & 0x8000 ) {
        mLightCount = 0;
    }
    else if ( GetAsyncKeyState( '1' ) & 0x8000 ) {
        mLightCount = 1;
    }
    else if ( GetAsyncKeyState( '2' ) & 0x8000 ) {
        mLightCount = 2;
    }
    else if ( GetAsyncKeyState( '3' ) & 0x8000 ) {
        mLightCount = 3;
    }
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

    mD3DImmediateContext->IASetInputLayout( InputLayouts::PosNormal );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Set constant buffers.
    XMMATRIX view = XMLoadFloat4x4( &mView );
    XMMATRIX proj = XMLoadFloat4x4( &mProj );
    XMMATRIX viewProj = view * proj;

    // Set per-frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mEyePosW );

    // Figure out which tech to use.
    ID3DX11EffectTechnique* tech = Effects::BasicFX->Light1Tech;
    switch ( mLightCount ) {
    default:
    case 1:
        break;

    case 2:
        tech = Effects::BasicFX->Light2Tech;
        break;

    case 3:
        tech = Effects::BasicFX->Light3Tech;
        break;
    }

    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );
    const UINT stride = sizeof( Vertex::PosNormal );
    const UINT offset = 0;
    for ( UINT p = 0; p < techDesc.Passes; ++p ) {
        // Bind shape VBs.
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mShapesVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mShapesIB, DXGI_FORMAT_R32_UINT, 0 );

        // Prepare to draw grid.
        // Calculate matrices.
        XMMATRIX world = XMLoadFloat4x4( &mGridWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world * view * proj;

        // Set constant buffers.
        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mGridMat );

        tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mGridIndexCount, mGridIndexOffset, mGridVertexOffset );

        // Draw box.
        world = XMLoadFloat4x4( &mBoxWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mBoxMat );

        tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
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
            Effects::BasicFX->SetMaterial( mCylinderMat );

            tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset );
        }

        // Draw the spheres.
        for ( int i = 0; i < 10; ++i )
        {
            world = XMLoadFloat4x4( &mSphereWorld[i] );
            worldInvTranspose = MathHelper::InverseTranspose( world );
            worldViewProj = world*view*proj;

            Effects::BasicFX->SetWorld( world );
            Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
            Effects::BasicFX->SetWorldViewProj( worldViewProj );
            Effects::BasicFX->SetMaterial( mSphereMat );

            tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset );
        }

        // Draw skull.
        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mSkullVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        world = XMLoadFloat4x4( &mSkullWorld );
        worldInvTranspose = MathHelper::InverseTranspose( world );
        worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mSkullMat );

        tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mSkullIndexCount, 0, 0 );
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

void App::buildShapeBuffers( void )
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

    std::vector<Vertex::PosNormal> vertices( totalVertexCount );

    UINT k = 0;
    for ( size_t i = 0; i < box.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = box.vertices[i].position;
        vertices[k].normal = box.vertices[i].normal;
    }

    for ( size_t i = 0; i < grid.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = grid.vertices[i].position;
        vertices[k].normal = grid.vertices[i].normal;
    }

    for ( size_t i = 0; i < sphere.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = sphere.vertices[i].position;
        vertices[k].normal = sphere.vertices[i].normal;
    }

    for ( size_t i = 0; i < cylinder.vertices.size(); ++i, ++k )
    {
        vertices[k].pos = cylinder.vertices[i].position;
        vertices[k].normal = cylinder.vertices[i].normal;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::PosNormal ) * totalVertexCount;
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

void App::buildSkullBuffers( void )
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

    std::vector<Vertex::PosNormal> vertices( vcount );
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
    vbd.ByteWidth = sizeof( Vertex::PosNormal ) * vcount;
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