// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "Camera.h"
#include "D3DApp.h"
#include "d3dx11Effect.h"
#include "DirectXCollision.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "LightHelper.h"
#include "MathHelper.h"
#include "RenderStates.h"
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

    void BuildMeshGeometryBuffers();
    void Pick( int sx, int sy );

private:

    ID3D11Buffer* mMeshVB;
    ID3D11Buffer* mMeshIB;

    // Keep system memory copies of the Mesh geometry for picking.
    std::vector<Vertex::Basic32> mMeshVertices;
    std::vector<UINT> mMeshIndices;

    BoundingBox mMeshBox;    

    DirectionalLight mDirLights[3];
    Material mMeshMat;
    Material mPickedTriangleMat;

    // Define transformations from local spaces to world space.
    XMFLOAT4X4 mMeshWorld;

    UINT mMeshIndexCount;

    UINT mPickedTriangle;

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
    : D3DApp( hInstance ), mMeshVB( 0 ), mMeshIB( 0 ), mMeshIndexCount( 0 ), mPickedTriangle( -1 )
{
    mMainWindowCaption = L"Picking Demo";

    mLastMousePos.x = 0;
    mLastMousePos.y = 0;

    mCam.SetPosition( 0.0f, 2.0f, -15.0f );

    XMMATRIX MeshScale = XMMatrixScaling( 0.5f, 0.5f, 0.5f );
    XMMATRIX MeshOffset = XMMatrixTranslation( 0.0f, 1.0f, 0.0f );
    XMStoreFloat4x4( &mMeshWorld, XMMatrixMultiply( MeshScale, MeshOffset ) );

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

    mMeshMat.ambient = XMFLOAT4( 0.4f, 0.4f, 0.4f, 1.0f );
    mMeshMat.diffuse = XMFLOAT4( 0.8f, 0.8f, 0.8f, 1.0f );
    mMeshMat.specular = XMFLOAT4( 0.8f, 0.8f, 0.8f, 16.0f );

    mPickedTriangleMat.ambient = XMFLOAT4( 0.0f, 0.8f, 0.4f, 1.0f );
    mPickedTriangleMat.diffuse = XMFLOAT4( 0.0f, 0.8f, 0.4f, 1.0f );
    mPickedTriangleMat.specular = XMFLOAT4( 0.0f, 0.0f, 0.0f, 16.0f );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

App::~App( void )
{
    ReleaseCOM( mMeshVB );
    ReleaseCOM( mMeshIB );

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool App::init( void )
{
    if ( !D3DApp::init() )
        return false;

    // Must init Effects first since InputLayouts depend on shader signatures.
    Effects::InitAll( mD3DDevice );
    InputLayouts::InitAll( mD3DDevice );
    RenderStates::InitAll( mD3DDevice );

    BuildMeshGeometryBuffers();

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
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::drawScene( void )
{
    mD3DImmediateContext->ClearRenderTargetView( mRenderTargetView, reinterpret_cast<const float*>( &Colors::Silver ) );
    mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    mD3DImmediateContext->IASetInputLayout( InputLayouts::Basic32 );
    mD3DImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT stride = sizeof( Vertex::Basic32 );
    UINT offset = 0;

    mCam.UpdateViewMatrix();

    XMMATRIX view = mCam.View();
    XMMATRIX proj = mCam.Proj();
    XMMATRIX viewProj = mCam.ViewProj();




    // Set per frame constants.
    Effects::BasicFX->SetDirLights( mDirLights );
    Effects::BasicFX->SetEyePosW( mCam.GetPosition() );

    ID3DX11EffectTechnique* activeMeshTech = Effects::BasicFX->Light3Tech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeMeshTech->GetDesc( &techDesc );
    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
        // Draw the Mesh.

        if ( GetAsyncKeyState( '1' ) & 0x8000 )
            mD3DImmediateContext->RSSetState( RenderStates::WireframeRS );

        mD3DImmediateContext->IASetVertexBuffers( 0, 1, &mMeshVB, &stride, &offset );
        mD3DImmediateContext->IASetIndexBuffer( mMeshIB, DXGI_FORMAT_R32_UINT, 0 );

        XMMATRIX world = XMLoadFloat4x4( &mMeshWorld );
        XMMATRIX worldInvTranspose = MathHelper::InverseTranspose( world );
        XMMATRIX worldViewProj = world*view*proj;

        Effects::BasicFX->SetWorld( world );
        Effects::BasicFX->SetWorldInvTranspose( worldInvTranspose );
        Effects::BasicFX->SetWorldViewProj( worldViewProj );
        Effects::BasicFX->SetMaterial( mMeshMat );

        activeMeshTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
        mD3DImmediateContext->DrawIndexed( mMeshIndexCount, 0, 0 );

        // Restore default
        mD3DImmediateContext->RSSetState( 0 );

        // Draw just the picked triangle again with a different material to highlight it.

        if ( mPickedTriangle != -1 )
        {
            // Change depth test from < to <= so that if we draw the same triangle twice, it will still pass
            // the depth test.  This is because we redraw the picked triangle with a different material
            // to highlight it.  

            mD3DImmediateContext->OMSetDepthStencilState( RenderStates::LessEqualDSS, 0 );

            Effects::BasicFX->SetMaterial( mPickedTriangleMat );
            activeMeshTech->GetPassByIndex( p )->Apply( 0, mD3DImmediateContext );
            mD3DImmediateContext->DrawIndexed( 3, 3 * mPickedTriangle, 0 );

            // restore default
            mD3DImmediateContext->OMSetDepthStencilState( 0, 0 );
        }
    }

    HR( mSwapChain->Present( 0, 0 ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::onMouseDown( WPARAM btnState, int x, int y )
{
    if ( ( btnState & MK_LBUTTON ) != 0 ) {
        mLastMousePos.x = x;
        mLastMousePos.y = y;

        SetCapture( mMainWindow );
    }
    else if ( ( btnState & MK_RBUTTON ) != 0 ) {
        Pick( x, y );
    }
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
    else if ( ( btnState & MK_RBUTTON ) != 0 ) {
        Pick( x, y );
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::BuildMeshGeometryBuffers()
{
    std::ifstream fin( "Models/car.txt" );

    if ( !fin )
    {
        MessageBox( 0, L"Models/car.txt not found.", 0, 0 );
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
    mMeshVertices.resize( vcount );
    for ( UINT i = 0; i < vcount; ++i )
    {
        fin >> mMeshVertices[i].pos.x >> mMeshVertices[i].pos.y >> mMeshVertices[i].pos.z;
        fin >> mMeshVertices[i].normal.x >> mMeshVertices[i].normal.y >> mMeshVertices[i].normal.z;

        XMVECTOR P = XMLoadFloat3( &mMeshVertices[i].pos );

        vMin = XMVectorMin( vMin, P );
        vMax = XMVectorMax( vMax, P );
    }

    XMStoreFloat3( &mMeshBox.Center, 0.5f*( vMin + vMax ) );
    XMStoreFloat3( &mMeshBox.Extents, 0.5f*( vMax - vMin ) );

    fin >> ignore;
    fin >> ignore;
    fin >> ignore;

    mMeshIndexCount = 3 * tcount;
    mMeshIndices.resize( mMeshIndexCount );
    for ( UINT i = 0; i < tcount; ++i )
    {
        fin >> mMeshIndices[i * 3 + 0] >> mMeshIndices[i * 3 + 1] >> mMeshIndices[i * 3 + 2];
    }

    fin.close();

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof( Vertex::Basic32 ) * vcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &mMeshVertices[0];
    HR( mD3DDevice->CreateBuffer( &vbd, &vinitData, &mMeshVB ) );

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof( UINT ) * mMeshIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &mMeshIndices[0];
    HR( mD3DDevice->CreateBuffer( &ibd, &iinitData, &mMeshIB ) );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void App::Pick( int sx, int sy )
{
    XMMATRIX P = mCam.Proj();

    // Compute picking ray in view space.
    const float vx = ( +2.f * sx / mClientWidth - 1.f ) / P( 0, 0 );
    const float vy = ( -2.f * sy / mClientHeight + 1.f ) / P( 1, 1 );

    XMVECTOR rayOrigin = XMVectorSet( 0.f, 0.f, 0.f, 1.f );
    XMVECTOR rayDir = XMVectorSet( vx, vy, 1.f, 0.f );

    // Transform ray to local space of mesh.
    XMMATRIX V = mCam.View();
    XMMATRIX invView = XMMatrixInverse( &XMMatrixDeterminant( V ), V );

    XMMATRIX W = XMLoadFloat4x4( &mMeshWorld );
    XMMATRIX invWorld = XMMatrixInverse( &XMMatrixDeterminant( W ), W );

    XMMATRIX toLocal = XMMatrixMultiply( invView, invWorld );

    rayOrigin = XMVector3TransformCoord( rayOrigin, toLocal );
    rayDir = XMVector3TransformNormal( rayDir, toLocal );

    // Ray direction must be normalized for intersection test.
    rayDir = XMVector3Normalize( rayDir );

    //
    // Test for collision of bounding box first, then triangle mesh collision.

    mPickedTriangle = -1;
    float tmin = 0.f;
    if ( mMeshBox.Intersects( rayOrigin, rayDir, tmin ) ) {
        // Find nearest triangle/ray intersection.
        tmin = MathHelper::Infinity;
        for ( UINT i = 0; i < mMeshIndices.size() / 3; ++i ) {
            // Indices for this triangle.
            UINT i0 = mMeshIndices[i * 3 + 0];
            UINT i1 = mMeshIndices[i * 3 + 1];
            UINT i2 = mMeshIndices[i * 3 + 2];

            // Vertices for this triangle.
            XMVECTOR v0 = XMLoadFloat3( &mMeshVertices[i0].pos );
            XMVECTOR v1 = XMLoadFloat3( &mMeshVertices[i1].pos );
            XMVECTOR v2 = XMLoadFloat3( &mMeshVertices[i2].pos );

            float t = 0.f;
            if ( TriangleTests::Intersects( rayOrigin, rayDir, v0, v1, v2, t ) ) {
                if ( t < tmin ) {
                    tmin = t;

                    mPickedTriangle = i;
                }
            }
        }
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //