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
#include "Vertex.h"

#include "DirectXCollision.h"

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

using namespace DirectX;

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

struct InstancedData {
    XMFLOAT4X4 World;
    XMFLOAT4 Color;
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

    void BuildSkullGeometryBuffers();
    void BuildInstancedBuffer();

private:

    ID3D11Buffer* mSkullVB;
    ID3D11Buffer* mSkullIB;
    ID3D11Buffer* mInstancedBuffer;

    // Bounding box of the skull.
    BoundingBox mSkullBox;
    BoundingFrustum mCamFrustum;

    UINT mVisibleObjectCount;

    // Keep a system memory copy of the world matrices for culling.
    std::vector<InstancedData> mInstancedData;

    bool mFrustumCullingEnabled;

    DirectionalLight mDirLights[3];
    Material mSkullMat;

    // Define transformations from local spaces to world space.
    XMFLOAT4X4 mSkullWorld;

    UINT mSkullIndexCount;

    Camera mCam;

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
    : D3DApp( hInstance ), mSkullVB( 0 ), mSkullIB( 0 ), mSkullIndexCount( 0 ), mInstancedBuffer( 0 ),
    mVisibleObjectCount( 0 ), mFrustumCullingEnabled( true )
{
    mMainWindowCaption = L"Instancing and Culling Demo";

    srand( (unsigned int)time( (time_t *)NULL ) );

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition( 0.0f, 2.0f, -15.0f );

    XMMATRIX I = XMMatrixIdentity();

    XMMATRIX skullScale = XMMatrixScaling( 0.5f, 0.5f, 0.5f );
    XMMATRIX skullOffset = XMMatrixTranslation( 0.0f, 1.0f, 0.0f );
    XMStoreFloat4x4( &mSkullWorld, XMMatrixMultiply( skullScale, skullOffset ) );


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

    mSkullMat.ambient = XMFLOAT4( 0.4f, 0.4f, 0.4f, 1.0f );
    mSkullMat.diffuse = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mSkullMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    ReleaseCOM( mSkullVB );
    ReleaseCOM( mSkullIB );
    ReleaseCOM( mInstancedBuffer );

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

    BuildSkullGeometryBuffers();
    BuildInstancedBuffer();

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onResize( void )
{
    D3DApp::onResize();

    mCam.SetLens( 0.25f * MathHelper::Pi, getAspectRatio(), 1.f, 1000.f );

    // In view space, build the frustum from the projection matrix.
    BoundingFrustum::CreateFromMatrix( mCamFrustum, mCam.Proj() );
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
    if ( GetAsyncKeyState( '1' ) & 0x8000 )
        mFrustumCullingEnabled = true;
    else if ( GetAsyncKeyState( '2' ) & 0x8000 )
        mFrustumCullingEnabled = false;

    //
    // Perform frustum culling.

    mCam.UpdateViewMatrix();
    mVisibleObjectCount = 0;

    if ( mFrustumCullingEnabled ) {
        XMVECTOR detView = XMMatrixDeterminant( mCam.View() );
        XMMATRIX invView = XMMatrixInverse( &detView, mCam.View() );

        D3D11_MAPPED_SUBRESOURCE mappedData;
        mD3DImmediateContext->Map( mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );

        InstancedData* dataView = reinterpret_cast<InstancedData*>( mappedData.pData );

        for ( UINT i = 0; i < mInstancedData.size(); ++i ) {
            XMMATRIX W = XMLoadFloat4x4( &mInstancedData[i].World );
            XMMATRIX invWorld = XMMatrixInverse( &XMMatrixDeterminant( W ), W );

            // View space to local space.
            XMMATRIX toLocal = XMMatrixMultiply( invView, invWorld );

            // Decompose the matrix into individual parts.
            XMVECTOR scale, rotation, translation;
            XMMatrixDecompose( &scale, &rotation, &translation, toLocal );

            // Transform the camera frustum from view space to object's local space.
            BoundingFrustum localFrust;
            mCamFrustum.Transform( localFrust, XMVectorGetX( scale ), rotation, translation );

            // Test intersection of frustum with box.
            if ( localFrust.Contains( mSkullBox ) ) {
                dataView[mVisibleObjectCount++] = mInstancedData[i];
            }
        }

        mD3DImmediateContext->Unmap( mInstancedBuffer, 0 );
    }
    else {
        D3D11_MAPPED_SUBRESOURCE mappedData;
        mD3DImmediateContext->Map( mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );

        InstancedData* dataView = reinterpret_cast<InstancedData*>( mappedData.pData );

        for ( UINT i = 0; i < mInstancedData.size(); ++i )
        {
            dataView[mVisibleObjectCount++] = mInstancedData[i];
        }

        mD3DImmediateContext->Unmap( mInstancedBuffer, 0 );
    }

    std::wostringstream outs;
    outs.precision( 6 );
    outs << L"Instancing and Culling Demo" <<
        L"    " << mVisibleObjectCount <<
        L" objects visible out of " << mInstancedData.size();
    mMainWindowCaption = outs.str();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    // We're drawing instanced geometry.
    mD3DImmediateContext->IASetInputLayout( InputLayouts::InstancedBasic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT stride[2] = { sizeof( Vertex::Basic32 ), sizeof( InstancedData ) };
    UINT offset[2] = { 0,0 };

    ID3D11Buffer* vbs[2] = { mSkullVB, mInstancedBuffer };

    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    XMMATRIX viewProj = mCam.ViewProj();

    // Set per frame constants.
    Effects::InstancedBasicFX->SetDirLights( mDirLights );
    Effects::InstancedBasicFX->SetEyePosW( mCam.GetPosition() );

    ID3DX11EffectTechnique* tech = Effects::InstancedBasicFX->Light3Tech;

    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        // Draw the skull.

        mD3DImmediateContext->IASetVertexBuffers( 0, 2, vbs, stride, offset );
        mD3DImmediateContext->IASetIndexBuffer( mSkullIB, DXGI_FORMAT_R32_UINT, 0 );

        XMMATRIX world = XMLoadFloat4x4( &mSkullWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );

        Effects::InstancedBasicFX->SetWorld( world );
        Effects::InstancedBasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::InstancedBasicFX->SetViewProj( viewProj );
        Effects::InstancedBasicFX->SetMaterial( mSkullMat );

        tech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexedInstanced( mSkullIndexCount, mVisibleObjectCount, 0, 0, 0 );
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

    XMFLOAT3 vMinf3( +MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity );
    XMFLOAT3 vMaxf3( -MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity );

    XMVECTOR vMin = XMLoadFloat3( &vMinf3 );
    XMVECTOR vMax = XMLoadFloat3( &vMaxf3 );
    std::vector<Vertex::Basic32> vertices( vcount );
    for ( UINT i = 0; i < vcount; ++i )
    {
        fin >> vertices[i].pos.x >> vertices[i].pos.y >> vertices[i].pos.z;
        fin >> vertices[i].normal.x >> vertices[i].normal.y >> vertices[i].normal.z;

        XMVECTOR P = XMLoadFloat3( &vertices[i].pos );

        vMin = XMVectorMin( vMin, P );
        vMax = XMVectorMax( vMax, P );
    }

    XMStoreFloat3( &mSkullBox.Center, 0.5f*( vMin + vMax ) );
    XMStoreFloat3( &mSkullBox.Extents, 0.5f*( vMax - vMin ) );

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

void App::BuildInstancedBuffer()
{
    const int n = 5;
    mInstancedData.resize( n*n*n );

    float width = 200.0f;
    float height = 200.0f;
    float depth = 200.0f;

    float x = -0.5f*width;
    float y = -0.5f*height;
    float z = -0.5f*depth;
    float dx = width / ( n - 1 );
    float dy = height / ( n - 1 );
    float dz = depth / ( n - 1 );
    for ( int k = 0; k < n; ++k )
    {
        for ( int i = 0; i < n; ++i )
        {
            for ( int j = 0; j < n; ++j )
            {
                // Position instanced along a 3D grid.
                mInstancedData[k*n*n + i*n + j].World = XMFLOAT4X4(
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    x + j*dx, y + i*dy, z + k*dz, 1.0f );

                // Random color.
                mInstancedData[k*n*n + i*n + j].Color.x = MathHelper::RandF( 0.0f, 1.0f );
                mInstancedData[k*n*n + i*n + j].Color.y = MathHelper::RandF( 0.0f, 1.0f );
                mInstancedData[k*n*n + i*n + j].Color.z = MathHelper::RandF( 0.0f, 1.0f );
                mInstancedData[k*n*n + i*n + j].Color.w = 1.0f;
            }
        }
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof( InstancedData ) * mInstancedData.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbd.MiscFlags = 0;
    vbd.StructureByteStride = 0;

    HR( mD3DDevice->CreateBuffer( &vbd, 0, &mInstancedBuffer ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //