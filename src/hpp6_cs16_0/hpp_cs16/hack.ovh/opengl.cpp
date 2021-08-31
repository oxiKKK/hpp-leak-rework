#include "main.h"

GLCOLOR4FPROC pglColor4f = nullptr;
WGLSWAPBUFFERSPROC pwglSwapBuffers = nullptr;
WGLSWAPLAYERBUFFERSPROC pwglSwapLayerBuffers = nullptr;

CHookData glHookwglSwapBuffers;
CHookData glHookwglSwapLayerBuffers;
CHookData glHookglColor4f;

bool InitImGui = false;

void InitCheatMenu()
{
	CWindow window;

	window.Show();
	window.PushWindow(CheatMenu);

	g_DrawGUI.GetWindow().push_back(window);
}

void HandleGLSceneUpdate()
{
	static auto r_norefresh = g_Engine.pfnGetCvarPointer("r_norefresh");

	AntiScreen();

	if (g_Globals.m_bSnapshot || g_Globals.m_bScreenshot)
		return;

	if (InitImGui)
	{
		if (g_Globals.m_bUnloading)
		{
			if (g_DrawGUI.IsDrawing())
				g_DrawGUI.Hide();
		}

		if (!r_norefresh)
			r_norefresh = g_Engine.pfnGetCvarPointer("r_norefresh");

		if (r_norefresh && !r_norefresh->value)
		{
			ImGui_ImplSdlGL2_NewFrame();

			g_RenderScene.Begin();
			g_RenderScene.Render();
			g_RenderScene.End();

			g_DrawGUI.Frame();
			g_DrawGUI.Render();

			ImGui::Render();
		}
	}
	else
	{
		ImGui_ImplSdlGL2_Init(g_hWnd);
		InitCheatMenu();

		g_DrawGUI.RunFadeout();
		g_DrawGUI.Draw();

		InitImGui = true;
	}
}

static void APIENTRY glColor4fHook(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (g_Globals.m_bRenderColoredModels)
	{
		red = (g_Globals.m_iRenderType == RenderType_Material) ? (g_Globals.m_flRenderColor[0] * red) : g_Globals.m_flRenderColor[0];
		green = (g_Globals.m_iRenderType == RenderType_Material) ? (g_Globals.m_flRenderColor[1] * green) : g_Globals.m_flRenderColor[1];
		blue = (g_Globals.m_iRenderType == RenderType_Material) ? (g_Globals.m_flRenderColor[2] * blue) : g_Globals.m_flRenderColor[2];
	}

	((GLCOLOR4FPROC)glHookglColor4f.origFunc)(red, green, blue, alpha);
}

static BOOL APIENTRY wglSwapBuffersHook(HDC hdc)
{
	HandleGLSceneUpdate();

	BOOL bResult = ((WGLSWAPBUFFERSPROC)glHookwglSwapBuffers.origFunc)(hdc);

	if (!g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return bResult;
}

static BOOL WINAPI wglSwapLayerBuffersHook(HDC hdc)
{
	HandleGLSceneUpdate();

	BOOL bResult = ((WGLSWAPLAYERBUFFERSPROC)glHookwglSwapLayerBuffers.origFunc)(hdc);

	if (!g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return bResult;
}

bool HookOpenGL()
{
	HMODULE hGL = GetModuleHandle(TEXT("opengl32.dll"));

	if (hGL == INVALID_HANDLE_VALUE)
	{
		TraceLog("> %s: handle opengl32.dll not found.\n", __FUNCTION__);
		return false;
	}

	bool bSuccess = false;

	pwglSwapLayerBuffers = (WGLSWAPLAYERBUFFERSPROC)g_Offsets.FindSteamOverlay();

	if (!pwglSwapLayerBuffers)
		pwglSwapBuffers = (WGLSWAPBUFFERSPROC)GetProcAddress(hGL, "wglSwapBuffers");
	else
		TraceLog("> %s: hooking gameoverlayrenderer.dll.\n", __FUNCTION__);

	pglColor4f = (GLCOLOR4FPROC)GetProcAddress(hGL, "glColor4f");

	if ((pwglSwapLayerBuffers || pwglSwapBuffers) && pglColor4f)
	{
		if (!pwglSwapLayerBuffers)
			glHookwglSwapBuffers.Hook((FARPROC)pwglSwapBuffers, (FARPROC)wglSwapBuffersHook);
		else
			glHookwglSwapLayerBuffers.Hook((FARPROC)pwglSwapLayerBuffers, (FARPROC)wglSwapLayerBuffersHook);

		glHookglColor4f.Hook((FARPROC)pglColor4f, (FARPROC)glColor4fHook);

		bSuccess = true;
	}

	if (bSuccess)
	{
		if (!pwglSwapLayerBuffers)
			glHookwglSwapBuffers.Rehook();
		else
			glHookwglSwapLayerBuffers.Rehook();

		glHookglColor4f.Rehook();
	}

	return bSuccess;
}

bool UnHookOpenGL()
{
	glHookwglSwapBuffers.Unhook();
	glHookwglSwapLayerBuffers.Unhook();
	glHookglColor4f.Unhook();
	return true;
}