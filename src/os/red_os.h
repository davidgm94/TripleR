#pragma once

#define MAX_FILENAME_LENGTH 1024
static char sourceDirectory[MAX_FILENAME_LENGTH];
static char shaderDirectory[MAX_FILENAME_LENGTH];
static char assetDirectory[MAX_FILENAME_LENGTH];
#if _WINDOWS
#define RRR_WIN32 1
#elif __linux__
#define RRR_LINUX
#endif
//#ifdef __cplusplus
// extern "C"
//{
// #endif
#if _WIN64
#if _DEBUG
#define NDEBUG 0
#else
#define NDEBUG 1
#endif
#else
#endif

// #define FORCE_INLINE 0
//
// #if _MSC_VER
//
// #if FORCE_INLINE
// #define inline __forceinline
// #else
// #define inline inline
// #endif
//
// #elif __GNUC__
// #if FORCE_INLINE
// #define inline inline __attribute__((always_inline))
// #else
// #define inline inline
// #endif
//
//#endif

#define ARRAYCOUNT(arr) (sizeof(arr) / sizeof(arr[0]))

// BOOLEAN
#ifndef __cplusplus
#define bool	_Bool
#define false	0
#define true	1
#define nullptr NULL
#endif
#define null nullptr

// TYPES
#include <stdint.h>
    typedef signed char        i8;
    typedef short              i16;
    typedef int                i32;
    typedef long long          i64;
    typedef unsigned char      u8;
    typedef unsigned short     u16;
    typedef unsigned int       u32;
    typedef unsigned long long u64;
    typedef u16 f16;

	//typedef u64 size_t;
    typedef i64 intptr;
    typedef u64 uintptr;
    typedef unsigned int uint;

#define BYTE_SIZE 1Ui64
#define KILOBYTE (1024 * BYTE_SIZE)
#define MEGABYTE (1024 * KILOBYTE)
#define GIGABYTE (1024 * MEGABYTE)

#include <vector>
    using std::vector;
// ASSERT
#include <assert.h>

	// NOT USED
	#if NDEBUG
	#define red_assert(condition)
	#else
	#if 0
	#define red_assert(condition) if (!(condition)) { platform_DebugInfo("[ASSERT FAILED] The expression " #condition " is false\n"); platform_DebugBreak(); }
	#define msg_red_assert(condition, message) if (!(condition)) { platform_DebugInfo("[ASSERT FAILED] The expression " #condition " is false\n[LOCATION] " #message "\n\n"); platform_DebugBreak(); }
	#endif
	
	#define msg_assert(condition, message) msg_red_assert(condition, message)
	#define invalid_code_path assert(!"Invalid code path")
	#define invalid_default_case default: { invalid_code_path; break; }
	#endif
	// END NOT USED

#define os_assert(cond) (assert(cond))

// ERROR				??
// MATH					??
#include <math.h>
// SIGNAL
#include <signal.h>
// ALIGNMENT
// VARIABLE ARGS
#include <stdarg.h>
// ATOMIC TYPES
// STDDEF
#include <stddef.h>
	// OFFSETS: #define offsetof(s,m) ((size_t)&(((s*)0)->m))
// STDIO
#include <stdio.h>
	static inline FILE* os_fopen(const char* filename, const char* mode)
	{
		return fopen(filename, mode);
	}
	static inline int os_fclose(FILE* stream)
	{
		return fclose(stream);
	}
	static inline int os_fflush(FILE* stream)
	{
		return fflush(stream);
	}
	static inline size_t os_fread(void* ptr, size_t size, size_t count, FILE* stream)
	{
		return fread(ptr, size, count, stream);
	}
	static inline size_t os_fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
	{
		return fwrite(ptr, size, count, stream);
	}
	static inline int os_puts(const char* str)
	{
		return puts(str);
	}
	// TODO: Does this work?
	static inline int os_printf(const char* format, ...)
	{
		int result;
		va_list args;
		va_start(args, format);
		result = vfprintf(stdout, format, args);
		va_end(args);
		return result;
	}
	static inline int os_sprintf(char* buffer, const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		int result = vsprintf(buffer, format, args);

		va_end(args);
		return result;
	}
	static inline int os_fprintf(FILE* stream, const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		int result = vfprintf(stream, format, args);

		va_end(args);
		return result;
	}

// STRING
#include <string.h>
    static inline char* os_strtok(char* str, const char* delimiters)
    {
	   return strtok(str, delimiters);
    }
    static inline char* os_strcat(char* destination, const char* source)
    {
	    return strcat(destination, source);
    }
	static inline char* os_strcpy(char* destination, const char* source)
	{
		return strcpy(destination, source);
	}
	static inline int os_strcmp(const char* str1, const char* str2)
	{
		return strcmp(str1, str2);
	}
	static inline size_t os_strlen(const char* str)
	{
		return strlen(str);
	}

	static inline void os_memcpy(void* destination, const void* source, size_t bytes)
	{
		return (void)memcpy(destination, source, bytes);
	}
	static inline void* os_memset(void* ptr, int value, size_t bytes)
	{
		return memset(ptr, value, bytes);
	}
	static inline int os_memcmp(const void* ptr1, const void* ptr2, size_t bytes)
	{
		return memcmp(ptr1, ptr2, bytes);
	}
	static inline void* os_memmove(void* destination, const void* source, size_t bytes)
	{
		return memmove(destination, source, bytes);
	}
// THREAD SYSTEM
// MEMORY
#include <stdlib.h>
	static inline void* os_malloc(size_t bytes)
	{
		return malloc(bytes);
	}
	static inline void* os_calloc(size_t elementCount, size_t elementSize)
	{
		return calloc(elementCount, elementSize);
	}
	static inline void* os_realloc(void* ptr, size_t bytes)
	{
		return realloc(ptr, bytes);
	}
	static inline void os_free(void* ptr)
	{
		return free(ptr);
	}
	static inline void* os_aligned_alloc(size_t alignment, size_t size)
	{
		assert(0);
		return 0;
	}
// WINDOW
    typedef void* os_window_application_pointer;

    typedef struct os_window_list_entry
    {
        os_window_application_pointer window;
        struct os_window_list_entry* next;
    } os_window_list_entry;

	static os_window_list_entry osWindowListEntry;

	void os_setWindowTitle(os_window_application_pointer windowApplication, const char* title);

// WINDOW LOOP
    bool os_windowShouldClose(os_window_application_pointer windowApplication);
    void os_windowHandleEvents(os_window_application_pointer windowApplication);
// SHELL
// AUDIO
// NETWORKING

// LOGGER
#define SHOW_DEBUG 1
#define SHOW_WARNING 1
#define SHOW_ERROR 1
#define SHOW_CRITICAL 1

    typedef enum
    {
        DEBUG,
        WARNING,
        ERROR,
        CRITICAL
    } RED_LOG_SEVERITY;

// CONTAINERS
#define fast_vector_malloc(type, count) (type*)os_malloc(sizeof(type) * count)
#define fast_vector_free(pointer) os_free(pointer)
#define FAST_VECTOR(type) typedef struct { type* data; size_t size; } fast_vector_##type;\
    static inline void allocate_fast_vector_##type(fast_vector_##type* vector, size_t count)\
    {\
        vector->data = fast_vector_malloc(type, count);\
        vector->size = count;\
    }\
    \
    static inline void deallocate_fast_vector_##type(fast_vector_##type* vector, size_t count)\
    {\
        fast_vector_free(vector->data);\
        vector->data = null;\
        vector->size = 0;\
    }

    FAST_VECTOR(u32);
	FAST_VECTOR(u8);
	FAST_VECTOR(char);
	typedef fast_vector_char raw_str;

static os_window_application_pointer currentWindow = null;

typedef struct
{
    void* windowHandle;
    void* anotherOSHandle;
} os_window_handles;

typedef struct
{
	u32 width;
	u32 height;
} os_window_dimensions;

typedef enum
{
	APPLICATION_STATE_FINISHED,
	APPLICATION_STATE_RESIZING,
	APPLICATION_STATE_PAUSED,
	APPLICATION_STATE_ELEMENT_COUNT,
} APPLICATION_STATE;

typedef struct application_info
{
	bool s[APPLICATION_STATE_ELEMENT_COUNT];
} application_state;

typedef struct
{
	u32 halfTransitionCount;
	bool endedDown;
} engine_key_state;

typedef struct
{
	char keyState[UINT8_MAX + 1];
} engine_keyboard_state;

