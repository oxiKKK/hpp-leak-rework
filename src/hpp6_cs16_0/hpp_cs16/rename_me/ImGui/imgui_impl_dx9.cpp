#include "..\Main.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "Font_LucidaGrande.h"

static INT64                    g_Time = 0;
static INT64                    g_TicksPerSecond = 0;
static LPDIRECT3DVERTEXBUFFER9  g_pVB = NULL;
static LPDIRECT3DINDEXBUFFER9   g_pIB = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;
static bool						g_bWindowMove = false;
static POINT					g_MousePos;

struct CUSTOMVERTEX
{
	float    pos[3];
	D3DCOLOR col;
	float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

ImFont* Lucida11px;
ImFont* Lucida18px;

void ImGui_ImplDX9_RenderDrawLists(ImDrawData* draw_data)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
		return;

	if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
	{
		if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
		g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
		if (g_lpD3D9Device->CreateVertexBuffer(g_VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL) < 0)
			return;
	}
	if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
	{
		if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
		g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
		if (g_lpD3D9Device->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, NULL) < 0)
			return;
	}

	IDirect3DStateBlock9* d3d9_state_block = NULL;
	if (g_lpD3D9Device->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
		return;

	CUSTOMVERTEX* vtx_dst;
	ImDrawIdx* idx_dst;
	if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
		return;
	if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
		return;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
		for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
		{
			vtx_dst->pos[0] = vtx_src->pos.x;
			vtx_dst->pos[1] = vtx_src->pos.y;
			vtx_dst->pos[2] = 0.0f;
			vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
			vtx_dst->uv[0] = vtx_src->uv.x;
			vtx_dst->uv[1] = vtx_src->uv.y;
			vtx_dst++;
			vtx_src++;
		}
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	g_pVB->Unlock();
	g_pIB->Unlock();
	g_lpD3D9Device->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	g_lpD3D9Device->SetIndices(g_pIB);
	g_lpD3D9Device->SetFVF(D3DFVF_CUSTOMVERTEX);

	D3DVIEWPORT9 vp;
	vp.X = vp.Y = 0;
	vp.Width = (DWORD)io.DisplaySize.x;
	vp.Height = (DWORD)io.DisplaySize.y;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;
	g_lpD3D9Device->SetViewport(&vp);

	g_lpD3D9Device->SetPixelShader(NULL);
	g_lpD3D9Device->SetVertexShader(NULL);
	g_lpD3D9Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_lpD3D9Device->SetRenderState(D3DRS_LIGHTING, false);
	g_lpD3D9Device->SetRenderState(D3DRS_ZENABLE, false);
	g_lpD3D9Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	g_lpD3D9Device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	g_lpD3D9Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	g_lpD3D9Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_lpD3D9Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_lpD3D9Device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	g_lpD3D9Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_lpD3D9Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_lpD3D9Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_lpD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	g_lpD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	g_lpD3D9Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	g_lpD3D9Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	g_lpD3D9Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	{
		const float L = 0.5f, R = io.DisplaySize.x + 0.5f, T = 0.5f, B = io.DisplaySize.y + 0.5f;
		D3DMATRIX mat_identity = { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } };
		D3DMATRIX mat_projection =
		{
			2.0f / (R - L),   0.0f,         0.0f,  0.0f,
			0.0f,         2.0f / (T - B),   0.0f,  0.0f,
			0.0f,         0.0f,         0.5f,  0.0f,
			(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f,
		};
		g_lpD3D9Device->SetTransform(D3DTS_WORLD, &mat_identity);
		g_lpD3D9Device->SetTransform(D3DTS_VIEW, &mat_identity);
		g_lpD3D9Device->SetTransform(D3DTS_PROJECTION, &mat_projection);
	}

	int vtx_offset = 0;
	int idx_offset = 0;
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
				g_lpD3D9Device->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
				g_lpD3D9Device->SetScissorRect(&r);
				g_lpD3D9Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount / 3);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}

	d3d9_state_block->Apply();
	d3d9_state_block->Release();
}

static bool IsAnyMouseButtonDown()
{
	ImGuiIO& io = ImGui::GetIO();

	for (int n = 0; n < IM_ARRAYSIZE(io.MouseDown); ++n)
		if (io.MouseDown[n])
			return true;

	return false;
}

IMGUI_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();

	switch (msg)
	{
	case WM_LBUTTONDOWN:	
		SetCapture(g_hWnd);
		GetCursorPos(&g_MousePos);
	
		io.MouseDown[0] = true;
		g_bWindowMove = true;
		return true;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	{
		int button = 0;
		if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) button = 0;
		if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) button = 1;
		if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) button = 2;

		if (!IsAnyMouseButtonDown() && GetCapture() == NULL)
			SetCapture(hwnd);

		io.MouseDown[button] = true;
		return 0;
	}

	case WM_LBUTTONUP:
		io.MouseDown[0] = false;
		g_bWindowMove = false;

		ReleaseCapture();
		return true;

	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	{
		int button = 0;
		if (msg == WM_LBUTTONUP) button = 0;
		if (msg == WM_RBUTTONUP) button = 1;
		if (msg == WM_MBUTTONUP) button = 2;

		io.MouseDown[button] = false;

		if (!IsAnyMouseButtonDown() && GetCapture() == hwnd)
			ReleaseCapture();

		return 0;
	}
	case WM_MOUSEWHEEL:
		io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.f : -1.f;
		return 0;

	case WM_MOUSEHWHEEL:
		io.MouseWheelH += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.f : -1.f;
		return 0;

	case WM_MOUSEMOVE:
		io.MousePos.x = (signed short)(lParam);
		io.MousePos.y = (signed short)(lParam >> 16);

		if (g_bWindowMove)
		{
			POINT pTmp;
			GetCursorPos(&pTmp);

			RECT R;
			GetWindowRect(g_hWnd, &R);

			if (g_MousePos.y < R.top + 30)
			{
				MoveWindow(g_hWnd,
					R.left + pTmp.x - g_MousePos.x,
					R.top + pTmp.y - g_MousePos.y,
					R.right - R.left,
					R.bottom - R.top, true);

				g_MousePos = pTmp;
			}
		}
		return 0;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (wParam < 256)
			io.KeysDown[wParam] = 1;
		return 0;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		if (wParam < 256)
			io.KeysDown[wParam] = 0;
		return 0;

	case WM_CHAR:
		wchar_t wch;
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
		io.AddInputCharacter(wch);
	}

	return 0;
}

