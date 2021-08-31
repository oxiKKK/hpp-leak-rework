#include "main.h"

CRenderScene g_RenderScene;
CDrawList g_DrawList;

void CRenderScene::Begin()
{
	const ImGuiIO& io = ImGui::GetIO();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::Begin("##BackBuffer", static_cast<bool*>(0), ImVec2(), 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);
	ImGui::SetWindowPos(ImVec2(), ImGuiSetCond_Always);
	ImGui::SetWindowSize(io.DisplaySize, ImGuiSetCond_Always);
}

void CRenderScene::Render()
{
	if (cvar.visuals)
		g_Visuals.Run();

	g_DrawGUI.Fadeout();
}

void CRenderScene::End()
{
	ImGui::GetCurrentWindow()->DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}

void CDrawList::AddText(float x, float y, const ImColor& color, float fontSize, FontFlag flags, const char* format, ...)
{
	if (!format)
		return;

	ImGuiIO& io = ImGui::GetIO();
	const ImFont* pFont = io.Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();

	if (pDrawList == nullptr)
		return;

	char cBuff[256] = { '\0' };
	va_list vlist = nullptr;
	va_start(vlist, format);
	vsprintf_s(cBuff, format, vlist);
	va_end(vlist);

	pDrawList->PushTextureID(io.Fonts->TexID);

	const float flSize = fontSize == 0 ? pFont->FontSize : fontSize;
	const ImVec2 vecTextSize = pFont->CalcTextSizeA(flSize, FLT_MAX, 0, cBuff);
	const ImColor colorOutline = ImColor(0.f, 0.f, 0.f, color.Value.w);

	if (flags & FontFlags_CenterX)
		x -= vecTextSize.x * 0.5f;

	if (flags & FontFlags_CenterY)
		y -= vecTextSize.y * 0.5f;

	if (flags & FontFlags_Shadow)
		pDrawList->AddText(pFont, flSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);

	if (flags & FontFlags_Outline)
	{
		pDrawList->AddText(pFont, flSize, ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
		pDrawList->AddText(pFont, flSize, ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
		pDrawList->AddText(pFont, flSize, ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
		pDrawList->AddText(pFont, flSize, ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);

		pDrawList->AddText(pFont, flSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
		pDrawList->AddText(pFont, flSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
		pDrawList->AddText(pFont, flSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
		pDrawList->AddText(pFont, flSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(colorOutline), cBuff);
	}

	pDrawList->AddText(pFont, flSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(color), cBuff);
	pDrawList->PopTextureID();
}

void CDrawList::AddLine(const ImVec2& from, const ImVec2& to, const ImColor& color, float thickness)
{
	ImGui::GetCurrentWindow()->DrawList->AddLine(from, to, ImGui::ColorConvertFloat4ToU32(color), thickness);
}

void CDrawList::AddCircle(const ImVec2& position, float radius, const ImColor& color, int segments)
{
	ImGui::GetCurrentWindow()->DrawList->AddCircle(position, radius, ImGui::ColorConvertFloat4ToU32(color), segments);
}

void CDrawList::AddCircleFilled(const ImVec2& position, float radius, const ImColor& color, int segments)
{
	ImGui::GetCurrentWindow()->DrawList->AddCircleFilled(position, radius, ImGui::ColorConvertFloat4ToU32(color), segments);
}

void CDrawList::AddRect(const ImVec2& position, const ImVec2& size, const ImColor& color, float rounding)
{
	ImGui::GetCurrentWindow()->DrawList->AddRect(position, size, ImGui::ColorConvertFloat4ToU32(color), rounding);
}

void CDrawList::AddRectFilled(const ImVec2& position, const ImVec2& size, const ImColor& color, float rounding)
{
	ImGui::GetCurrentWindow()->DrawList->AddRectFilled(position, size, ImGui::ColorConvertFloat4ToU32(color), rounding);
}

void CDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& color, float thickness)
{
	ImGui::GetCurrentWindow()->DrawList->AddQuad(a, b, c, d, ImGui::ColorConvertFloat4ToU32(color), thickness);
}

void CDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& color)
{
	ImGui::GetCurrentWindow()->DrawList->AddQuadFilled(a, b, c, d, ImGui::ColorConvertFloat4ToU32(color));
}

void CDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& color)
{
	ImGui::GetCurrentWindow()->DrawList->AddTriangleFilled(a, b, c, ImGui::ColorConvertFloat4ToU32(color));
}

void CDrawList::DrawFillArea(float x, float y, float w, float h, const ImColor& color)
{
	AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color);
}

void CDrawList::DrawBox(float x, float y, float w, float h, const ImColor& color)
{
	AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color);
}

void CDrawList::DrawBoxOutline(float x, float y, float w, float h, const ImColor& color)
{
	DrawBox(x - 1, y - 1, w + 2, h + 2, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawBox(x + 1, y + 1, w - 2, h - 2, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawBox(x, y, w, h, color);
}

void CDrawList::DrawCornerBox(float x, float y, float w, float h, const ImColor& color)
{
	AddLine(ImVec2(x, y), ImVec2(x + w / 4, y), color);
	AddLine(ImVec2(x, y), ImVec2(x, y + h / 4), color);

	AddLine(ImVec2(x + w - 1, y), ImVec2(x + w - 1 - w / 4, y), color);
	AddLine(ImVec2(x + w - 1, y), ImVec2(x + w - 1, y + h / 4), color);

	AddLine(ImVec2(x, y + h - 1), ImVec2(x + w / 4, y + h - 1), color);
	AddLine(ImVec2(x, y + h - 1), ImVec2(x, y + h - 1 - h / 4), color);

	AddLine(ImVec2(x + w - 1, y + h - 1), ImVec2(x + w - 1, y + h - 1 - h / 4), color);
	AddLine(ImVec2(x + w - 1, y + h - 1), ImVec2(x + w - 1 - w / 4, y + h - 1), color);
}

void CDrawList::DrawCornerBoxOutline(float x, float y, float w, float h, const ImColor& color)
{
	DrawCornerBox(x - 1, y + 1, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawCornerBox(x - 1, y - 1, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawCornerBox(x + 1, y + 1, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawCornerBox(x + 1, y - 1, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));

	DrawCornerBox(x - 1, y, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawCornerBox(x, y - 1, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawCornerBox(x, y + 1, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawCornerBox(x + 1, y, w, h, ImColor(0.f, 0.f, 0.f, color.Value.w));

	DrawCornerBox(x, y, w, h, color);
}

void CDrawList::DrawRoundBox(float x, float y, float w, float h, const ImColor& color)
{
	AddRect(ImVec2(x, y), ImVec2(x + w, y + h), color, 3);
}

void CDrawList::DrawRoundBoxOutline(float x, float y, float w, float h, const ImColor& color)
{
	DrawRoundBox(x - 1, y - 1, w + 2, h + 2, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawRoundBox(x + 1, y + 1, w - 2, h - 2, ImColor(0.f, 0.f, 0.f, color.Value.w));
	DrawRoundBox(x, y, w, h, color);
}

void CDrawList::DrawHitbox(float* pflOrigin, float* pflVecScreen, const ImColor& color)
{
	float flScreenBone1[2] = {};
	float flScreenBone2[2] = {};

	if (g_Utils.CalcScreen(pflOrigin, flScreenBone1) && g_Utils.CalcScreen(pflVecScreen, flScreenBone2))
		AddLine(ImVec2(flScreenBone1[0], flScreenBone1[1]), ImVec2(flScreenBone2[0], flScreenBone2[1]), color);
}