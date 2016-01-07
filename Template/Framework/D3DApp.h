// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file D3DApp.h
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#pragma once

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "D3DUtil.h"
#include "GameTimer.h"

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

//namespace D3DApp {

    class D3DApp
    {

    public:

        D3DApp( void ) { }

        D3DApp( HINSTANCE hInstance );
        
        virtual ~D3DApp( void );

        HINSTANCE getAppInst( void ) const;
        HWND getMainWindow( void ) const;
        float getAspectRatio( void ) const;

        int run( void );

        // Framework methods:

        virtual bool init( void );
        virtual void onResize( void );
        virtual void updateScene( const float dt ) = 0;
        virtual void drawScene( void ) = 0;
        virtual LRESULT msgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

        virtual void onMouseDown( WPARAM btnState, int x, int y ) { }
        virtual void onMouseUp( WPARAM btnState, int x, int y ) { }
        virtual void onMouseMove( WPARAM btnState, int x, int y ) { }

    protected:

        bool initMainWindow( void );
        bool initDirect3D( void );

        void calculateFrameStats( void );

    protected:

        HINSTANCE mAppInstance;
        HWND mMainWindow;
        bool mPaused, mMinimized, mMaximized, mResizing;
        UINT m4xMsaaQuality;

        GameTimer mTimer;

        // Direct3D:

        ID3D11Device* mD3DDevice;
        ID3D11DeviceContext* mD3DImmediateContext;
        IDXGISwapChain* mSwapChain;
        ID3D11Texture2D* mDepthStencilBuffer;
        ID3D11RenderTargetView* mRenderTargetView;
        ID3D11DepthStencilView* mDepthStencilView;
        D3D11_VIEWPORT mViewport;

        // For derived classes:

        std::wstring mMainWindowCaption;
        D3D_DRIVER_TYPE mD3DDriverType;
        int mClientWidth;
        int mClientHeight;
        bool mEnable4xMsaa;

    };

//}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //