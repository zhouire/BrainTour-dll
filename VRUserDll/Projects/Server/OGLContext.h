#pragma once

struct OGL
{
	static const bool       UseDebugContext = false;

	HWND                    Window;
	HDC                     hDC;
	HGLRC                   WglContext;
	OVR::GLEContext         GLEContext;
	bool                    Running;
	bool                    Key[256];
	int                     WinSizeW;
	int                     WinSizeH;
	GLuint                  fboId;
	HINSTANCE               hInstance;

	static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
	{
		OGL *p = reinterpret_cast<OGL *>(GetWindowLongPtr(hWnd, 0));
		switch (Msg)
		{
		case WM_KEYDOWN:
			p->Key[wParam] = true;
			break;
		case WM_KEYUP:
			p->Key[wParam] = false;
			break;
		case WM_DESTROY:
			p->Running = false;
			break;
		default:
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		}
		if ((p->Key['Q'] && p->Key[VK_CONTROL]) || p->Key[VK_ESCAPE])
		{
			p->Running = false;
		}
		return 0;
	}

	OGL() :
		Window(nullptr),
		hDC(nullptr),
		WglContext(nullptr),
		GLEContext(),
		Running(false),
		WinSizeW(0),
		WinSizeH(0),
		fboId(0),
		hInstance(nullptr)
	{
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~OGL()
	{
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindow(HINSTANCE hInst, LPCWSTR title)	
	{
		hInstance = hInst;
		Running = true;

		WNDCLASSW wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_CLASSDC;
		wc.lpfnWndProc = WindowProc;
		wc.cbWndExtra = sizeof(struct OGL *);
		wc.hInstance = GetModuleHandleW(NULL);
		wc.lpszClassName = L"ORT";
		RegisterClassW(&wc);

		// adjust the window size and show at InitDevice time
		Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
		
		if (!Window) return false;

		SetWindowLongPtr(Window, 0, LONG_PTR(this));

		hDC = GetDC(Window);

		return true;
	}

	void CloseWindow()
	{
		if (Window)
		{
			if (hDC)
			{
				ReleaseDC(Window, hDC);
				hDC = nullptr;
			}
			DestroyWindow(Window);
			Window = nullptr;
			UnregisterClassW(L"OGL", hInstance);
		}
	}

	// Note: currently there is no way to get GL to use the passed pLuid
	//bool InitDevice(int vpW, int vpH, const LUID* /*pLuid*/, bool windowed = true)
	bool InitDevice(int vpW, int vpH, bool windowed = true)
	{
		UNREFERENCED_PARAMETER(windowed);

		WinSizeW = vpW;
		WinSizeH = vpH;

		RECT size = { 0, 0, vpW, vpH };
		AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
		const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
		if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
			return false;

		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;
		{
			// First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 16;
			int pf = ChoosePixelFormat(hDC, &pfd);
			VALIDATE(pf, "Failed to choose pixel format.");

			VALIDATE(SetPixelFormat(hDC, pf, &pfd), "Failed to set pixel format.");

			HGLRC context = wglCreateContext(hDC);
			VALIDATE(context, "wglCreateContextfailed.");
			VALIDATE(wglMakeCurrent(hDC, context), "wglMakeCurrent failed.");

			wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
			wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			assert(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

			wglDeleteContext(context);
		}

		// Now create the real context that we will be using.
		int iAttributes[] =
		{
			// WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 16,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
			0, 0
		};

		float fAttributes[] = { 0, 0 };
		int   pf = 0;
		UINT  numFormats = 0;

		VALIDATE(wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats),
			"wglChoosePixelFormatARBFunc failed.");

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		VALIDATE(SetPixelFormat(hDC, pf, &pfd), "SetPixelFormat failed.");

		GLint attribs[16];
		int   attribCount = 0;
		if (UseDebugContext)
		{
			attribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
			attribs[attribCount++] = WGL_CONTEXT_DEBUG_BIT_ARB;
		}

		attribs[attribCount] = 0;

		WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
		VALIDATE(wglMakeCurrent(hDC, WglContext), "wglMakeCurrent failed.");

		OVR::GLEContext::SetCurrentContext(&GLEContext);
		GLEContext.Init();

		glGenFramebuffers(1, &fboId);

		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);

		if (UseDebugContext && GLE_ARB_debug_output)
		{
			glDebugMessageCallbackARB(DebugGLCallback, NULL);
			if (glGetError())
			{
				OVR_DEBUG_LOG(("glDebugMessageCallbackARB failed."));
			}

			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

			// Explicitly disable notification severity output.
			glDebugMessageControlARB(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		}

		return true;
	}

	/*
	bool HandleMessages(void)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return Running;
	}

	void Run(bool(*MainLoop)(bool retryCreate))
	{
		while (HandleMessages())
		{
			// true => we'll attempt to retry for ovrError_DisplayLost
			if (!MainLoop(true))
				break;
			// Sleep a bit before retrying to reduce CPU load while the HMD is disconnected
			Sleep(10);
		}
	}
	*/

	void ReleaseDevice()
	{
		if (fboId)
		{
			glDeleteFramebuffers(1, &fboId);
			fboId = 0;
		}
		if (WglContext)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(WglContext);
			WglContext = nullptr;
		}
		GLEContext.Shutdown();
	}

	static void GLAPIENTRY DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		UNREFERENCED_PARAMETER(source);
		UNREFERENCED_PARAMETER(type);
		UNREFERENCED_PARAMETER(id);
		UNREFERENCED_PARAMETER(severity);
		UNREFERENCED_PARAMETER(length);
		UNREFERENCED_PARAMETER(message);
		UNREFERENCED_PARAMETER(userParam);
		OVR_DEBUG_LOG(("Message from OpenGL: %s\n", message));
	}
};

//static OGL Platform;