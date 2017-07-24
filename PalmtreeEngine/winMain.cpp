
#ifdef _WIN32

#include <Windows.h>
#include <GL\glew.h>
#include <GL/GL.h>
#include <thread>
//Predefine windows callback
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HGLRC glContext = NULL;
HDC deviceContext = NULL;
std::thread* renderThread;

void Render()
{
	wglMakeCurrent(deviceContext, glContext);
	while (true)
	{
		if (glContext != NULL)
		{
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
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
		glewExperimental = true;
		glewInit();

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