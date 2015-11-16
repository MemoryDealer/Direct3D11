// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file main.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DApp.h"
#include <Windows.h>

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

class InitApp : public D3DApp
{
public:
    
    InitApp( HINSTANCE hInstance )
    : D3DApp( hInstance )
    {
    }

    ~InitApp( void )
    {
    }

    virtual bool init( void ) override
    {
        if ( !D3DApp::init() ) {
            return false;
        }
        
        return true;
    }

    virtual void onResize( void ) override
    {
        D3DApp::onResize();
    }

    virtual void updateScene( const float dt ) override
    {

    }

    virtual void drawScene( void ) override
    {
        assert( mD3DImmediateContext );
        assert( mSwapChain );

        mD3DImmediateContext->
            ClearRenderTargetView( mRenderTargetView,
                                   reinterpret_cast<const float*>( &Colors::Magenta ) );
        mD3DImmediateContext->ClearDepthStencilView( mDepthStencilView,
                                                     D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                                     1.0f,
                                                     0 );

        HR( mSwapChain->Present( 0, 0 ) );
    }
};

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

int WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    PSTR cmdLine,
                    int showCmd )
{
#if defined( DEBUG ) || defined( _DEBUG )
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    InitApp app( hInstance );

    if ( !app.init() ) {
        return 1;
    }

    return app.run();
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //