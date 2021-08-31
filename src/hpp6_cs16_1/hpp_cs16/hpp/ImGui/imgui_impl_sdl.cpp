#include "..\main.h"
#include "..\ProggyCleanTTF.h"

DWORD64 g_Time, g_TicksPerSecond;
GLuint g_FontTexture;

ImFont* Verdana_16px = nullptr;
ImFont* ProggyClean_13px = nullptr;

void ImGui_ImplSdl_RenderDrawLists(ImDrawData* draw_data)
{
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();

	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
		return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// We are using the OpenGL fixed pipeline to make the example code simpler to read!
	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
		glVertexPointer(3, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
			}
			idx_buffer += pcmd->ElemCount;
		}
	}

	// Restore modified state
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]); glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

bool ImGui_ImplSdlGL2_CreateDeviceObjects()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

															  // Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glGenTextures(1, &g_FontTexture);
	glBindTexture(GL_TEXTURE_2D, g_FontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	// Store our identifier
	io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

	// Restore state
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return true;
}

void ImGui_ImplSdlGL2_InvalidateDeviceObjects()
{
	if (g_FontTexture)
	{
		glDeleteTextures(1, &g_FontTexture);
		ImGui::GetIO().Fonts->TexID = 0;
		g_FontTexture = 0;
	}
}

bool ImGui_ImplSdlGL2_Init(HWND hWnd)
{
	if (!QueryPerformanceFrequency((LARGE_INTEGER*)&g_TicksPerSecond))
		return false;

	if (!QueryPerformanceCounter((LARGE_INTEGER*)&g_Time))
		return false;

	ImGuiIO& io = ImGui::GetIO();

	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	io.KeyMap[ImGuiKey_A] = 'A';
	io.KeyMap[ImGuiKey_C] = 'C';
	io.KeyMap[ImGuiKey_V] = 'V';
	io.KeyMap[ImGuiKey_X] = 'X';
	io.KeyMap[ImGuiKey_Y] = 'Y';
	io.KeyMap[ImGuiKey_Z] = 'Z';

	io.RenderDrawListsFn = ImGui_ImplSdl_RenderDrawLists;
	io.ClipboardUserData = nullptr;
	io.ImeWindowHandle = hWnd;

	ImGuiStyle& style = ImGui::GetStyle();

	style.AntiAliasedLines = false;
	style.AntiAliasedFill = false;
	style.WindowPadding = ImVec2(0.f, 0.f);
	style.PopupRounding = 0.f;
	style.FramePadding = ImVec2(4.f, 3.f);
	style.ItemSpacing = ImVec2(8.f, 4.f);
	style.ItemInnerSpacing = ImVec2(4.f, 4.f);
	style.TouchExtraPadding = ImVec2(0.f, 0.f);
	style.IndentSpacing = 21.f;
	style.ScrollbarSize = 8.f;
	style.GrabMinSize = 8.f;
	style.WindowBorderSize = 2;
	style.ChildBorderSize = 1.f;
	style.PopupBorderSize = 1.f;
	style.FrameBorderSize = 1.f;
	style.WindowRounding = 0.f;
	style.ChildRounding = 0.f;
	style.ScrollbarRounding = 0.f;
	style.GrabRounding = 0.f;
	style.WindowTitleAlign = ImVec2(0.f, 0.5f);
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

	//Themes::DefaultColorTheme();

	io.Fonts->AddFontDefault();

	ImFontConfig fontConfig;
	fontConfig.GlyphOffset.y = -2;

	Verdana_16px = io.Fonts->AddFontFromFileTTF(std::string(g_Globals.m_sSettingsPath + "verdana.ttf").c_str(), 16, &fontConfig, io.Fonts->GetGlyphRangesCyrillic());
	ProggyClean_13px = io.Fonts->AddFontFromMemoryCompressedBase85TTF(proggy_clean_ttf_compressed_data_base85, 13);

	return true;
}

void ImGui_ImplSdlGL2_Shutdown()
{
	ImGui_ImplSdlGL2_InvalidateDeviceObjects();
	ImGui::Shutdown();
}

void ImGui_ImplSdlGL2_NewFrame()
{
	if (!g_FontTexture)
		ImGui_ImplSdlGL2_CreateDeviceObjects();

	ImGuiIO& io = ImGui::GetIO();

	const float flGameScreenWidth = static_cast<float>(g_Screen.iWidth);
	const float flGameScreenHeight = static_cast<float>(g_Screen.iHeight);

	io.DisplaySize = ImVec2(flGameScreenWidth, flGameScreenHeight);
	io.DisplayFramebufferScale = ImVec2(1.0, 1.0F);

	DWORD64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);
	io.DeltaTime = (float)(currentTime - g_Time) / g_TicksPerSecond;
	g_Time = currentTime;

	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	ImGui::NewFrame();
}

LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();

	switch (msg)
	{
	case WM_LBUTTONDOWN:
		io.MouseDown[0] = true;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_RBUTTONDOWN:
		io.MouseDown[1] = true;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_RBUTTONUP:
		io.MouseDown[1] = false;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_MBUTTONDOWN:
		io.MouseDown[2] = true;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_MBUTTONUP:
		io.MouseDown[2] = false;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.f : -1.f;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_MOUSEHWHEEL:
		io.MouseWheelH += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.f : -1.f;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_MOUSEMOVE:
		io.MousePos.x = LOWORD(lParam);
		io.MousePos.y = HIWORD(lParam);
		return true;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = true;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = false;
		ImGui::Control::KeyAssingEnd();
		return true;

	case WM_CHAR:
		wchar_t wch;
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
		io.AddInputCharacter(wch);
		return true;
	}

	return false;
}
