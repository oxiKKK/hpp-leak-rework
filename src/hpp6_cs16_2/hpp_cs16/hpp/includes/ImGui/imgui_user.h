#pragma once

namespace ImGui
{
	IMGUI_API bool ImageButtonTab(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4 & bg_col = ImVec4(0, 0, 0, 0), const ImVec4 & tint_col = ImVec4(1, 1, 1, 1));    // <0 frame_padding uses default frame padding settings. 0 for no padding
	IMGUI_API bool ImageButtonBySize(ImTextureID user_texture_id, const ImVec2& size, const ImVec2 image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4 & bg_col = ImVec4(0, 0, 0, 0));    // <0 frame_padding uses default frame padding settings. 0 for no padding	
	IMGUI_API bool BeginCustom(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
	IMGUI_API bool BeginChildCustom(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
	IMGUI_API void EndChildCustom();
	IMGUI_API void Spacing(int count);
	IMGUI_API void Separator(float width);
}