enum platform_keys
{
	MOUSE_LEFT_BUTTON = 0x01,
	MOUSE_RIGHT_BUTTON = 0x02,
	CANCEL = 0x03,
	MOUSE_MIDDLE_BUTTON = 0x04,
	MOUSE_X1_BUTTON = 0x05,
	MOUSE_X2_BUTTON = 0x06,
	UNDEFINED = 0x07,
	KEYBOARD_BACKSPACE = 0x08,
	KEYBOARD_TAB = 0x09,
	KEYBOARD_CLEAR = 0x0C,
	KEYBOARD_ENTER = 0x0D,
	KEYBOARD_SHIFT = 0x10,
	KEYBOARD_CONTROL = 0x11,
	KEYBOARD_ALT = 0x12,
	KEYBOARD_PAUSE = 0x13,
	KEYBOARD_CAPS_LOCK = 0x14,
	KEYBOARD_KANA_HANGUEL = 0x15,
	KEYBOARD_JUNJA = 0x17,
	KEYBOARD_FINAL = 0x18,
	KEYBOARD_HANJA = 0x19,
	KEYBOARD_ESCAPE = 0x1B,
	KEYBOARD_SPACE = 0x20,
	KEYBOARD_PAGE_UP = 0x21,
	KEYBOARD_PAGE_DOWN = 0x22,
	KEYBOARD_END = 0x23,
	KEYBOARD_HOME = 0x24,
	KEYBOARD_LEFT = 0x25,
	KEYBOARD_UP = 0x26,
	KEYBOARD_RIGHT = 0x27,
	KEYBOARD_DOWN = 0x28,
	KEYBOARD_SELECT = 0x29,
	KEYBOARD_PRINT = 0x2A,
	KEYBOARD_EXECUTE = 0x2B,
	KEYBOARD_PRINT_SCREEN = 0x2C,
	KEYBOARD_INSERT = 0x2D,
	KEYBOARD_DELETE = 0x2E,
	KEYBOARD_HELP = 0x2F,
	KEYBOARD_0 = 0x31,
	KEYBOARD_1 = 0x32,
	KEYBOARD_2 = 0x33,
	KEYBOARD_3 = 0x34,
	KEYBOARD_4 = 0x35,
	KEYBOARD_5 = 0x36,
	KEYBOARD_6 = 0x37,
	KEYBOARD_7 = 0x38,
	KEYBOARD_8 = 0x39,
	KEYBOARD_9 = 0x3A,
	KEYBOARD_A = 0x41,
	KEYBOARD_B = 0x42,
	KEYBOARD_C = 0x43,
	KEYBOARD_D = 0x44,
	KEYBOARD_E = 0x45,
	KEYBOARD_F = 0x46,
	KEYBOARD_G = 0x47,
	KEYBOARD_H = 0x48,
	KEYBOARD_I = 0x49,
	KEYBOARD_J = 0x4A,
	KEYBOARD_K = 0x4B,
	KEYBOARD_L = 0x4C,
	KEYBOARD_M = 0x4D,
	KEYBOARD_N = 0x4E,
	KEYBOARD_O = 0x4F,
	KEYBOARD_P = 0x50,
	KEYBOARD_Q = 0x51,
	KEYBOARD_R = 0x52,
	KEYBOARD_S = 0x53,
	KEYBOARD_T = 0x54,
	KEYBOARD_U = 0x55,
	KEYBOARD_V = 0x56,
	KEYBOARD_W = 0x57,
	KEYBOARD_X = 0x58,
	KEYBOARD_Y = 0x59,
	KEYBOARD_Z = 0x5A,
	KEYBOARD_LEFT_WINDOWS = 0x5B,
	KEYBOARD_RIGHT_WINDOWS = 0x5C,
	KEYBOARD_APPLICATION = 0x5D,
	KEYBOARD_SLEEP = 0x5F,
	KEYBOARD_NOTEPAD_0 = 0x60,
	KEYBOARD_NOTEPAD_1 = 0x61,
	KEYBOARD_NOTEPAD_2 = 0x62,
	KEYBOARD_NOTEPAD_3 = 0x63,
	KEYBOARD_NOTEPAD_4 = 0x64,
	KEYBOARD_NOTEPAD_5 = 0x65,
	KEYBOARD_NOTEPAD_6 = 0x66,
	KEYBOARD_NOTEPAD_7 = 0x67,
	KEYBOARD_NOTEPAD_8 = 0x68,
	KEYBOARD_NOTEPAD_9 = 0x69,
	KEYBOARD_MULTIPLY = 0x6A,
	KEYBOARD_ADD = 0x6B,
	KEYBOARD_SEPARATOR = 0x6C,
	KEYBOARD_SUBSTRACT = 0x6D,
	KEYBOARD_DECIMAL = 0x6E,
	KEYBOARD_DIVIDE = 0x6F,
	KEYBOARD_F1 = 0x70,
	KEYBOARD_F2 = 0x71,
	KEYBOARD_F3 = 0x72,
	KEYBOARD_F4 = 0x73,
	KEYBOARD_F5 = 0x74,
	KEYBOARD_F6 = 0x75,
	KEYBOARD_F7 = 0x76,
	KEYBOARD_F8 = 0x77,
	KEYBOARD_F9 = 0x78,
	KEYBOARD_F10 = 0x79,
	KEYBOARD_F11 = 0x7A,
	KEYBOARD_F12 = 0x7B,
	KEYBOARD_F13 = 0x7C,
	KEYBOARD_F14 = 0x7D,
	KEYBOARD_F15 = 0x7E,
	KEYBOARD_F16 = 0x7F,
	KEYBOARD_F17 = 0x80,
	KEYBOARD_F18 = 0x81,
	KEYBOARD_F19 = 0x82,
	KEYBOARD_F20 = 0x83,
	KEYBOARD_F21 = 0x84,
	KEYBOARD_F22 = 0x85,
	KEYBOARD_F23 = 0x86,
	KEYBOARD_F24 = 0x87,
	KEYBOARD_NUMLOCK = 0x90,
	KEYBOARD_SCROLL = 0x91,
	KEYBOARD_LEFT_SHIFT = 0xA0,
	KEYBOARD_RIGHT_SHIFT = 0xA1,
	KEYBOARD_LEFT_CONTROL = 0xA2,
	KEYBOARD_RIGHT_CONTROL = 0xA3,
	KEYBOARD_LEFT_MENU = 0xA4,
	KEYBOARD_RIGHT_MENU = 0xA5,
	KEYBOARD_BROWSER_BACK = 0xA6,
	KEYBOARD_BROWSER_FORWARD = 0xA7,
	KEYBOARD_BROWSER_REFRESH = 0xA8,
	KEYBOARD_BROWSER_STOP = 0xA9,
	KEYBOARD_BROWSER_SEARCH = 0xAA,
	KEYBOARD_BROWSER_FAVORITES = 0xAB,
	KEYBOARD_BROWSER_HOME = 0xAC,
	KEYBOARD_VOLUME_MUTE = 0xAD,
	KEYBOARD_VOLUME_DOWN = 0xAE,
	KEYBOARD_VOLUME_UP = 0xAF,
	KEYBOARD_MEDIA_NEXT_TRACK = 0xB0,
	KEYBOARD_MEDIA_PREV_TRACK = 0xB1,
	KEYBOARD_MEDIA_STOP = 0xB2,
	KEYBOARD_MEDIA_PLAY_PAUSE = 0xB3,
	KEYBOARD_LAUNCH_MAIL = 0xB4,
	KEYBOARD_LAUNCH_MEDIA_SELECT = 0xB5,
	KEYBOARD_LAUNCH_APP1 = 0xB6,
	KEYBOARD_LAUNCH_APP2 = 0xB7,
	// (...)
};

