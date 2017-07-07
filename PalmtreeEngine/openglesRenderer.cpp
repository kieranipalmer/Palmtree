#include "openglesRenderer.h"
#include <memory>


OpenGLESRenderer::OpenGLESRenderer()
{
}


OpenGLESRenderer::~OpenGLESRenderer()
{
}

void OpenGLESRenderer::InitDisplay(NativeWindowType window)
{
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};

	EGLint dummy, format;
	EGLint numConfigs;
	_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(_display, 0, 0);
	//Getsthe number of configs available for the display
	eglChooseConfig(_display, attribs, nullptr, 0, &numConfigs);

	//TODO: REPLACE WITH CUSTOM MEMORY ALLOCATOR?
	std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
	//Choose the best config out of all the available ones, ES sorts the list from best-> worse so default to best?
	eglChooseConfig(_display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
	_config = supportedConfigs[0];
	eglGetConfigAttrib(_display, _config, EGL_NATIVE_VISUAL_ID, &format);
	//TODO, what attribs can I pass in here instead of NULL?
	_surface = eglCreateWindowSurface(_display, _config, window, NULL);
	_context = eglCreateContext(_display, _config, NULL, NULL);

	if (eglMakeCurrent(_display, _surface, _surface, _context) == EGL_FALSE)
	{
		//TODO FAIL!

	}

	//Gets the width and height of the display
	eglQuerySurface(_display, _surface, EGL_WIDTH, &_width);
	eglQuerySurface(_display, _surface, EGL_HEIGHT, &_height);
	
}

void OpenGLESRenderer::DrawFrame() {
    if(!_display || !_surface)
    {
        return;
    }

    glClearColor(255,0,128,0);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(_display,_surface);
}
