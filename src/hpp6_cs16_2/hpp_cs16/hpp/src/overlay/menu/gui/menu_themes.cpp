#include "framework.h"

void CMenuThemes::SetDefaultDarkTheme()
{
	GImGui->Style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
	GImGui->Style.Colors[ImGuiCol_WindowBg] = ImColor(26, 24, 28, 255);
	GImGui->Style.Colors[ImGuiCol_PopupBg] = ImColor(41, 38, 43, 255);
	GImGui->Style.Colors[ImGuiCol_ChildBg] = ImColor(32, 30, 35, 255);
	GImGui->Style.Colors[ImGuiCol_FrameBg] = ImColor(61, 59, 63, 255);
	GImGui->Style.Colors[ImGuiCol_FrameBgHovered] = ImColor(96, 93, 96, 255);
	GImGui->Style.Colors[ImGuiCol_FrameBgActive] = ImColor(146, 36, 248, 255);
	GImGui->Style.Colors[ImGuiCol_CheckMark] = ImColor(146, 36, 248, 255);
	GImGui->Style.Colors[ImGuiCol_TitleBg] = ImColor(146, 36, 248, 150);
	GImGui->Style.Colors[ImGuiCol_TitleBgActive] = ImColor(146, 36, 248, 150);
	GImGui->Style.Colors[ImGuiCol_Tab] = ImColor(146, 36, 248, 150);
	GImGui->Style.Colors[ImGuiCol_TabHovered] = ImColor(146, 36, 248, 150);
	GImGui->Style.Colors[ImGuiCol_TabActive] = ImColor(146, 36, 248, 255);
	GImGui->Style.Colors[ImGuiCol_Button] = ImColor(61, 59, 63, 255);
	GImGui->Style.Colors[ImGuiCol_ButtonHovered] = ImColor(96, 93, 96, 255);
	GImGui->Style.Colors[ImGuiCol_ButtonActive] = ImColor(103, 100, 103, 255);
	GImGui->Style.Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 0);
	GImGui->Style.Colors[ImGuiCol_HeaderHovered] = ImColor(61, 59, 63, 150);
	GImGui->Style.Colors[ImGuiCol_HeaderActive] = ImColor(61, 59, 63, 150);
	GImGui->Style.Colors[ImGuiCol_Border] = ImColor(41, 38, 43, 255);
	GImGui->Style.Colors[ImGuiCol_BorderShadow] = ImColor(18, 18, 35, 255);//ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	GImGui->Style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.37f, 0.37f, 0.37f, 0.00f);
	GImGui->Style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(80, 80, 80, 204);
	GImGui->Style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(100, 100, 100, 204);
	GImGui->Style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(100, 100, 100, 255);
	GImGui->Style.Colors[ImGuiCol_ModalWindowDarkening] = ImColor(0, 0, 0, 180);
}