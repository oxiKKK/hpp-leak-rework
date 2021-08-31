#include "main.h"

CDrawing g_Drawing;

void CDrawList::AddText(float x, float y, const ImColor& col, const float size, FontFlag flags, const char* format, ...)
{
	if (strlen(format) == 0)
		return;

	auto& io = ImGui::GetIO();
	const auto* const pFont = io.Fonts->Fonts[0];
	auto* const pDrawList = ImGui::GetWindowDrawList();

	char szBuffer[256] = { '\0' };
	va_list vlist = nullptr;
	va_start(vlist, format);
	vsprintf_s(szBuffer, format, vlist);
	va_end(vlist);

	pDrawList->PushTextureID(io.Fonts->TexID);

	const auto font_size = size == 0.0F ? pFont->FontSize : size;
	const auto text_size = pFont->CalcTextSizeA(font_size, FLT_MAX, 0, szBuffer);
	const auto color_outline = ImColor(0.0F, 0.0F, 0.0F, col.Value.w);

	if (flags & FontFlags_CenterX)
		x -= text_size.x * 0.5F;

	if (flags & FontFlags_CenterY)
		y -= text_size.y * 0.5F;

	if (flags & FontFlags_Shadow)
		pDrawList->AddText(pFont, font_size, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);

	if (flags & FontFlags_Outline)
	{
		pDrawList->AddText(pFont, font_size, ImVec2(x, y - 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
		pDrawList->AddText(pFont, font_size, ImVec2(x, y + 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
		pDrawList->AddText(pFont, font_size, ImVec2(x + 1, y), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
		pDrawList->AddText(pFont, font_size, ImVec2(x - 1, y), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);

		pDrawList->AddText(pFont, font_size, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
		pDrawList->AddText(pFont, font_size, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
		pDrawList->AddText(pFont, font_size, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
		pDrawList->AddText(pFont, font_size, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(color_outline), szBuffer);
	}

	pDrawList->AddText(pFont, font_size, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(col), szBuffer);
	pDrawList->PopTextureID();
}

void CDrawList::AddLine(const ImVec2& from, const ImVec2& to, const ImColor& col, const float thickness)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddLine(from, to, ImGui::ColorConvertFloat4ToU32(col), thickness);
}

void CDrawList::AddCircle(const ImVec2& position, const float radius, const ImColor& col, const int segments)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddCircle(position, radius, ImGui::ColorConvertFloat4ToU32(col), segments);
}

void CDrawList::AddCircleFilled(const ImVec2& position, const float radius, const ImColor& col, const int segments)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddCircleFilled(position, radius, ImGui::ColorConvertFloat4ToU32(col), segments);
}

void CDrawList::AddRect(const ImVec2& position, const ImVec2& size, const ImColor& col, const float rounding)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddRect(position, size, ImGui::ColorConvertFloat4ToU32(col), rounding);
}

void CDrawList::AddRectFilled(const ImVec2& position, const ImVec2& size, const ImColor& col, const float rounding)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddRectFilled(position, size, ImGui::ColorConvertFloat4ToU32(col), rounding);
}

void CDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col, const float thickness)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddQuad(a, b, c, d, ImGui::ColorConvertFloat4ToU32(col), thickness);
}

void CDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImColor& col)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddQuadFilled(a, b, c, d, ImGui::ColorConvertFloat4ToU32(col));
}

void CDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col, const float thickness)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddTriangle(a, b, c, ImGui::ColorConvertFloat4ToU32(col));
}

void CDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImColor& col)
{
	const auto* const window = ImGui::GetCurrentWindow();
	window->DrawList->AddTriangleFilled(a, b, c, ImGui::ColorConvertFloat4ToU32(col));
}

void CDrawing::RectFilled(const float x, const float y, const float w, const float h, const ImColor& col)
{
	AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), col);
}

void CDrawing::Rect(const float x, const float y, const float w, const float h, const ImColor& col)
{
	AddRect(ImVec2(x, y), ImVec2(x + w, y + h), col);
}

void CDrawing::RectOutlined(const float x, const float y, const float w, const float h, const ImColor& col)
{
	AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	AddRect(ImVec2(x + 1, y + 1), ImVec2(x + w - 1, y + h - 1), ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	AddRect(ImVec2(x, y), ImVec2(x + w, y + h), col);
}

void CDrawing::RectCorners(const float x, const float y, const float w, const float h, const ImColor& col)
{
	AddLine(ImVec2(x, y), ImVec2(x + w * 0.25F, y), col);
	AddLine(ImVec2(x, y), ImVec2(x, y + h * 0.25F), col);

	AddLine(ImVec2(x + w - 1, y), ImVec2(x + w - 1 - w * 0.25F, y), col);
	AddLine(ImVec2(x + w - 1, y), ImVec2(x + w - 1, y + h * 0.25F), col);

	AddLine(ImVec2(x, y + h - 1), ImVec2(x + w * 0.25F, y + h - 1), col);
	AddLine(ImVec2(x, y + h - 1), ImVec2(x, y + h - 1 - h * 0.25F), col);

	AddLine(ImVec2(x + w - 1, y + h - 1), ImVec2(x + w - 1, y + h - 1 - h * 0.25F), col);
	AddLine(ImVec2(x + w - 1, y + h - 1), ImVec2(x + w - 1 - w * 0.25F, y + h - 1), col);
}

void CDrawing::RectCornersOutlined(const float x, const float y, const float w, const float h, const ImColor& col)
{
	RectCorners(x - 1, y + 1, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	RectCorners(x - 1, y - 1, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	RectCorners(x + 1, y + 1, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	RectCorners(x + 1, y - 1, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));

	RectCorners(x - 1, y, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	RectCorners(x, y - 1, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	RectCorners(x, y + 1, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));
	RectCorners(x + 1, y, w, h, ImColor(0.0F, 0.0F, 0.0F, col.Value.w));

	RectCorners(x, y, w, h, col);
}

void CDrawing::RectRound(const float x, const float y, const float w, const float h, const ImColor& col)
{
	AddRect(ImVec2(x, y), ImVec2(x + w, y + h), col, 3.0F);
}

void CDrawing::RectRoundOutlined(const float x, const float y, const float w, const float h, const ImColor& col)
{
	AddRect(ImVec2(x - 1, y - 1), ImVec2(x + w + 1, y + h + 1), ImColor(0.0F, 0.0F, 0.0F, col.Value.w), 3.0F);
	AddRect(ImVec2(x + 1, y + 1), ImVec2(x + w - 1, y + h - 1), ImColor(0.0F, 0.0F, 0.0F, col.Value.w), 3.0F);
	AddRect(ImVec2(x, y), ImVec2(x + w, y + h), col, 3.0F);
}

void CDrawing::LineByOrigin(const Vector& from, const Vector& to, const ImColor& col)
{
	float flScreenBone1[2], flScreenBone2[2];

	if (g_Utils.CalcScreen(from, &flScreenBone1[0]) && g_Utils.CalcScreen(to, &flScreenBone2[0]))
		AddLine(ImVec2(flScreenBone1[0], flScreenBone1[1]), ImVec2(flScreenBone2[0], flScreenBone2[1]), col);
}