void os_debugBreak(void);
void os_debugInfo(const char* message, RED_LOG_SEVERITY severity);

// TIME
void os_debugCounter(const char* string, int counter);

#define MEASURE_PERFORMANCE 1
#if MEASURE_PERFORMANCE
#define DEBUG_COUNTER(string, counter) (platformDebugCounter(#string, counter))
#else
#define DEBUG_COUNTER(string, counter)
#endif

i64 os_performanceCounter(void);
u64 os_rdtsc(void);

typedef enum
{
	NANOSECONDS,
	MILISECONDS,
	SECONDS,
} TIME_UNIT;

float os_getTime(TIME_UNIT timeUnit);
float os_getTimeElapsed(TIME_UNIT timeUnit);

u32 os_getLogicalCoreCount(void);


// OS GENERIC
static inline raw_str os_readFile(const char* fileFullPath)
{
	FILE* file = os_fopen(fileFullPath, "rb"); // read binary flag
	os_assert(file);

	fseek(file, 0, SEEK_END);
	long length = ftell(file); // length (bytes) of the shader bytecode
	os_assert(length >= 0);
	fseek(file, 0, SEEK_SET);

	char* fileContent = null;
	fileContent = (char*)os_malloc(length + 1);
	size_t rc = os_fread(&fileContent[0], 1, length, file);
	bool readCharacterCountMatches = rc == (size_t)length;
	os_assert(readCharacterCountMatches);
	fclose(file);
#if _DEBUG
	os_printf("FILE READ:\n%s", fileContent);
#endif

	return { fileContent, (u64)length };
}

os_window_application_pointer os_startup(const char* windowTitle, os_window_dimensions* windowDimensions);
os_window_handles os_getWindowHandles(os_window_application_pointer windowApplication);
os_window_dimensions os_getWindowDimensions(os_window_application_pointer windowApplication);
// #ifdef __cplusplus
// }
// #endif
