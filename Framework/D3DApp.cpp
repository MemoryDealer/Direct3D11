// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file D3DApp.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"

#include <windowsx.h>

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

namespace {
    D3DApp* gD3DApp = nullptr;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

// Must define the main window procedure outside of a class.
LRESULT CALLBACK MainWindowProc( HWND hwnd,
                                 UINT msg,
                                 WPARAM wParam,
                                 LPARAM lParam )
{
    return gD3DApp->msgProc( hwnd, msg, wParam, lParam );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

//using namespace D3DApp;

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

D3DApp::D3DApp( HINSTANCE hInstance )
: mAppInstance( hInstance )
, mMainWindow( nullptr )
, mPaused( false )
, mMinimized( false )
, mMaximized( false )
, mResizing( false )
, m4xMsaaQuality( 0 )
, mTimer( )
, mD3DDevice( nullptr )
, mD3DImmediateContext( nullptr )
, mSwapChain( nullptr )
, mDepthStencilBuffer( nullptr )
, mRenderTargetView( nullptr )
, mDepthStencilView( nullptr )
, mViewport( )
, mMainWindowCaption( L"D3D11 App" )
, mD3DDriverType( D3D_DRIVER_TYPE_HARDWARE )
, mClientWidth( 1024 )
, mClientHeight( 768 )
, mEnable4xMsaa( false )
{
    ZeroMemory( &mViewport, sizeof( D3D11_VIEWPORT ) );

    gD3DApp = this;
}

D3DApp* gD3DApp = nullptr;

D3DApp::~D3DApp( void )
{
    ReleaseCOM( mRenderTargetView );
    ReleaseCOM( mDepthStencilView );
    ReleaseCOM( mSwapChain );
    ReleaseCOM( mDepthStencilBuffer );

    // Restore default settings.
    if ( mD3DImmediateContext ) {
        mD3DImmediateContext->ClearState();
    }

    ReleaseCOM( mD3DImmediateContext );
    ReleaseCOM( mD3DDevice );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

HINSTANCE D3DApp::getAppInst( void ) const
{
    return mAppInstance;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

HWND D3DApp::getMainWindow( void ) const
{
    return mMainWindow;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

float D3DApp::getAspectRatio( void ) const
{
    return static_cast<float>(mClientWidth) / static_cast<float>( mClientHeight );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

int D3DApp::run( void )
{
    MSG msg = { 0 };

    mTimer.reset();

    while ( msg.message != WM_QUIT ) {
        if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else {
            mTimer.tick();

            if ( !mPaused ) {
                calculateFrameStats();
                updateScene( mTimer.deltaTime() );
                drawScene();
            }
            else {
                Sleep( 100 );
            }
        }
    }

    return static_cast<int>( msg.wParam );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool D3DApp::init( void )
{
    if ( !initMainWindow() ) {
        return false;
    }

    if ( !initDirect3D() ) {
        return false;
    }

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void D3DApp::onResize( void )
{
    assert( mD3DImmediateContext );
    assert( mD3DDevice );
    assert( mSwapChain );

    // Release old views since they hold references to the buffers we are 
    // destroying, and also release depth stencil buffer.

    ReleaseCOM( mRenderTargetView );
    ReleaseCOM( mDepthStencilView );
    ReleaseCOM( mDepthStencilBuffer );

    // Resize the swap chain and re-create the render target view.

    HR( mSwapChain->ResizeBuffers( 1,
                                   mClientWidth,
                                   mClientHeight,
                                   DXGI_FORMAT_R8G8B8A8_UNORM,
                                   0 ) );
    ID3D11Texture2D* backBuffer;

    // Get the back buffer (only one, so index 0).
    HR( mSwapChain->GetBuffer( 0, 
                               __uuidof( ID3D11Texture2D ),
                               reinterpret_cast<void**>( &backBuffer ) ) );

    // Create a render target view to the back buffer.
    HR( mD3DDevice->CreateRenderTargetView( backBuffer,
                                            nullptr,
                                            &mRenderTargetView ) );
    ReleaseCOM( backBuffer );

    // Create depth/stencil buffer and view for it.
    // This is just a 2D texture that stores depth/stencil information.

    D3D11_TEXTURE2D_DESC dsd;
    dsd.Width = mClientWidth;
    dsd.Height = mClientHeight;
    dsd.MipLevels = 1;
    dsd.ArraySize = 1;
    dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    if ( mEnable4xMsaa ) {
        dsd.SampleDesc.Count = 4;
        dsd.SampleDesc.Quality = m4xMsaaQuality;
    }
    else {
        dsd.SampleDesc.Count = 1;
        dsd.SampleDesc.Quality = 0;
    }

    dsd.Usage = D3D11_USAGE_DEFAULT;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dsd.CPUAccessFlags = 0;
    dsd.MiscFlags = 0;

    HR( mD3DDevice->CreateTexture2D( &dsd,
                                     nullptr,
                                     &mDepthStencilBuffer ) );
    HR( mD3DDevice->CreateDepthStencilView( mDepthStencilBuffer,
                                            nullptr,
                                            &mDepthStencilView ) );

    // Bind the render target view and depth/stencil view to the pipeline.

    mD3DImmediateContext->OMSetRenderTargets( 1, 
                                              &mRenderTargetView, 
                                              mDepthStencilView );

    // Set the viewport transform.

    mViewport.TopLeftX = 0;
    mViewport.TopLeftY = 0;
    mViewport.Width = static_cast<float>( mClientWidth );
    mViewport.Height = static_cast<float>( mClientHeight );
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;

    mD3DImmediateContext->RSSetViewports( 1, &mViewport );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

LRESULT D3DApp::msgProc( HWND hwnd,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam )
{
    switch ( msg ) {

    case WM_ACTIVATE:
        if ( LOWORD( wParam ) == WA_INACTIVE ) {
            mPaused = true;
            mTimer.stop();
        }
        else {
            mPaused = false;
            mTimer.start();
        }
        return 0;

    case WM_SIZE:
        mClientWidth = LOWORD( lParam );
        mClientHeight = HIWORD( lParam );
        if ( mD3DDevice ) {
            switch ( wParam ) {
            default:
                break;

            case SIZE_MINIMIZED:
                mPaused = true;
                mMinimized = true;
                mMaximized = false;
                break;

            case SIZE_MAXIMIZED:
                mPaused = false;
                mMinimized = false;
                mMaximized = true;
                onResize();
                break;

            case SIZE_RESTORED:
                // Restoring from mimimized state.
                if ( mMinimized ) {
                    mPaused = false;
                    mMinimized = false;
                    onResize();
                }
                // Restoring from maximized state.
                else if ( mMaximized ) {
                    mPaused = false;
                    mMaximized = false;
                    onResize();
                }
                else if ( mResizing ) {

                }
                else {
                    onResize();
                }
                break;
            }
        }
        return 0;

        // User grabs resize bar.
    case WM_ENTERSIZEMOVE:
        mPaused = true;
        mResizing = true;
        mTimer.stop();
        return 0;

        // User releases resize bar.
    case WM_EXITSIZEMOVE:
        mPaused = false;
        mResizing = false;
        mTimer.start();
        onResize();
        return 0;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

    case WM_MENUCHAR:
        return MAKELRESULT( 0, MNC_CLOSE );

    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>( lParam )->ptMinTrackSize.x = 200;
        reinterpret_cast<MINMAXINFO*>( lParam )->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        onMouseDown( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
        return 0;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        onMouseUp( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
        return 0;

    case WM_MOUSEMOVE:
        onMouseMove( wParam, GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );
        return 0;
    }

    return DefWindowProc( hwnd, msg, wParam, lParam );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool D3DApp::initMainWindow( void )
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = mAppInstance;
    wc.hIcon = LoadIcon( 0, IDI_APPLICATION );
    wc.hCursor = LoadCursor( 0, IDC_ARROW );
    wc.hbrBackground = static_cast<HBRUSH>( GetStockObject( NULL_BRUSH ) );
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    if ( !RegisterClass( &wc ) )
    {
        MessageBox( 0, L"RegisterClass Failed.", 0, 0 );
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT R = { 0, 0, mClientWidth, mClientHeight };
    AdjustWindowRect( &R, WS_OVERLAPPEDWINDOW, false );
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    mMainWindow = CreateWindow( L"D3DWndClassName", 
                                mMainWindowCaption.c_str(),
                                WS_OVERLAPPEDWINDOW, 
                                CW_USEDEFAULT, 
                                CW_USEDEFAULT, 
                                width, 
                                height, 
                                0, 
                                0, 
                                mAppInstance, 
                                0 );
    if ( !mMainWindow )
    {
        MessageBox( 0, L"CreateWindow Failed.", 0, 0 );
        return false;
    }

#if !defined ( __NO_CONSOLE__ )
    AllocConsole();
    FILE* pCout = nullptr;
    freopen_s( &pCout, "CONOUT$", "w", stdout );
    freopen_s( &pCout, "CONOUT$", "w", stderr );
    fclose( pCout );
#endif

    ShowWindow( mMainWindow, SW_SHOW );
    UpdateWindow( mMainWindow );

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

bool D3DApp::initDirect3D( void )
{
    // Create device and device context first.

    UINT createDeviceFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice( nullptr,
                                    mD3DDriverType,
                                    nullptr,
                                    createDeviceFlags,
                                    nullptr,
                                    0,
                                    D3D11_SDK_VERSION,
                                    &mD3DDevice,
                                    &featureLevel,
                                    &mD3DImmediateContext );
    if ( FAILED( hr ) ) {
        MessageBox( nullptr, L"D3D11CreateDevice failed.", nullptr, 0 );
        return false;
    }

    if ( featureLevel != D3D_FEATURE_LEVEL_11_0 ) {
        MessageBox( nullptr, L"Direct3D Feature Level 11 not supported.", nullptr, 0 );
        return false;
    }

    // Check for multisampling quality support on the hardware.
    // (All D3D11 devices support multisampling).

    HR( mD3DDevice->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM,
                                                   4,
                                                   &m4xMsaaQuality ) );
    assert( m4xMsaaQuality > 0 );

    // Setup swap chain descriptor data.

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = mClientWidth;
    sd.BufferDesc.Height = mClientHeight;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    if ( mEnable4xMsaa ) {
        sd.SampleDesc.Count = 4;
        sd.SampleDesc.Quality = m4xMsaaQuality - 1;
    }
    else {
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
    }

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;
    sd.OutputWindow = mMainWindow;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = 0;

    // Create a swap chain using the IDXGIFactory that was used to create the 
    // device.

    IDXGIDevice* dxgiDevice = nullptr;
    HR( mD3DDevice->QueryInterface( __uuidof( IDXGIDevice ), 
                                    reinterpret_cast<void**>( &dxgiDevice ) ) );

    IDXGIAdapter* dxgiAdapter = nullptr;
    HR( dxgiDevice->GetParent( __uuidof( IDXGIAdapter ), 
                               reinterpret_cast<void**>( &dxgiAdapter ) ) );

    IDXGIFactory* dxgiFactory = nullptr;
    HR( dxgiAdapter->GetParent( __uuidof( IDXGIFactory ),
                                reinterpret_cast<void**>( &dxgiFactory ) ) );

    HR( dxgiFactory->CreateSwapChain( mD3DDevice, &sd, &mSwapChain ) );

    ReleaseCOM( dxgiDevice );
    ReleaseCOM( dxgiAdapter );
    ReleaseCOM( dxgiFactory );

    // Remaining steps to create D3D are also required by resize.
    onResize();

    return true;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void D3DApp::calculateFrameStats( void )
{
    static int frameCount = 0;
    static float elapsedTime = 0.0f;

    ++frameCount;

    // Get averages over one second period.
    if ( ( mTimer.totalTime() - elapsedTime ) >= 1.0f ) {
        float fps = static_cast<float>( frameCount );
        float mspf = 1000.0f / fps;

        std::wostringstream oss;
        oss.precision( 6 );
        oss << mMainWindowCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText( mMainWindow, oss.str().c_str() );

        frameCount = 0;
        elapsedTime += 1.0f;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //