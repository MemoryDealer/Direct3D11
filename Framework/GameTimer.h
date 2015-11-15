// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
/// \file GameTimer.h
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

#pragma once

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

namespace D3DApp {

    class GameTimer
    {

    public:

        GameTimer( void );

        float totalTime( void ) const;
        float deltaTime( void ) const;

        void reset( void );
        void start( void );
        void stop( void );
        void tick( void );

    private:

        double mSecondsPerCount;
        double mDeltaTime;

        __int64 mBaseTime;
        __int64 mPausedTime;
        __int64 mStopTime;
        __int64 mPrevTime;
        __int64 mCurrTime;

        bool mStopped;

    };

}

// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
