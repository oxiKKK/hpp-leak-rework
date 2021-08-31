#include <Windows.h>

struct IDirect3DDevice9;

IMGUI_API bool        ImGui_ImplDX9_Init();
IMGUI_API void        ImGui_ImplDX9_Shutdown();
IMGUI_API void        ImGui_ImplDX9_NewFrame();

IMGUI_API bool		  ImGui_ImplDX9_CreateDeviceObjects();
IMGUI_API void        ImGui_ImplDX9_InvalidateDeviceObjects();