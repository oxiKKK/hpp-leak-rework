#include "main.h"

static void ApplyColorTheme(ImGuiStyle& style)
{
	for (size_t i = 0; i < ImGuiCol_COUNT; i++)
		for (size_t j = 0; j < IM_ARRAYSIZE(cvar.gui[i]); j++)
			cvar.gui[i][j] = style.Colors[i][j];
}

void Themes::DefaultColorTheme()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(230, 230, 230, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(102, 102, 102, 250);
	style.Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(13, 13, 13, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(15, 15, 15, 255);
	style.Colors[ImGuiCol_Border] = ImColor(38, 38, 38, 255);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = ImColor(38, 38, 38, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(46, 46, 46, 255);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(40, 40, 40, 255);
	style.Colors[ImGuiCol_TitleBg] = ImColor(250, 250, 250, 255);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(250, 250, 250, 255);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(250, 250, 250, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(219, 219, 219, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(38, 38, 38, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(100, 100, 100, 204);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(125, 125, 125, 204);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(125, 125, 125, 255);
	style.Colors[ImGuiCol_CheckMark] = ImColor(128, 163, 33, 255);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(128, 163, 33, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(128, 184, 33, 255);
	style.Colors[ImGuiCol_Button] = ImColor(128, 163, 33, 230);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(128, 163, 33, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(128, 184, 33, 255);
	style.Colors[ImGuiCol_Header] = ImColor(128, 163, 33, 102);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(128, 163, 33, 230);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(128, 163, 33, 255);
	style.Colors[ImGuiCol_Separator] = ImColor(59, 59, 59, 255);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(59, 59, 59, 255);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(59, 59, 59, 255);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 143);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 171);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 242);
	style.Colors[ImGuiCol_CloseButton] = ImColor(151, 151, 151, 128);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(100, 100, 100, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 110, 89, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(230, 180, 0, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(255, 115, 0, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(128, 184, 33, 128);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);
	
	ApplyColorTheme(style);
}

void Themes::SimplifiedWhiteRed()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(0, 0, 0, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(102, 102, 102, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(251, 251, 251, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(237, 237, 237, 255);
	style.Colors[ImGuiCol_Border] = ImColor(249, 64, 64, 0);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(229, 55, 55, 4);
	style.Colors[ImGuiCol_FrameBg] = ImColor(226, 226, 226, 145);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(243, 129, 125, 255);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(243, 129, 125, 255);
	style.Colors[ImGuiCol_TitleBg] = ImColor(249, 64, 64, 189);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(253, 141, 136, 255);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(6, 5, 5, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(249, 249, 249, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(255, 66, 66, 206);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(203, 59, 59, 204);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(235, 49, 49, 255);
	style.Colors[ImGuiCol_CheckMark] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(239, 127, 123, 255);
	style.Colors[ImGuiCol_Button] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(243, 129, 125, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(253, 141, 136, 255);
	style.Colors[ImGuiCol_Header] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(249, 74, 67, 204);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(237, 134, 130, 255);
	style.Colors[ImGuiCol_Separator] = ImColor(242, 228, 228, 235);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 143);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 171);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 242);
	style.Colors[ImGuiCol_CloseButton] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(243, 129, 125, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(253, 141, 136, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(254, 254, 254, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(249, 64, 64, 100);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);
	
	ApplyColorTheme(style);
}

void Themes::SimplifiedNightRed()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 205);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(98, 98, 98, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(21, 32, 44, 255);
	style.Colors[ImGuiCol_Border] = ImColor(21, 29, 38, 0);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = ImColor(27, 33, 41, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(168, 76, 72, 255);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(189, 40, 33, 255);
	style.Colors[ImGuiCol_TitleBg] = ImColor(230, 56, 49, 255);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(240, 95, 89, 255);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(6, 5, 5, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(85, 65, 65, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(27, 33, 41, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(178, 34, 34, 204);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(203, 59, 59, 204);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(235, 49, 49, 255);
	style.Colors[ImGuiCol_CheckMark] = ImColor(230, 56, 49, 255);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_Button] = ImColor(230, 56, 49, 255);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(232, 75, 69, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(240, 95, 89, 255);
	style.Colors[ImGuiCol_Header] = ImColor(230, 56, 49, 255);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(232, 75, 69, 255);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(240, 95, 89, 255);
	style.Colors[ImGuiCol_Separator] = ImColor(26, 34, 42, 255);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 143);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 171);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 242);
	style.Colors[ImGuiCol_CloseButton] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(243, 129, 125, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(253, 141, 136, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(100, 100, 100, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 110, 89, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(203, 57, 51, 100);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);

	ApplyColorTheme(style);
}

void Themes::SimplifiedWhiteOrange()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(0, 0, 0, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(102, 102, 102, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(250, 250, 250, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(232, 232, 231, 255);
	style.Colors[ImGuiCol_Border] = ImColor(38, 38, 38, 0);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = ImColor(18, 18, 18, 16);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(255, 127, 0, 175);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(255, 127, 0, 175);
	style.Colors[ImGuiCol_TitleBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(252, 252, 252, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(255, 127, 0, 232);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(255, 127, 0, 174);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(253, 127, 2, 133);
	style.Colors[ImGuiCol_CheckMark] = ImColor(255, 127, 0, 253);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 127, 0, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 127, 0, 175);
	style.Colors[ImGuiCol_Button] = ImColor(255, 127, 0, 204);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(255, 127, 0, 223);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(255, 127, 0, 168);
	style.Colors[ImGuiCol_Header] = ImColor(255, 127, 0, 175);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(255, 127, 0, 223);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(255, 127, 0, 255);
	style.Colors[ImGuiCol_Separator] = ImColor(255, 127, 0, 78);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(59, 59, 59, 0);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(59, 59, 59, 0);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 0);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 0);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 0);
	style.Colors[ImGuiCol_CloseButton] = ImColor(151, 151, 151, 0);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(254, 254, 254, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(255, 127, 0, 100);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);

	ApplyColorTheme(style);
}

void Themes::SimplifiedNightOrange()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(98, 98, 98, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(21, 32, 44, 255);
	style.Colors[ImGuiCol_Border] = ImColor(21, 29, 38, 0);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = ImColor(27, 33, 41, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(255, 127, 0, 148);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(255, 127, 0, 216);
	style.Colors[ImGuiCol_TitleBg] = ImColor(255, 127, 0, 183);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(255, 127, 0, 247);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(6, 5, 5, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(85, 65, 65, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(27, 33, 41, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(255, 127, 0, 247);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(255, 127, 0, 191);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(255, 127, 0, 149);
	style.Colors[ImGuiCol_CheckMark] = ImColor(255, 127, 0, 255);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 127, 0, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 127, 0, 205);
	style.Colors[ImGuiCol_Button] = ImColor(255, 127, 0, 247);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(255, 127, 0, 210);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(255, 127, 0, 161);
	style.Colors[ImGuiCol_Header] = ImColor(255, 127, 0, 191);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(255, 127, 0, 173);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(255, 127, 0, 247);
	style.Colors[ImGuiCol_Separator] = ImColor(26, 34, 42, 255);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 143);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 171);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 242);
	style.Colors[ImGuiCol_CloseButton] = ImColor(249, 64, 64, 220);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(243, 129, 125, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(253, 141, 136, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(100, 100, 100, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 110, 89, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(255, 127, 0, 100);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);

	ApplyColorTheme(style);
}

void Themes::SimplifiedWhiteCyan()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(0, 0, 0, 205);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(98, 98, 98, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(203, 213, 223, 255);
	style.Colors[ImGuiCol_Border] = ImColor(21, 29, 38, 0);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = ImColor(239, 240, 241, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(12, 232, 235, 129);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(12, 232, 235, 141);
	style.Colors[ImGuiCol_TitleBg] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(6, 5, 5, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(85, 65, 65, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(12, 232, 235, 238);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(12, 232, 235, 185);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(12, 232, 235, 144);
	style.Colors[ImGuiCol_CheckMark] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(12, 232, 235, 114);
	style.Colors[ImGuiCol_Button] = ImColor(12, 232, 235, 219);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(12, 232, 235, 119);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(12, 232, 235, 84);
	style.Colors[ImGuiCol_Header] = ImColor(12, 232, 235, 149);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(12, 232, 235, 94);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_Separator] = ImColor(12, 232, 235, 64);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(247, 247, 247, 255);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 0);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 0);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 0);
	style.Colors[ImGuiCol_CloseButton] = ImColor(151, 151, 151, 0);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(254, 254, 254, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(12, 232, 235, 100);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);

	ApplyColorTheme(style);
}

void Themes::SimplifiedNightCyan()
{
	auto& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 205);
	style.Colors[ImGuiCol_TextDisabled] = ImColor(98, 98, 98, 255);
	style.Colors[ImGuiCol_WindowBg] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_ChildWindowBg] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_PopupBg] = ImColor(21, 32, 44, 255);
	style.Colors[ImGuiCol_Border] = ImColor(21, 29, 38, 0);
	style.Colors[ImGuiCol_BorderShadow] = ImColor(0, 0, 0, 0);
	style.Colors[ImGuiCol_FrameBg] = ImColor(27, 33, 41, 255);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(12, 232, 235, 129);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(12, 232, 235, 141);
	style.Colors[ImGuiCol_TitleBg] = ImColor(255, 127, 0, 183);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(255, 127, 0, 247);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(6, 5, 5, 255);
	style.Colors[ImGuiCol_MenuBarBg] = ImColor(85, 65, 65, 255);
	style.Colors[ImGuiCol_ScrollbarBg] = ImColor(27, 33, 41, 255);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(12, 232, 235, 238);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(12, 232, 235, 185);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(12, 232, 235, 144);
	style.Colors[ImGuiCol_CheckMark] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(12, 232, 235, 114);
	style.Colors[ImGuiCol_Button] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(12, 232, 235, 119);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(12, 232, 235, 102);
	style.Colors[ImGuiCol_Header] = ImColor(12, 232, 235, 149);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(12, 232, 235, 94);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(12, 232, 235, 204);
	style.Colors[ImGuiCol_Separator] = ImColor(26, 34, 42, 255);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(21, 29, 38, 255);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(204, 204, 204, 143);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(66, 151, 250, 171);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(66, 151, 250, 242);
	style.Colors[ImGuiCol_CloseButton] = ImColor(151, 151, 151, 128);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImColor(250, 100, 92, 255);
	style.Colors[ImGuiCol_CloseButtonActive] = ImColor(253, 141, 136, 255);
	style.Colors[ImGuiCol_PlotLines] = ImColor(100, 100, 100, 255);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImColor(255, 110, 89, 255);
	style.Colors[ImGuiCol_PlotHistogram] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImColor(199, 41, 35, 255);
	style.Colors[ImGuiCol_TextSelectedBg] = ImColor(12, 232, 235, 100);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 89);
	style.Colors[ImGuiCol_DragDropTarget] = ImColor(0, 0, 0, 0);

	ApplyColorTheme(style);
}