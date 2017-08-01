
#ifdef _WIN32

#include <Windows.h>
#include <GL\glew.h>
#include <GL/GL.h>
#include <thread>
#include <fstream>
#include "Texture.h"
#include "PNGParser.h"
//Predefine windows callback
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HGLRC glContext = NULL;
HDC deviceContext = NULL;
std::thread* renderThread;

const char* vertexSource = R"glsl(
    #version 150 core

	in vec3 position;
	in vec2 uv;
	out vec2 Texcoord;

	void main()
	{
		Texcoord = uv;
		gl_Position = vec4(position, 1.0);
	}
	)glsl";

const char* fragSource = R"glsl(
   #version 150 core
	in vec2 Texcoord;
	out vec4 outColor;
	uniform sampler2D tex;
	void main()
	{
		vec4 col = texture(tex,Texcoord);
		outColor = vec4(col.xyz,1.0);
	}
)glsl";

void Render()
{
	int tmp;

	// Get the current bits  
	tmp = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

	// Clear the upper 16 bits and OR in the desired freqency  
	tmp = (tmp & 0x0000FFFF) | _CRTDBG_CHECK_ALWAYS_DF;

	// Set the new bits  
	_CrtSetDbgFlag(tmp);

	wglMakeCurrent(deviceContext, glContext);
	glewExperimental = true;
	glewInit();

	GLfloat verts[30] = { -0.5f, 0.5f, 0.0f,0.0f,0.0f, //Top left
						 -0.5f, -0.5f, 0.0f,0.0f,1.0f, //Bottom Left
						0.5f, -0.5f, 0.0f,1.0f,1.0f,//Bottom Right

						0.5f,-0.5f,0.0f,1.0f,1.0f,//Bottom right
						0.5f,0.5f,0.0f,1.0f,0.0f,//Top right
						-0.5f,0.5f,0.0f,0.0f,0.0f };//Top left

	//Upload vert data to GPU
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	//Create shaders
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);


	GLint vertexShaderStatus;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderStatus);
	char vBuffer[512];
	glGetShaderInfoLog(vertexShader, 512, NULL, vBuffer);

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, NULL);
	glCompileShader(fragShader);

	GLint fragShaderStatus;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &fragShaderStatus);
	char fBuffer[512];
	glGetShaderInfoLog(fragShader, 512, NULL, fBuffer);


	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragShader);
	//Binds the output of the fragment shader to the correct output buffer (0) 
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);

	GLint linked;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint infolen = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1)
		{
			char* infolog = (char*)malloc(sizeof(char)*infolen);
			glGetProgramInfoLog(shaderProgram, infolen, NULL, infolog);
		}
	}

	glUseProgram(shaderProgram);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Open file at the EOF
	std::ifstream file("C:/Users/kiera/Pictures/f01n0g08.png", std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	uint8_t* buffer = (uint8_t*)malloc(size);
	if (!file.read((char*)buffer, size))
	{
		return;
	}
	Texture* testText = PNGParser::ParsePNG(buffer);
	free(buffer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, testText->GetWidth(), testText->GetHeight(), 0, GL_RGBA, GL_FLOAT, testText->_pixelData);
	delete testText;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(glGetAttribLocation(shaderProgram, "uv"), 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glUniform1i(glGetUniformLocation(shaderProgram, "tex"), 0);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	while (true)
	{
		if (glContext != NULL)
		{
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			SwapBuffers(deviceContext);
		}
	}
}

int wmain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
	MSG msg = { 0 };
	//Windows class describing the window
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = L"PalmtreeWindowsTest";
	wc.style = CS_OWNDC;

	if (!RegisterClass(&wc))
	{
		return 1;
	}

  	CreateWindow(wc.lpszClassName, L"Palmtree Engine", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, hInstance, 0);

	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
			32,                        //Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                        //Number of bits for the depthbuffer
			8,                        //Number of bits for the stencilbuffer
			0,                        //Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		HDC windowHandleToDeviceContext = GetDC(hwnd);
		int letWindowsChoosePixelFormat;
		letWindowsChoosePixelFormat = ChoosePixelFormat(windowHandleToDeviceContext, &pfd);
		SetPixelFormat(windowHandleToDeviceContext, letWindowsChoosePixelFormat, &pfd);
		HGLRC glRenderingContext = wglCreateContext(windowHandleToDeviceContext);
		

		glContext = glRenderingContext;
		deviceContext = windowHandleToDeviceContext;
		renderThread = new std::thread(Render);

	}
	break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}
#endif