//
// Created by david on 11/17/19.
//
#pragma once
#if __linux__

#define MILISECONDS_IN_ONE_SECOND 1000
#define NANOSECONDS_IN_ONE_MILISECOND 1000000
#define NANOSECONDS_IN_ONE_SECOND 1000000000
typedef struct
{
   struct timespec value;
} timer;

static inline struct timespec timer_update(timer* timerToUpdate, i32 clockMode)
{
    struct timespec poppedTimer;
    poppedTimer.tv_nsec = timerToUpdate->value.tv_nsec;
    poppedTimer.tv_sec = timerToUpdate->value.tv_sec;
    clock_gettime(clockMode, &timerToUpdate->value);
    return poppedTimer;
}

static inline i64 timer_popMs(timer* timerToQuery)
{
    i64 nanoseconds = timerToQuery->value.tv_nsec;
    i64 seconds = timerToQuery->value.tv_sec;
    return (MILISECONDS_IN_ONE_SECOND * seconds) + (nanoseconds / NANOSECONDS_IN_ONE_MILISECOND);
}

static inline i64 timer_popNs(timer* timerToQuery)
{
    i64 nanoseconds = timerToQuery->value.tv_nsec + (timerToQuery->value.tv_sec * NANOSECONDS_IN_ONE_SECOND);
    return nanoseconds;
}
#endif
