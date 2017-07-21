#pragma once
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <Texture.h>

class OpenGLESRenderer
{
public:
	OpenGLESRenderer();
	~OpenGLESRenderer();
	void InitDisplay(NativeWindowType window, Texture* texture);
    void DrawFrame();
private:
	EGLConfig _config;
	EGLSurface _surface;
	EGLContext _context;
	EGLDisplay _display;
	EGLint _width, _height;
	GLuint shaderProgram;

    GLfloat verts[30] = {-0.5f, 0.5f, 0.0f,0.0f,0.0f, //Top left
                        -0.5f, -0.5f, 0.0f,0.0f,1.0f, //Bottom Left
                        0.5f, -0.5f, 0.0f,1.0f,1.0f,//Bottom Right

                        0.5f,-0.5f,0.0f,1.0f,1.0f,//Bottom right
                        0.5f,0.5f,0.0f,1.0f,0.0f,//Top right
                        -0.5f,0.5f,0.0f,0.0f,0.0f};//Top left

    GLuint VBO;
    GLuint testText;

	const char* vertexSource = R"glsl(
    attribute vec4 position;
    attribute vec2 uv;
    varying vec4 vertPos;
    varying vec2 UV;
    void main()
    {
        vertPos = position;
        UV = uv;
        gl_Position = position;
    }
	)glsl";

	const char* fragSource = R"glsl(
    precision mediump float;
    varying vec4 vertPos;
    varying vec2 UV;
    uniform sampler2D tex;
	void main()
	{
        vec4 col = texture2D(tex,vec2(UV.x,UV.y));
    	gl_FragColor = vec4(col.rgb,1.0);
	}
)glsl";



};

