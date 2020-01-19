//
// Created by David on 19/01/2020.
//
#include "os/red_os.h"
#include "os/win32/red_win32.h"

#if RRR_WIN32
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, const char* argv[])
#endif
{
    os_window_dimensions windowDimensions = {1024, 578};
    os_window_application_pointer os_app = os_startup("Hello", &windowDimensions);

    while (!os_windowShouldClose(os_app))
    {
        os_windowHandleEvents(os_app);
    }
}