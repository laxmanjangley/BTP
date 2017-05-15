#ifndef TIMER_H
#define TIMER_H

#include <cstdio>

#if defined(_WIN32)
#else
#include <sys/time.h>
#endif

namespace MaliSDK
{
    /**
     * \brief Provides a platform independent high resolution timer.
     * \note The timer measures real time, not CPU time.
     */
    class Timer
    {
    private:
        double oldTime;
    #if defined(_WIN32)
        double resetStamp;
        double invFreq;
    #else
        timeval startTime;
        timeval currentTime;
    #endif
    public:
        /**
         * \brief Default Constructor
         */
        Timer();

        /**
         * \brief Resets the timer to 0.0f.
         */
        void reset();

        /**
         * \brief Returns the time passed since object creation or since reset() was last called.
         * \return Float containing the current time.
         */
        float getTime();

        float getDelta();
    };
}
#endif /* TIMER_H */