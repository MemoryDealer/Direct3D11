// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file GameTimer.cpp
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#include "GameTimer.h"

#include <Windows.h>

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

//using namespace D3DApp;

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

GameTimer::GameTimer( void )
: mSecondsPerCount( 0.0 )
, mDeltaTime( -1.0 )
, mBaseTime( 0 )
, mPausedTime( 0 )
, mPrevTime( 0 )
, mCurrTime( 0 )
, mStopped( false )
{
    // Get initial seconds per count.
    __int64 countsPerSec;
    QueryPerformanceFrequency( 
        reinterpret_cast<LARGE_INTEGER*>( &countsPerSec ) );
    mSecondsPerCount = 1.0 / static_cast<double>( countsPerSec );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

float GameTimer::totalTime( void ) const
{
    if ( mStopped ) {
        return static_cast<float>( ( ( 
            mStopTime - mPausedTime ) - mBaseTime ) * mSecondsPerCount );
    }
    else {
        return static_cast<float>( ( ( 
            mCurrTime - mPausedTime ) - mBaseTime ) * mSecondsPerCount );
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

float GameTimer::deltaTime( void ) const
{
    return static_cast<float>( mDeltaTime );
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void GameTimer::reset( void )
{
    __int64 currTime;
    QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &currTime ) );

    mBaseTime = currTime;
    mPrevTime = currTime;
    mStopTime = 0;
    mStopped = false;
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void GameTimer::start( void )
{
    if ( mStopped ) {
        __int64 startTime;
        QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &startTime ) );

        mPausedTime += ( startTime - mStopTime );
        mPrevTime = startTime;
        mStopTime = 0;
        mStopped = false;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void GameTimer::stop( void )
{
    if ( !mStopped ) {
        __int64 currTime;
        QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &currTime ) );

        mStopTime = currTime;
        mStopped = true;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

void GameTimer::tick( void )
{
    if ( mStopped ) {
        mDeltaTime = 0.0;
        return;
    }

    __int64 currTime;
    QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &currTime ) );
    mCurrTime = currTime;

    mDeltaTime = ( mCurrTime - mPrevTime ) * mSecondsPerCount;

    mPrevTime = mCurrTime;

    // Negative time possible if CPU goes into power save mode.
    if ( mDeltaTime < 0.0 ) {
        mDeltaTime = 0.0;
    }
}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //