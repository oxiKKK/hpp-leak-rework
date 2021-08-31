#include "main.h"

static WGLSWAPBUFFERSPROC pwglSwapBuffers = nullptr;
static GLCOLOR4FPROC pglColor4f = nullptr;

static CHookData wgl_swap_buffers;
static CHookData gl_color4f;

extern CSkipFrames g_SkipFrames;

static inline void BeginScene()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::Begin("##BackBuffer", static_cast<bool*>(0), ImVec2(), 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
	ImGui::SetWindowPos(ImVec2(), ImGuiSetCond_Always);
	ImGui::SetWindowSize(ImGui::GetIO().DisplaySize, ImGuiSetCond_Always);
}

static inline void EndScene()
{
	ImGui::GetCurrentWindow()->DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}

static void Snow()
{
	const size_t nSnowAmount = 100;
	static ImVec2 SnowPos[nSnowAmount];
	static int SnowVelocity[nSnowAmount];

	if (g_DrawGUI.IsDrawing())
	{
		for (size_t i = 0; i < nSnowAmount; i++)
		{
			if (SnowPos[i].IsZero())
			{
				SnowPos[i].x = g_Engine.pfnRandomFloat(1.0F, g_Screen.iWidth);
				SnowPos[i].y = g_Engine.pfnRandomFloat(1.0F, g_Screen.iHeight);
				SnowVelocity[i] = g_Engine.pfnRandomLong(0, 1) == 0 ? -1 : 1;
			}

			if (SnowPos[i].x >= -5.0F && SnowPos[i].x <= g_Screen.iWidth + 5.0F)
				g_Drawing.AddCircleFilled(SnowPos[i], 5.0F, ImColor(IM_COL32_WHITE), 10);

			const float speed_y = 1.5F / (ImGui::GetIO().Framerate / 100.0F);
			const float speed_x = 0.08F / (ImGui::GetIO().Framerate / 100.0F);

			SnowPos[i].y += speed_y;
			SnowPos[i].x += SnowVelocity[i] == 1 ? speed_x : -speed_x;

			if (SnowPos[i].y > g_Screen.iHeight)
			{
				SnowPos[i].x = g_Engine.pfnRandomFloat(1, g_Screen.iWidth);
				SnowPos[i].y = 0.0F;
				SnowVelocity[i] = g_Engine.pfnRandomLong(0, 1) == 0 ? -1 : 1;
			}
		}
	}
}

static inline void HandleGLSceneUpdate()
{
	static bool bInitImGui = false;

	if (g_Globals.m_bUnloadLibrary)
	{
		g_DrawGUI.Hide();

		if (!g_pIGameUI->IsGameUIActive())
		{
			g_Engine.Key_Event(K_ESCAPE, 1);
			g_Engine.Key_Event(K_ESCAPE, 0);
		}

		if (!g_pConsole->IsConsoleVisible())
			g_pConsole->Activate();

		Menu::DrawCursor();
		return;
	}

	AntiScreen();

	if (g_Globals.m_bSnapshot || g_Globals.m_bScreenshot)
		return;

	if (!bInitImGui)
	{
		ImGui_ImplSdlGL2_Init(g_Globals.m_hWnd);

		g_DrawGUI.Create("menu", Menu::DrawMenu);
		g_DrawGUI.Create("player list", Menu::DrawPlayerList);
		g_DrawGUI.GetWindowByName("menu").Show();
		g_DrawGUI.Draw();
		g_DrawGUI.FadeRunning();

		bInitImGui = true;
		return;
	}

	ImGui_ImplSdlGL2_NewFrame();

	// render scene start
	{
		BeginScene();

		if (cvar.visuals && !state.visuals_panic && client_static->state == ca_active && client_static->connect_time > 0.5)
		{
			static CVisuals visuals;
			visuals.Run();
		}

		Menu::DrawMenuFade();
		//Snow();

		EndScene();
	}
	// render scene stop

	if (!g_DrawGUI.IsDrawing())
		Menu::Tab::ClearCurrentWeaponId();

	Menu::DrawCursor();
	g_DrawGUI.Run();

	ImGui::Render();
}

static BOOL APIENTRY wglSwapBuffersHook(HDC hdc)
{
	HandleGLSceneUpdate();

	auto ret = ((WGLSWAPBUFFERSPROC)wgl_swap_buffers.origFunc)(hdc);

	if (cvar.hide_from_obs && !state.visuals_panic && !g_Globals.m_bSnapshot && !g_Globals.m_bScreenshot)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	}

	return ret;
}

static void APIENTRY glColor4fHook(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (g_Globals.m_bRenderColoredModels)
	{
		red = (g_Globals.m_iRenderType == RenderType_Material) ? (g_Globals.m_flRenderColor[0] * red) : g_Globals.m_flRenderColor[0];
		green = (g_Globals.m_iRenderType == RenderType_Material) ? (g_Globals.m_flRenderColor[1] * green) : g_Globals.m_flRenderColor[1];
		blue = (g_Globals.m_iRenderType == RenderType_Material) ? (g_Globals.m_flRenderColor[2] * blue) : g_Globals.m_flRenderColor[2];
		alpha = g_Globals.m_flRenderColor[3];
	}

	((GLCOLOR4FPROC)gl_color4f.origFunc)(red, green, blue, alpha);
}

bool HookOpenGL()
{
	HMODULE hOpenGL32 = GetModuleHandle("opengl32.dll");

	if (hOpenGL32 == INVALID_HANDLE_VALUE || hOpenGL32 == NULL)
	{
		TraceLog("> %s: handle opengl32.dll not found.\n", __FUNCTION__);
		return false;
	}

	pglColor4f = (GLCOLOR4FPROC)(GetProcAddress(hOpenGL32, "glColor4f"));

	if (pglColor4f == nullptr)
	{
		TraceLog("> %s: color4f not found.\n", __FUNCTION__);
		return false;
	}

	if (g_pISteamUtils && g_pISteamUtils->IsOverlayEnabled() && g_Offsets.gameoverlayrenderer.handle)
		pwglSwapBuffers = (WGLSWAPBUFFERSPROC)(g_Offsets.FindSteamOverlay());

	if (pwglSwapBuffers == nullptr)
		pwglSwapBuffers = (WGLSWAPBUFFERSPROC)(GetProcAddress(hOpenGL32, "wglSwapBuffers"));

	if (pwglSwapBuffers == nullptr)
	{
		TraceLog("> %s: swapbuffers not found.\n", __FUNCTION__);
		return false;
	}

	gl_color4f.Hook((FARPROC)pglColor4f, (FARPROC)glColor4fHook);
	gl_color4f.Rehook();

	wgl_swap_buffers.Hook((FARPROC)pwglSwapBuffers, (FARPROC)wglSwapBuffersHook);
	wgl_swap_buffers.Rehook();

	return true;
}

void UnHookOpenGL()
{
	wgl_swap_buffers.Unhook();
	gl_color4f.Unhook();
}