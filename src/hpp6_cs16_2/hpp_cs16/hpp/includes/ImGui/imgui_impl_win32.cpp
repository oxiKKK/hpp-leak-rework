#include "framework.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>

static HWND                 g_hWnd = 0;
static INT64                g_Time = 0;
static INT64                g_TicksPerSecond = 0;
static ImGuiMouseCursor     g_LastMouseCursor = ImGuiMouseCursor_COUNT;

// Functions
bool ImGui_ImplWin32_Init(HWND hwnd)
{
    if (!QueryPerformanceFrequency((LARGE_INTEGER*)&g_TicksPerSecond))
        return false;

    if (!QueryPerformanceCounter((LARGE_INTEGER*)&g_Time))
        return false;

    g_hWnd = hwnd;

    GImGui->IO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	GImGui->IO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	GImGui->IO.BackendPlatformName = "imgui_impl_win32";
	GImGui->IO.ImeWindowHandle = hwnd;

	GImGui->IO.KeyMap[ImGuiKey_Tab] = VK_TAB;
	GImGui->IO.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
	GImGui->IO.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
	GImGui->IO.KeyMap[ImGuiKey_UpArrow] = VK_UP;
	GImGui->IO.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
	GImGui->IO.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
	GImGui->IO.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
	GImGui->IO.KeyMap[ImGuiKey_Home] = VK_HOME;
	GImGui->IO.KeyMap[ImGuiKey_End] = VK_END;
	GImGui->IO.KeyMap[ImGuiKey_Insert] = VK_INSERT;
	GImGui->IO.KeyMap[ImGuiKey_Delete] = VK_DELETE;
	GImGui->IO.KeyMap[ImGuiKey_Backspace] = VK_BACK;
	GImGui->IO.KeyMap[ImGuiKey_Space] = VK_SPACE;
	GImGui->IO.KeyMap[ImGuiKey_Enter] = VK_RETURN;
	GImGui->IO.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
	GImGui->IO.KeyMap[ImGuiKey_KeyPadEnter] = VK_RETURN;
	GImGui->IO.KeyMap[ImGuiKey_A] = 'A';
	GImGui->IO.KeyMap[ImGuiKey_C] = 'C';
	GImGui->IO.KeyMap[ImGuiKey_V] = 'V';
	GImGui->IO.KeyMap[ImGuiKey_X] = 'X';
	GImGui->IO.KeyMap[ImGuiKey_Y] = 'Y';
	GImGui->IO.KeyMap[ImGuiKey_Z] = 'Z';

	return true;
}

void ImGui_ImplWin32_Shutdown()
{
    g_hWnd = (HWND)0;
}

static void ImGui_ImplWin32_UpdateMousePos()
{
    if (GImGui->IO.WantSetMousePos)
    {
		POINT pos = { (int)(GImGui->IO.MousePos.x), (int)(GImGui->IO.MousePos.y) };
        ClientToScreen(g_hWnd, &pos);
        SetCursorPos(pos.x, pos.y);
    }

	GImGui->IO.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

    POINT pos;

    if (HWND active_window = GetForegroundWindow())
	{
		if (active_window == g_hWnd || IsChild(active_window, g_hWnd))
		{
			if (GetCursorPos(&pos) && ScreenToClient(g_hWnd, &pos))
				GImGui->IO.MousePos = ImVec2((float)pos.x, (float)pos.y);
		}
	}
}

void ImGui_ImplWin32_NewFrame()
{
    IM_ASSERT(GImGui->IO.Fonts->IsBuilt());

	SCREENINFO screeninfo;
	screeninfo.iSize = sizeof(SCREENINFO);
	g_Engine.pfnGetScreenInfo(&screeninfo);

	GImGui->IO.DisplaySize = ImVec2((float)(screeninfo.iWidth), (float)(screeninfo.iHeight));

	if (!GImGui->IO.DisplaySize.x || !GImGui->IO.DisplaySize.y) 
	{
		RECT rect;
		GetClientRect(g_hWnd, &rect);
		GImGui->IO.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
	}

    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
	GImGui->IO.DeltaTime = (float)(current_time - g_Time) / g_TicksPerSecond;
    g_Time = current_time;

	GImGui->IO.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000);
	GImGui->IO.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000);
	GImGui->IO.KeyAlt = (GetKeyState(VK_MENU) & 0x8000);
	GImGui->IO.KeySuper = false;

    ImGui_ImplWin32_UpdateMousePos();
}

// Allow compilation with old Windows SDK. MinGW doesn't have default _WIN32_WINNT/WINVER versions.
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif

// Process Win32 mouse/keyboard inputs.
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
// PS: In this Win32 handler, we use the capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinates when dragging mouse outside of our window bounds.
// PS: We treat DBLCLK messages as regular mouse down messages, so this code will work on windows classes that have the CS_DBLCLKS flag set. Our own example app code doesn't set this flag.
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui::GetCurrentContext() == NULL)
        return FALSE;

    ImGuiIO& io = ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
    {
        int button = 0;
        if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
        if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
        if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
        if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
            ::SetCapture(hwnd);
        io.MouseDown[button] = true;
        return TRUE;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    {
        int button = 0;
        if (msg == WM_LBUTTONUP) { button = 0; }
        if (msg == WM_RBUTTONUP) { button = 1; }
        if (msg == WM_MBUTTONUP) { button = 2; }
        if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
        io.MouseDown[button] = false;
        if (!ImGui::IsAnyMouseDown() && ::GetCapture() == hwnd)
            ::ReleaseCapture();
        return TRUE;
    }
    case WM_MOUSEWHEEL:
        io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return TRUE;
    case WM_MOUSEHWHEEL:
        io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
        return TRUE;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam < 256)
            io.KeysDown[wParam] = 1;
        return TRUE;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < 256)
            io.KeysDown[wParam] = 0;
        return TRUE;
    case WM_CHAR:
		wchar_t wch;
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
		io.AddInputCharacter(wch);
        return TRUE;
	case WM_MOUSEMOVE:
		return TRUE;
    }
    return FALSE;
}

