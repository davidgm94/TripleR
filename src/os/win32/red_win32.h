#if RRR_WIN32
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define WINMAIN_HEADER int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#define QPC(name) i64 name; (void)QueryPerformanceCounter((LARGE_INTEGER*)&name)
typedef struct
{
	HINSTANCE instance;
	HINSTANCE previousInstance;
	PSTR commandLineArguments;
	int showCommand;
} winmain_parameters;
typedef struct
{
	winmain_parameters winmainArguments;
	HWND window;
} win32_native_handles;
typedef struct
{
	bool s[4];
} win32_application_state;
typedef struct
{
	i64 performanceFrequency;
	i64 firstPerformanceCounter;
} win32_timer;
typedef struct
{
	win32_native_handles handles;
	win32_application_state applicationState;
	win32_timer timer;
} win32_window_application;

static win32_window_application win32;

static inline double getTime(i64 qpc)
{
	return (double)qpc / (double)win32.timer.performanceFrequency;
}

#define CompletePastWritesBeforeFutureWrites _WriteBarrier(); _mm_sfence()
#define CompletePastReadsBeforeFutureReads _ReadBarrier()

win32_window_application* win32_startup(const char* windowTitle, os_window_dimensions* window, win32_window_application* win32);

LRESULT CALLBACK win32_messageCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	application_state* app_info = ((application_state*)GetWindowLongPtr(window, GWLP_USERDATA));
	switch (message)
	{
		case (WM_PAINT):
		{
			(void)ValidateRect(window, nullptr);
		} break;
		case (WM_CLOSE):
		{
			if (app_info)
			{
				app_info->s[APPLICATION_STATE_FINISHED] = true;
				DestroyWindow(window);
				PostQuitMessage(0);
			}
		} break;
		case (WM_CREATE):
		{
			LPCREATESTRUCT p_create_struct = (LPCREATESTRUCT)(lParam);
			(void)SetWindowLongPtr(window, GWLP_USERDATA, (LONG_PTR)(p_create_struct->lpCreateParams));
		} break;
		default:
		{

		} break;
	}
	return DefWindowProc(window, message, wParam, lParam);
}

static inline void win32_setupConsole(const char* title)
{
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w+", stdout);
	freopen_s(&stream, "CONOUT$", "w+", stderr);
	SetConsoleTitle(TEXT(title));
}

static inline HWND win32_createWindow(HINSTANCE instance, WNDPROC window_proc, int width, int height, const char* window_title, void* pointer_to_data)
{
	WNDCLASSEX window_class = { 0 };
	window_class.cbWndExtra = 64;
	window_class.cbSize = sizeof(window_class);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = window_proc;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	window_class.lpszClassName = "Window Class";
	RegisterClassEx(&window_class);
	
	RECT window_rect = { 0, 0, width, height };
	AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	HWND window = CreateWindow(window_class.lpszClassName, window_title,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
		nullptr,        // We have no parent window.
		nullptr,        // We aren't using menus.
		instance, pointer_to_data);

	return window;
}

win32_window_application* win32_startup(const char* windowTitle, os_window_dimensions* window, win32_window_application* win32_app)
{
	win32_setupConsole(windowTitle);
	win32_app->applicationState.s[APPLICATION_STATE_FINISHED] = false;
	win32_app->applicationState.s[APPLICATION_STATE_RESIZING] = false;
	win32_app->applicationState.s[APPLICATION_STATE_PAUSED] = false;
	(void)QueryPerformanceFrequency((LARGE_INTEGER*)&win32_app->timer.performanceFrequency);
	(void)QueryPerformanceCounter((LARGE_INTEGER*)&win32_app->timer.firstPerformanceCounter);
	win32_app->handles.window = win32_createWindow(win32_app->handles.winmainArguments.instance, win32_messageCallback, (u32)window->width, (u32)window->height, windowTitle, &win32_app->applicationState);
	(void)ShowWindow(win32_app->handles.window, SW_SHOW);
	return win32_app;
}

os_window_handles os_getWindowHandles(os_window_application_pointer windowApplication)
{
	win32_window_application* win32_windowApplication = (win32_window_application*) windowApplication;
	return { win32_windowApplication->handles.window, win32_windowApplication->handles.winmainArguments.instance };
}

os_window_application_pointer os_startup(const char* windowTitle, os_window_dimensions* windowDimensions)
{
	return win32_startup(windowTitle, windowDimensions, &win32);
}
bool os_windowShouldClose(os_window_application_pointer windowApplication)
{
	win32_window_application* win32_windowApplication = (win32_window_application*) windowApplication;
	return win32_windowApplication->applicationState.s[APPLICATION_STATE_FINISHED];
}

void os_windowHandleEvents(os_window_application_pointer windowApplication)
{
	win32_window_application* win32_windowApplication = (win32_window_application*) windowApplication;
	HWND window = win32_windowApplication->handles.window;
	MSG msg;
	while (PeekMessage(&msg, window, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void os_setWindowTitle(os_window_application_pointer windowApplication, const char* title)
{
    win32_window_application* win32_windowApplication = (win32_window_application*) windowApplication;
    (void)SetWindowTextA(win32_windowApplication->handles.window, title);
}
#endif
