//
// Created by David on 19/01/2020.
//
#include "os/red_os.h"
#include "os/win32/red_win32.h"
#include "graphics/red_graphics.h"

void compileShaders()
{
    char shaders[MAX_FILENAME_LENGTH][5];

    const char* shaderNames[] =
    {
        "triangle.vert",
        "triangle.frag",
        "model.vert",
        "model.frag",
        "model.mesh",
    };

    const char* shaderOutputNames[] =
    {
        "triangle.vert.spv",
        "triangle.frag.spv",
        "model.vert.spv",
        "model.frag.spv",
        "model.mesh.spv",
    };

    const char* commandName = "glslangValidator %s -V -o %s";

    char buffer[MAX_FILENAME_LENGTH];
    char bufferOutput[MAX_FILENAME_LENGTH];

    for (u32 i = 0; i < ARRAYCOUNT(shaderNames); i++)
    {
        char out[MAX_FILENAME_LENGTH];
        os_strcpy(buffer, shaderDirectory);
        os_strcpy(bufferOutput, buffer);
        os_appendFileOrDirectory(buffer, shaderNames[i]);
        os_appendFileOrDirectory(bufferOutput, shaderOutputNames[i]);
        os_sprintf(out, commandName, buffer, bufferOutput);
        system(out);
    }
}

#if RRR_WIN32
WINMAIN_HEADER
#else
int main(int argc, const char* argv[])
#endif
{
    os_window_dimensions windowDimensions = {1024, 578};
    os_window_application_pointer os_app = os_startup("RedEngine", &windowDimensions);
    os_window_handles handles = os_getWindowHandles(os_app);

    os_fillCurrentWorkingDirectory();
    os_fillShadersDirectory();
    os_fillAssetsDirectory();
#if RRR_WIN32
    compileShaders();
#endif

    vulkan_renderer vk;
    vk_load(&vk, &handles, &windowDimensions);

    while (!os_windowShouldClose(os_app))
    {
        os_windowHandleEvents(os_app);
        vk_render(&vk);
    }
}