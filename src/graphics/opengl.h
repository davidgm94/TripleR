#pragma once

//void win32_initOpenGL(HWND window)
//{
//	HDC DC = GetWindowDC(window);
	
	// TODO: fill in the future with better accuracy with what we want
//	PIXELFORMATDESCRIPTOR desiredPixelFormat = {0};
//	desiredPixelFormat.nSize = sizeof(desiredPixelFormat);
//	desiredPixelFormat.nVersion = 1;
//	desiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
//	desiredPixelFormat.iPixelType;
//	desiredPixelFormat.cColorBits = 32;
//	desiredPixelFormat.cRedBits;
//	desiredPixelFormat.cRedShift;
//	desiredPixelFormat.cGreenBits;
//	desiredPixelFormat.cGreenShift;
//	desiredPixelFormat.cBlueBits;
//	desiredPixelFormat.cBlueShift;
//	desiredPixelFormat.cAlphaBits = 8;
//	desiredPixelFormat.cAlphaShift;
//	desiredPixelFormat.cAccumBits;
//	desiredPixelFormat.cAccumRedBits;
//	desiredPixelFormat.cAccumGreenBits;
//	desiredPixelFormat.cAccumBlueBits;
//	desiredPixelFormat.cAccumAlphaBits;
//	desiredPixelFormat.cDepthBits;
//	desiredPixelFormat.cStencilBits;
//	desiredPixelFormat.cAuxBuffers;
//	desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
//	desiredPixelFormat.bReserved;
//	desiredPixelFormat.dwLayerMask;
//	desiredPixelFormat.dwVisibleMask;
//	desiredPixelFormat.dwDamageMask;
//
//	int suggestedPixelFormatIndex = ChoosePixelFormat(DC, &desiredPixelFormat);
//	PIXELFORMATDESCRIPTOR suggestedPixelFormatDescriptor;
//	(void)DescribePixelFormat(DC, suggestedPixelFormatIndex, sizeof(suggestedPixelFormatDescriptor), &suggestedPixelFormatDescriptor);
//	(void)SetPixelFormat(DC, suggestedPixelFormatIndex, &suggestedPixelFormatDescriptor);
//
//	HGLRC openGlRenderingContext = wglCreateContext(DC);
//	if (wglMakeCurrent(DC, openGlRenderingContext))
//	{
//		os_printf("OpenGL initialized\n");
//	}
//	else
//	{
//		os_printf("OpenGL failed to initialize\n");
//	}
//}