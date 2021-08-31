
IMGUI_API bool        ImGui_ImplSdlGL2_Init (HWND hWnd);
IMGUI_API void        ImGui_ImplSdlGL2_Shutdown ();
IMGUI_API void        ImGui_ImplSdlGL2_NewFrame ();

IMGUI_API void        ImGui_ImplSdlGL2_InvalidateDeviceObjects ();
IMGUI_API bool        ImGui_ImplSdlGL2_CreateDeviceObjects ();

IMGUI_API LRESULT	  ImGui_ImplWin32_WndProcHandler (HWND /*hwnd*/, UINT msg, WPARAM wParam, LPARAM lParam);
