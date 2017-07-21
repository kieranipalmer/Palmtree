#include "openglesRenderer.h"
#include <memory>
#include <cstdlib>


OpenGLESRenderer::OpenGLESRenderer()
{
}


OpenGLESRenderer::~OpenGLESRenderer()
{
}

void OpenGLESRenderer::InitDisplay(NativeWindowType window,Texture* texture)
{
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE }; // OpenGL 2.0
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
	_context = eglCreateContext(_display, _config, NULL, contextAttribs);

	if (eglMakeCurrent(_display, _surface, _surface, _context) == EGL_FALSE)
	{
		//TODO FAIL!

	}

	//Gets the width and height of the display
	eglQuerySurface(_display, _surface, EGL_WIDTH, &_width);
	eglQuerySurface(_display, _surface, EGL_HEIGHT, &_height);

     //setEGLContextClientVersion(2);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader,1,&vertexSource,NULL);
	glCompileShader(vertexShader);

    GLint vertexShaderStatus;
    glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&vertexShaderStatus);
    char vBuffer[512];
    glGetShaderInfoLog(vertexShader,512,NULL,vBuffer);

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader,1,&fragSource,NULL);
	glCompileShader(fragShader);

    GLint fragShaderStatus;
    glGetShaderiv(fragShader,GL_COMPILE_STATUS,&fragShaderStatus);
    char fBuffer[512];
    glGetShaderInfoLog(fragShader,512,NULL,fBuffer);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram,vertexShader);
	glAttachShader(shaderProgram,fragShader);
	glLinkProgram(shaderProgram);

    GLint linked;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

    if(!linked)
    {
        GLint infolen=0;
        glGetProgramiv(shaderProgram,GL_INFO_LOG_LENGTH,&infolen);
        if(infolen > 1)
        {
            char* infolog = (char*)malloc(sizeof(char)*infolen);
            glGetProgramInfoLog(shaderProgram,infolen,NULL,infolog);
        }
    }

    glUseProgram(shaderProgram);

    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);

    glGenTextures(1,&testText);
    glBindTexture(GL_TEXTURE_2D,testText);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->GetWidth(), texture->GetHeight(), 0, GL_RGBA, GL_FLOAT, texture->_pixelData);
}

void OpenGLESRenderer::DrawFrame() {
    if(!_display || !_surface)
    {
        return;
    }

    glViewport(0,0,_width,_height);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);

    glUseProgram(shaderProgram);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),0);
    glVertexAttribPointer(glGetAttribLocation(shaderProgram,"uv"),2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float)));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,testText);
    GLint texUniform = glGetUniformLocation(shaderProgram, "tex");
    glUniform1i(texUniform, 0);

	glDrawArrays(GL_TRIANGLES,0,6);

	eglSwapBuffers(_display,_surface);
}