bool ImGui_ImplDX9_Init()
{
	if (!QueryPerformanceFrequency((LARGE_INTEGER*)&g_TicksPerSecond))
		return false;

	if (!QueryPerformanceCounter((LARGE_INTEGER*)&g_Time))
		return false;

	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2(WinWidth, WinHeight);
	io.KeyMap[ImGuiKey_Tab] = VK_TAB;
	io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	io.KeyMap[ImGuiKey_Home] = VK_HOME;
	io.KeyMap[ImGuiKey_End] = VK_END;
	io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
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

	io.RenderDrawListsFn = ImGui_ImplDX9_RenderDrawLists;
	io.ImeWindowHandle = g_hWnd;

	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1;									// Global alpha applies to everything in ImGui
	style.WindowPadding = ImVec2(0, 0);					// Padding within a window
	style.WindowMinSize = ImVec2(32, 32);				// Minimum window size
	style.WindowRounding = 0;							// Radius of window corners rounding. Set to 0.0f to have rectangular windows
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.WindowTitleAlign = ImVec2(0, 0.5f);			// Alignment for title bar text
	style.FramePadding = ImVec2(4, 3);					// Padding within a framed rectangle (used by most widgets)
	style.FrameRounding = 6;							// Radius of frame corners rounding. Set to 0.0f to have rectangular frames (used by most widgets).
	style.FrameBorderSize = 1;
	style.ItemSpacing = ImVec2(8, 4);					// Horizontal and vertical spacing between widgets/lines
	style.ItemInnerSpacing = ImVec2(4, 4);				// Horizontal and vertical spacing between within elements of a composed widget (e.g. a slider and its label)
	style.TouchExtraPadding = ImVec2(0, 0);				// Expand reactive bounding box for touch-based system where touch position is not accurate enough. Unfortunately we don't sort widgets so priority on overlap will always be given to the first widget. So don't grow this too much!
	style.IndentSpacing = 21;							// Horizontal spacing when e.g. entering a tree node. Generally == (FontSize + FramePadding.x*2).
	style.ColumnsMinSpacing = 6;						// Minimum horizontal spacing between two columns
	style.ScrollbarSize = 14;							// Width of the vertical scrollbar, Height of the horizontal scrollbar
	style.ScrollbarRounding = 16;						// Radius of grab corners rounding for scrollbar
	style.GrabMinSize = 10;								// Minimum width/height of a grab box for slider/scrollbar
	style.GrabRounding = 2;								// Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);			// Alignment of button text when button is larger than text.
	style.DisplayWindowPadding = ImVec2(22, 22);		// Window positions are clamped to be visible within the display area by at least this amount. Only covers regular windows.
	style.DisplaySafeAreaPadding = ImVec2(4, 4);		// If you cannot see the edge of your screen (e.g. on a TV) increase the safe area padding. Covers popups/tooltips as well regular windows.
	style.AntiAliasedLines = true;						// Enable anti-aliasing on lines/borders. Disable if you are really short on CPU/GPU.
	style.CurveTessellationTol = 1.25f;					// Tessellation tolerance. Decrease for highly tessellated curves (higher quality, more polygons), increase to reduce quality.

	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.91f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.98f, 0.98f, 0.99f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.08f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.05f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.05f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.29f, 0.29f, 0.29f, 0.52f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.39f, 0.39f, 0.39f, 0.8f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 0.08f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.92f, 0.29f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.92f, 0.29f, 0.26f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.32f, 0.30f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.36f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

	io.Fonts->AddFontDefault();

	ImFontConfig fontConfig;

	fontConfig.OversampleH = fontConfig.OversampleV = 3;
	fontConfig.PixelSnapH = true;

	static const ImWchar ranges[] = { 0x0020, 0x00FF, 0x0400, 0x044F, 0 };

	Lucida11px = io.Fonts->AddFontFromMemoryCompressedTTF(lucida_compressed_data, lucida_compressed_size, 12, &fontConfig, ranges);
	Lucida18px = io.Fonts->AddFontFromMemoryCompressedTTF(lucida_compressed_data, lucida_compressed_size, 18, &fontConfig, ranges);

	return true;
}

void ImGui_ImplDX9_Shutdown()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui::Shutdown();
}

bool ImGui_ImplDX9_CreateFontsTexture()
{
	unsigned char* pixels;
	int width, height, bytesPerPixel;

	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);

	g_FontTexture = nullptr;

	if (g_lpD3D9Device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_FontTexture, 0) < 0)
		return false;

	D3DLOCKED_RECT tex_locked_rect;

	if (g_FontTexture->LockRect(0, &tex_locked_rect, 0, 0) != D3D_OK)
		return false;

	for (int y = 0; y < height; ++y)
		memcpy((unsigned char*)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytesPerPixel) * y, (width * bytesPerPixel));

	g_FontTexture->UnlockRect(0);

	io.Fonts->TexID = (void*)g_FontTexture;

	return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects()
{
	if (!g_lpD3D9Device)
		return;

	if (g_pVB)
	{
		g_pVB->Release();
		g_pVB = nullptr;
	}

	if (g_pIB)
	{
		g_pIB->Release();
		g_pIB = nullptr;
	}

	if (g_FontTexture)
	{
		g_FontTexture->Release();
		g_FontTexture = nullptr;
	}

	ImGui::GetIO().Fonts->TexID = nullptr;
}

bool ImGui_ImplDX9_CreateDeviceObjects()
{
	if (!g_lpD3D9Device)
		return false;

	if (!ImGui_ImplDX9_CreateFontsTexture())
		return false;

	return true;
}

void ImGui_ImplDX9_NewFrame()
{
	if (!g_FontTexture)
		ImGui_ImplDX9_CreateDeviceObjects();

	ImGuiIO& io = ImGui::GetIO();

	INT64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	io.DeltaTime = (float)(currentTime - g_Time) / g_TicksPerSecond;
	g_Time = currentTime;

	io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
	io.KeySuper = false;

	if (io.WantMoveMouse)
	{
		POINT pos = { (int)io.MousePos.x, (int)io.MousePos.y };
		ClientToScreen(g_hWnd, &pos);
		SetCursorPos(pos.x, pos.y);
	}

	ImGui::NewFrame();
}
