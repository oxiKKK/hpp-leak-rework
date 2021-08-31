#include "framework.h"

static void AntiScreen()
{
	constexpr auto SKIP_FRAMES_COUNT = 10;

	static auto nFramesCount = SKIP_FRAMES_COUNT;

	if (g_pGlobals->m_bSnapshot || g_pGlobals->m_bScreenshot)
	{
		if ((--nFramesCount) <= 0)
		{
			if (g_pGlobals->m_bSnapshot)
			{
				g_ClientCommandsMap["snapshot"]();
				g_pGlobals->m_bSnapshot = false;
			}
			
			if (g_pGlobals->m_bScreenshot)
			{
				g_ClientCommandsMap["screenshot"]();
				g_pGlobals->m_bScreenshot = false;
			}

			nFramesCount = SKIP_FRAMES_COUNT;
		}
	}
}

static void DrawCopyright()
{
	ImVec4 ColorWindowBg = GImGui->Style.Colors[ImGuiCol_WindowBg];
	ImVec4 ColorBorder = GImGui->Style.Colors[ImGuiCol_Border];

	if (!g_pMenu->IsOpened())
	{
		GImGui->Style.Colors[ImGuiCol_WindowBg].w *= 0.25f;
		GImGui->Style.Colors[ImGuiCol_Border].w *= 0.25f;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f);

	if (ImGui::Begin("##copyright", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize))
	{
		static float framerate, latency;
		static std::string systemtime;

		if (cvars::visuals.copyright_position[2])
		{
			ImGui::SetWindowPos(ImVec2(cvars::visuals.copyright_position[0], cvars::visuals.copyright_position[1]));
			cvars::visuals.copyright_position[2] = 0.f;
		}

		GImGui->CurrentWindow->Pos.x = std::clamp(GImGui->CurrentWindow->Pos.x, 6.f, GImGui->IO.DisplaySize.x - GImGui->CurrentWindow->Size.x - 6.f);
		GImGui->CurrentWindow->Pos.y = std::clamp(GImGui->CurrentWindow->Pos.y, 10.f, GImGui->IO.DisplaySize.y - GImGui->CurrentWindow->Size.y - 10.f);

		cvars::visuals.copyright_position[0] = GImGui->CurrentWindow->Pos.x;
		cvars::visuals.copyright_position[1] = GImGui->CurrentWindow->Pos.y;

		ImGui::PushFont(g_pFontList[Verdana_13px]);

		char timestring[9];
		_strtime_s(timestring);

		if (strnlen_s(timestring, sizeof(timestring)))
		{
			if (systemtime.empty() || systemtime.compare(timestring))
			{
				framerate = g_pGlobals->m_flFrameTime ? 1.f / g_pGlobals->m_flFrameTime : 0.f;
				latency = abs(static_cast<float>(client_state->frames[client_state->parsecountmod].latency) * 1000.f);
			}

			systemtime = timestring;

			ImGui::Text("hpp.ovh | ");
			ImGui::SameLine(0, 0);

			if (round(framerate) < 100.f)
			{
				ImGui::Text(" ");
				ImGui::SameLine(0, 0);
			}

			ImGui::Text("%.0f fps | ", framerate);
			ImGui::SameLine(0, 0);

			if (round(latency) >= 1000.f)
			{
				ImGui::Text("%.0f sec | ", latency / 1000.f);
			}
			else
			{
				if (round(latency) < 100.f)
				{
					ImGui::Text(" ");
					ImGui::SameLine(0, 0);
				}

				if (round(latency) < 10.f)
				{
					ImGui::Text(" ");
					ImGui::SameLine(0, 0);
				}

				ImGui::Text("%.0f ms | ", latency);
			}

			ImGui::SameLine(0, 0);
			ImGui::Text("%s", timestring);
		}

		ImGui::PopFont();
		ImGui::End();
	}

	ImGui::PopStyleVar();

	GImGui->Style.Colors[ImGuiCol_WindowBg] = ColorWindowBg;
	GImGui->Style.Colors[ImGuiCol_Border] = ColorBorder;
}

extern void R_DrawEntitiesOnList();

void HandleGLSceneUpdate()
{
	AntiScreen();

	if (GImGui && !Game::IsTakenScreenshot())
	{
		if (Game::IsConnected() && cvars::visuals.streamer_mode && !g_pIGameUI->IsGameUIActive())
		{
			glPushAttrib(GL_ALL_ATTRIB_BITS);

			if (g_pVisuals.get())
				g_pVisuals->CreateEntities();

			glPopAttrib();
		}

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		g_pRenderer->RenderScene([]
		{
			if (Game::IsConnected() && !IS_NULLPTR(g_pVisuals))
				g_pVisuals->Overlay();

			g_pMenu->DrawFadeBg();
			g_pMenu->CursorState();
			g_pMenu->Draw();

			g_pNotifications->Draw();
		});

		if (cvars::visuals.copyright)
			DrawCopyright();

		ImGui::Render();
	}
}