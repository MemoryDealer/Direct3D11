// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file D3DUtil.h
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#pragma once

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#if defined( DEBUG ) || defined( _DEBUG )
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <D3DX11.h>
#include <xnamath.h>
#include <DxErr.h>
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

// Error checker.
#if defined( DEBUG ) || defined( _DEBUG )
#ifndef HR
#define HR( x )\
{\
HRESULT hr = ( x );\
if( FAILED( hr ) ){\
DXTrace( __FILE__, ( DWORD )__LINE__, hr, L#x, true );\
}\
}
#endif

#else
#ifndef HR
#define HR( x ) ( x )
#endif
#endif

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#define ReleaseCOM( x ) { if( x ){ x->Release(); x = nullptr; } }
#define SafeDelete( x ) { delete x; x = nullptr; }

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //



// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //