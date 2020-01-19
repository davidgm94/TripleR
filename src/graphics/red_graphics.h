#include "model.h"
#include "vulkan.h"

typedef enum
{
	DIRECTX_11,
	DIRECTX_12,
	OPENGL,
	VULKAN,
	METAL,
	GRAPHICS_API_COUNT
} supported_graphics_apis;

typedef struct
{
#if _WIN64

#endif
	supported_graphics_apis supportTable[GRAPHICS_API_COUNT];
} graphics_core;

void graphics_startup(void);