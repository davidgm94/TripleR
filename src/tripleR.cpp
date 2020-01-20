//
// Created by David on 19/01/2020.
//
#include "os/red_os.h"
#include "os/win32/red_win32.h"
#include "graphics/red_graphics.h"

#if RRR_WIN32
WINMAIN_HEADER
#else
int main(int argc, const char* argv[])
#endif
{
    os_window_dimensions windowDimensions = {1024, 578};
    os_window_application_pointer os_app = os_startup("RedEngine", &windowDimensions);
    os_window_handles handles = os_getWindowHandles(os_app);

    vulkan_renderer vk;
    vk_load(&vk, &handles, &windowDimensions);

    while (!os_windowShouldClose(os_app))
    {
        os_windowHandleEvents(os_app);
        vk_render(&vk);
    }
}