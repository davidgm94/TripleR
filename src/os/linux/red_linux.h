//
// Created by david on 11/2/19.
//
#if ___linux___
#pragma once

void platform_DebugBreak(void)
{
    raise(SIGTRAP);
}

int platform_Printf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = vfprintf(stdout, format, args);
    va_end(args);

    return result;
}

int platform_Sprintf(char* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int result = vsprintf(buffer, format, args);
    va_end(args);
    return result;
}
void* platform_Memcpy(void* destination, void* source, size_t size)
{
    return memcpy(destination, source, size);
}

typedef enum {
   RESULT_ERROR,
   RESULT_SUCCESS
}result_t;
typedef struct {
    pthread_t threadHandle;
    result_t error;
} thread_info;

void* foo(void* parameter)
{
    int param = *((int*)parameter);
    printf("Thread %d started\n", param);
}
thread_info platform_createThread(void*(*fn)(void*), void* param)
{
    pthread_t thread;
    if (pthread_create(&thread, null, fn, param)) {
        return (thread_info) {thread, RESULT_SUCCESS};
    }
    else
    {
        return (thread_info) {null, RESULT_ERROR};
    }
}

typedef enum
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
} LOGGER_LEVEL;

void logger(LOGGER_LEVEL loggerLevel, const char* tag, const char* message)
{
    puts(message);
}
static inline struct timespec getUnixTimespec()
{
    struct timespec ts;
    int result = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (result != 0)
    {
        logger(ERROR, "[CLOCK]", "Error getting the clock from system");
    }
    return ts;
}
static inline i32 getSystemMiliseconds()
{
    struct timespec ts = getUnixTimespec();

    i32 seconds = ts.tv_sec;
    i32 nanoSeconds = ts.tv_nsec;
    i32 milliSeconds = 1000 * seconds + /*round*/((double)nanoSeconds / 1.0e6);
    return milliSeconds;
}

static inline i64 getSystemNanoseconds()
{
    struct timespec ts = getUnixTimespec();
    i64 seconds = ts.tv_sec;
    i64 nanoSeconds = ts.tv_nsec;

    nanoSeconds += seconds * 10e9;
    return nanoSeconds;
}
void platform_DebugInfo(const char* message)
{
   puts(message);
}

void* platform_alloc(size_t size)
{
    i64 start = getSystemNanoseconds();
    void* memory = mmap(null, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -2, 0);
    i64 end = getSystemNanoseconds();
    printf("Allocation of %zu bytes took %llu ns.\n", size, end - start);
    return memory;
}

#endif
