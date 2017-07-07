#pragma once
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>


class OpenGLESRenderer
{
public:
	OpenGLESRenderer();
	~OpenGLESRenderer();
	void InitDisplay(NativeWindowType window);
    void DrawFrame();
private:
	EGLConfig _config;
	EGLSurface _surface;
	EGLContext _context;
	EGLDisplay _display;
	EGLint _width, _height;

};

