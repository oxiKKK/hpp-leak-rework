#include "framework.h"

static CHookData swapwindow_hook;
static CHookData swapbuffers_hook;
static CHookData glcolor4f_hook;

extern void HandleGLSceneUpdate();

static void PrepareForUnloading()
{
	g_pMenu->Close();
	g_pMenu->CursorState();
}

static BOOL APIENTRY wglSwapBuffers(HDC hdc)
{
	if (!swapbuffers_hook.IsHooked())
		return TRUE;

	g_pGlobals->m_bIsUnloadingLibrary
		? PrepareForUnloading()
		: HandleGLSceneUpdate();

	using swapbuffers_t = BOOL(WINAPI*)(HDC);

	auto ret = reinterpret_cast<swapbuffers_t>(swapbuffers_hook.m_OrigFunc)(hdc);

	if (cvars::visuals.streamer_mode && !Game::IsTakenScreenshot())
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return ret;
}

static void SDL_GL_SwapWindow(int a1)
{
	if (!swapwindow_hook.IsHooked())
		return;

	g_pGlobals->m_bIsUnloadingLibrary
		? PrepareForUnloading()
		: HandleGLSceneUpdate();
	
	using swapwindow_t = void(__cdecl*)(int);

	reinterpret_cast<swapwindow_t>(swapwindow_hook.m_OrigFunc)(a1);

	if (cvars::visuals.streamer_mode && !Game::IsTakenScreenshot())
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}
}

static void SetRenderColoredModels(GLfloat& r, GLfloat& g, GLfloat& b, GLfloat& a)
{
	switch (g_pGlobals->m_iRenderType)
	{
	case FLAT:
	case TEXTURE:
		r = g_pGlobals->m_flRenderColor[0];
		g = g_pGlobals->m_flRenderColor[1];
		b = g_pGlobals->m_flRenderColor[2];
		break;
	case DARKENED:
		r = g_pGlobals->m_flRenderColor[0] * r;
		g = g_pGlobals->m_flRenderColor[1] * g;
		b = g_pGlobals->m_flRenderColor[2] * b;
		break;
	case LIGHTED:
		r = g_pGlobals->m_flRenderColor[0] / (r == 0.f ? 0.01f : r);
		g = g_pGlobals->m_flRenderColor[1] / (g == 0.f ? 0.01f : g);
		b = g_pGlobals->m_flRenderColor[2] / (b == 0.f ? 0.01f : b);
	}

	a = g_pGlobals->m_flRenderColor[3];
}

static void APIENTRY _glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
	if (!glcolor4f_hook.IsHooked())
		return;

	if (g_pGlobals->m_bRenderModels)
		SetRenderColoredModels(r, g, b, a);

	using glcolor4f_t = void(WINAPI*)(GLfloat, GLfloat, GLfloat, GLfloat);
	reinterpret_cast<glcolor4f_t>(glcolor4f_hook.m_OrigFunc)(r, g, b, a);
}

static bool HookSwapBuffers()
{
	HMODULE hOpenGL32 = GetModuleHandle("opengl32.dll");

	if (!hOpenGL32 || hOpenGL32 == INVALID_HANDLE_VALUE)
	{
		Utils::TraceLog(V("> %s: handle opengl32.dll not found.\n"), V(__FUNCTION__));
		return false;
	}

	FARPROC SwapBuffersProc = GetProcAddress(hOpenGL32, "wglSwapBuffers");

	if (!SwapBuffersProc)
	{
		Utils::TraceLog(V("> %s: wglSwapBuffers not found.\n"), V(__FUNCTION__));
		return false;
	}

	MAKE_HOOK(swapbuffers_hook, SwapBuffersProc, wglSwapBuffers);

	return true;
}

static bool HookglColor4f()
{
	HMODULE hOpenGL32 = GetModuleHandle("opengl32.dll");

	if (!hOpenGL32 || hOpenGL32 == INVALID_HANDLE_VALUE)
	{
		Utils::TraceLog(V("> %s: handle opengl32.dll not found.\n"), V(__FUNCTION__));
		return false;
	}

	FARPROC glColor4fProc = GetProcAddress(hOpenGL32, "glColor4f");

	if (!glColor4fProc)
	{
		Utils::TraceLog(V("> %s: glColor4f not found.\n"), V(__FUNCTION__));
		return false;
	}

	MAKE_HOOK(glcolor4f_hook, glColor4fProc, _glColor4f);

	return true;
}

static bool HookSwapWindow()
{
	HMODULE hSDL2 = GetModuleHandle("SDL2.dll");

	if (!hSDL2 || hSDL2 == INVALID_HANDLE_VALUE)
	{
		Utils::TraceLog(V("> %s: handle SDL2.dll not found.\n"), V(__FUNCTION__));
		return false;
	}

	FARPROC SwapWindowProc = GetProcAddress(hSDL2, "SDL_GL_SwapWindow");

	if (!SwapWindowProc)
	{
		Utils::TraceLog(V("> %s: SDL_GL_SwapWindow not found.\n"), V(__FUNCTION__));
		return false;
	}

	MAKE_HOOK(swapwindow_hook, SwapWindowProc, SDL_GL_SwapWindow);

	return true;
}

void HookControllerGL()
{
	if (cvars::visuals.streamer_mode || !g_pGlobals->m_bIsSDL2Hooked)
	{
		RESET_HOOK(swapwindow_hook);

		if (!swapbuffers_hook.IsHooked() && !HookSwapBuffers())
			g_pGlobals->m_bIsUnloadingLibrary = true;
	}
	else
	{
		RESET_HOOK(swapbuffers_hook);

		if (!swapwindow_hook.IsHooked() && !HookSwapWindow())
			g_pGlobals->m_bIsUnloadingLibrary = true;
	}

	if (Game::IsConnected() && !IS_NULLPTR(g_pVisuals))
	{
		if (!glcolor4f_hook.IsHooked() && !HookglColor4f())
			g_pGlobals->m_bIsUnloadingLibrary = true;
	}
	else
	{
		RESET_HOOK(glcolor4f_hook);
	}
}

bool HookOpenGL()
{
	g_pRenderer = std::make_unique<CRenderer>();
	g_pMenu = std::make_unique<CMenu>();
	g_pNotifications = std::make_unique<CNotifications>();

	g_pGlobals->m_bIsSDL2Hooked = HookSwapWindow();

	if (!HookSwapBuffers())
		return false;

	if (!HookglColor4f())
		return false;

	return true;
}

void UnHookOpenGL()
{
	RESET_HOOK(swapbuffers_hook);
	RESET_HOOK(swapwindow_hook);
	RESET_HOOK(glcolor4f_hook);
}