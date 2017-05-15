/*
 * This proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#include "Timer.h"

#if defined(_WIN32)

#include <cstdio>
#include <windows.h>

namespace MaliSDK
{
    Timer::Timer()
    {
        LARGE_INTEGER l;
        QueryPerformanceFrequency(&l);
        invFreq = 1.0f / l.QuadPart;
        reset();

        oldTime = getTime();
    }

    void Timer::reset()
    {
        LARGE_INTEGER l;
        QueryPerformanceCounter(&l);
        resetStamp = (((double)l.QuadPart) * invFreq);

        oldTime = getTime();
    }

    float Timer::getTime()
    {
        LARGE_INTEGER l;
        QueryPerformanceCounter(&l);
        return (float)(((double)l.QuadPart) * invFreq - resetStamp);
    }

    float Timer::getDelta(){
        float now = getTime();
        float delta = now - oldTime;
        oldTime = now;
        return delta;
    }
}
#else

#include <sys/time.h>

namespace MaliSDK
{
    Timer::Timer()
        : startTime()
        , currentTime()
    {
        startTime.tv_sec = 0;
        startTime.tv_usec = 0;
        currentTime.tv_sec = 0;
        currentTime.tv_usec = 0;

        reset();

        oldTime = getTime();
    }

    void Timer::reset()
    {
        gettimeofday(&startTime, NULL);

        oldTime = getTime();
    }

    float Timer::getTime()
    {
        gettimeofday(&currentTime, NULL);
        float seconds = (currentTime.tv_sec - startTime.tv_sec);
        float milliseconds = (float(currentTime.tv_usec - startTime.tv_usec)) / 1000000.0f;
        return seconds + milliseconds;
    }
    float Timer::getDelta(){
        float now = getTime();
        float delta = now - oldTime;
        oldTime = now;
        return delta;
    }
}
#endif
