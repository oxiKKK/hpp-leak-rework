#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "framework.h"

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <ctype.h>      // toupper
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat-nonliteral"      // warning : format string is not a string literal              // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant              // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wformat-nonliteral"        // warning: format not a string literal, format string not checked
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

// Window resizing from edges (when io.ConfigWindowsResizeFromEdges = true and ImGuiBackendFlags_HasMouseCursors is set in io.BackendFlags by back-end)
static const float WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS = 4.0f;     // Extend outside and inside windows. Affect FindHoveredWindow().
static const float WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER = 0.04f;    // Reduce visual noise by only highlighting the border after a certain time.

struct ImGuiResizeGripDef
{
	ImVec2  CornerPosN;
	ImVec2  InnerDir;
	int     AngleMin12, AngleMax12;
};

static const ImGuiResizeGripDef resize_grip_def[4] =
{
	{ ImVec2(1,1), ImVec2(-1,-1), 0, 3 }, // Lower right
	{ ImVec2(0,1), ImVec2(+1,-1), 3, 6 }, // Lower left
	{ ImVec2(0,0), ImVec2(+1,+1), 6, 9 }, // Upper left
	{ ImVec2(1,0), ImVec2(-1,+1), 9,12 }, // Upper right
};

namespace ImGui
{
	static bool BeginChildCustomEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags);
	static bool UpdateManualResize(ImGuiWindow* window, const ImVec2& size_auto_fit, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4]);
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
// The color used are the button colors.
bool ImGui::ImageButtonTab(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
	PushID((void*)(intptr_t)user_texture_id);
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const ImRect image_bb(window->DC.CursorPos + size * 0.5f - ImVec2(18, 18), window->DC.CursorPos + size * 0.5f + ImVec2(18, 18));
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, 2.f));
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));
	const auto alpha = GetStyleColorVec4(ImGuiCol_Text).w;
	window->DrawList->AddImage(user_texture_id, image_bb.Min, image_bb.Max, uv0, uv1, ImColor(1.00f, 1.00f, 1.00f, alpha));

	return pressed;
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
// The color used are the button colors.
bool ImGui::ImageButtonBySize(ImTextureID user_texture_id, const ImVec2& size, const ImVec2 image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
	PushID((void*)(intptr_t)user_texture_id);
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (size - image_size) * 0.5f;
	const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + image_size);
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);

	// Render   
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, 2.f);
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));
	const auto alpha = GetStyleColorVec4(ImGuiCol_Text).w;
	window->DrawList->AddImage(user_texture_id, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(ImVec4(1.00f, 1.00f, 1.00f, alpha)));

	return pressed;
}

static void SetWindowConditionAllowFlags(ImGuiWindow* window, ImGuiCond flags, bool enabled)
{
	window->SetWindowPosAllowFlags = enabled ? (window->SetWindowPosAllowFlags | flags) : (window->SetWindowPosAllowFlags & ~flags);
	window->SetWindowSizeAllowFlags = enabled ? (window->SetWindowSizeAllowFlags | flags) : (window->SetWindowSizeAllowFlags & ~flags);
	window->SetWindowCollapsedAllowFlags = enabled ? (window->SetWindowCollapsedAllowFlags | flags) : (window->SetWindowCollapsedAllowFlags & ~flags);
}

static ImGuiWindow* CreateNewWindow(const char* name, ImVec2 size, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;
	//IMGUI_DEBUG_LOG("CreateNewWindow '%s', flags = 0x%08X\n", name, flags);

	// Create window the first time
	ImGuiWindow* window = IM_NEW(ImGuiWindow)(&g, name);
	window->Flags = flags;
	g.WindowsById.SetVoidPtr(window->ID, window);

	// Default/arbitrary window position. Use SetNextWindowPos() with the appropriate condition flag to change the initial position of a window.
	window->Pos = ImVec2(60, 60);

	// User can disable loading and saving of settings. Tooltip and child windows also don't store settings.
	if (!(flags & ImGuiWindowFlags_NoSavedSettings))
		if (ImGuiWindowSettings* settings = ImGui::FindWindowSettings(window->ID))
		{
			// Retrieve settings from .ini file
			window->SettingsIdx = g.SettingsWindows.index_from_ptr(settings);
			SetWindowConditionAllowFlags(window, ImGuiCond_FirstUseEver, false);
			window->Pos = ImFloor(settings->Pos);
			window->Collapsed = settings->Collapsed;
			if (ImLengthSqr(settings->Size) > 0.00001f)
				size = ImFloor(settings->Size);
		}
	window->Size = window->SizeFull = ImFloor(size);
	window->DC.CursorStartPos = window->DC.CursorMaxPos = window->Pos; // So first call to CalcContentSize() doesn't return crazy values

	if ((flags & ImGuiWindowFlags_AlwaysAutoResize) != 0)
	{
		window->AutoFitFramesX = window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = false;
	}
	else
	{
		if (window->Size.x <= 0.0f)
			window->AutoFitFramesX = 2;
		if (window->Size.y <= 0.0f)
			window->AutoFitFramesY = 2;
		window->AutoFitOnlyGrows = (window->AutoFitFramesX > 0) || (window->AutoFitFramesY > 0);
	}

	g.WindowsFocusOrder.push_back(window);
	if (flags & ImGuiWindowFlags_NoBringToFrontOnFocus)
		g.Windows.push_front(window); // Quite slow but rare and only once
	else
		g.Windows.push_back(window);
	return window;
}

// Save and compare stack sizes on Begin()/End() to detect usage errors
static void CheckStacksSize(ImGuiWindow* window, bool write)
{
	// NOT checking: DC.ItemWidth, DC.AllowKeyboardFocus, DC.ButtonRepeat, DC.TextWrapPos (per window) to allow user to conveniently push once and not pop (they are cleared on Begin)
	ImGuiContext& g = *GImGui;
	short* p_backup = &window->DC.StackSizesBackup[0];
	{ int current = window->IDStack.Size;       if (write)*p_backup = (short)current; else IM_ASSERT(*p_backup == current && "PushID/PopID or TreeNode/TreePop Mismatch!");   p_backup++; }    // Too few or too many PopID()/TreePop()
	{ int current = window->DC.GroupStack.Size; if (write)*p_backup = (short)current; else IM_ASSERT(*p_backup == current && "BeginGroup/EndGroup Mismatch!");                p_backup++; }    // Too few or too many EndGroup()
	{ int current = g.BeginPopupStack.Size;     if (write)*p_backup = (short)current; else IM_ASSERT(*p_backup == current && "BeginMenu/EndMenu or BeginPopup/EndPopup Mismatch"); p_backup++; }// Too few or too many EndMenu()/EndPopup()
	// For color, style and font stacks there is an incentive to use Push/Begin/Pop/.../End patterns, so we relax our checks a little to allow them.
	{ int current = g.ColorModifiers.Size;      if (write)*p_backup = (short)current; else IM_ASSERT(*p_backup >= current && "PushStyleColor/PopStyleColor Mismatch!");       p_backup++; }    // Too few or too many PopStyleColor()
	{ int current = g.StyleModifiers.Size;      if (write)*p_backup = (short)current; else IM_ASSERT(*p_backup >= current && "PushStyleVar/PopStyleVar Mismatch!");           p_backup++; }    // Too few or too many PopStyleVar()
	{ int current = g.FontStack.Size;           if (write)*p_backup = (short)current; else IM_ASSERT(*p_backup >= current && "PushFont/PopFont Mismatch!");                   p_backup++; }    // Too few or too many PopFont()
	IM_ASSERT(p_backup == window->DC.StackSizesBackup + IM_ARRAYSIZE(window->DC.StackSizesBackup));
}

static ImVec2 CalcContentSize(ImGuiWindow* window)
{
	if (window->Collapsed)
		if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
			return window->ContentSize;
	if (window->Hidden && window->HiddenFramesCannotSkipItems == 0 && window->HiddenFramesCanSkipItems > 0)
		return window->ContentSize;

	ImVec2 sz;
	sz.x = (float)(int)((window->ContentSizeExplicit.x != 0.0f) ? window->ContentSizeExplicit.x : window->DC.CursorMaxPos.x - window->DC.CursorStartPos.x);
	sz.y = (float)(int)((window->ContentSizeExplicit.y != 0.0f) ? window->ContentSizeExplicit.y : window->DC.CursorMaxPos.y - window->DC.CursorStartPos.y);
	return sz;
}

static void SetCurrentWindow(ImGuiWindow* window)
{
	ImGuiContext& g = *GImGui;
	g.CurrentWindow = window;
	if (window)
		g.FontSize = g.DrawListSharedData.FontSize = window->CalcFontSize();
}

static ImVec2 CalcSizeAfterConstraint(ImGuiWindow* window, ImVec2 new_size)
{
	ImGuiContext& g = *GImGui;
	if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint)
	{
		// Using -1,-1 on either X/Y axis to preserve the current size.
		ImRect cr = g.NextWindowData.SizeConstraintRect;
		new_size.x = (cr.Min.x >= 0 && cr.Max.x >= 0) ? ImClamp(new_size.x, cr.Min.x, cr.Max.x) : window->SizeFull.x;
		new_size.y = (cr.Min.y >= 0 && cr.Max.y >= 0) ? ImClamp(new_size.y, cr.Min.y, cr.Max.y) : window->SizeFull.y;
		if (g.NextWindowData.SizeCallback)
		{
			ImGuiSizeCallbackData data;
			data.UserData = g.NextWindowData.SizeCallbackUserData;
			data.Pos = window->Pos;
			data.CurrentSize = window->SizeFull;
			data.DesiredSize = new_size;
			g.NextWindowData.SizeCallback(&data);
			new_size = data.DesiredSize;
		}
		new_size.x = ImFloor(new_size.x);
		new_size.y = ImFloor(new_size.y);
	}

	// Minimum size
	if (!(window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_AlwaysAutoResize)))
	{
		new_size = ImMax(new_size, g.Style.WindowMinSize);
		new_size.y = ImMax(new_size.y, window->TitleBarHeight() + window->MenuBarHeight() + ImMax(0.0f, g.Style.WindowRounding - 1.0f)); // Reduce artifacts with very small windows
	}
	return new_size;
}

static ImVec2 CalcSizeAutoFit(ImGuiWindow* window, const ImVec2& size_contents)
{
	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;
	ImVec2 size_decorations = ImVec2(0.0f, window->TitleBarHeight() + window->MenuBarHeight());
	ImVec2 size_pad = window->WindowPadding * 2.0f;
	ImVec2 size_desired = size_contents + size_pad + size_decorations;
	if (window->Flags & ImGuiWindowFlags_Tooltip)
	{
		// Tooltip always resize
		return size_desired;
	}
	else
	{
		// Maximum window size is determined by the viewport size or monitor size
		const bool is_popup = (window->Flags & ImGuiWindowFlags_Popup) != 0;
		const bool is_menu = (window->Flags & ImGuiWindowFlags_ChildMenu) != 0;
		ImVec2 size_min = style.WindowMinSize;
		if (is_popup || is_menu) // Popups and menus bypass style.WindowMinSize by default, but we give then a non-zero minimum size to facilitate understanding problematic cases (e.g. empty popups)
			size_min = ImMin(size_min, ImVec2(4.0f, 4.0f));
		ImVec2 size_auto_fit = ImClamp(size_desired, size_min, ImMax(size_min, g.IO.DisplaySize - style.DisplaySafeAreaPadding * 2.0f));

		// When the window cannot fit all contents (either because of constraints, either because screen is too small),
		// we are growing the size on the other axis to compensate for expected scrollbar. FIXME: Might turn bigger than ViewportSize-WindowPadding.
		ImVec2 size_auto_fit_after_constraint = CalcSizeAfterConstraint(window, size_auto_fit);
		bool will_have_scrollbar_x = (size_auto_fit_after_constraint.x - size_pad.x - size_decorations.x < size_contents.x && !(window->Flags & ImGuiWindowFlags_NoScrollbar) && (window->Flags & ImGuiWindowFlags_HorizontalScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		bool will_have_scrollbar_y = (size_auto_fit_after_constraint.y - size_pad.y - size_decorations.y < size_contents.y && !(window->Flags & ImGuiWindowFlags_NoScrollbar)) || (window->Flags & ImGuiWindowFlags_AlwaysVerticalScrollbar);
		if (will_have_scrollbar_x)
			size_auto_fit.y += style.ScrollbarSize;
		if (will_have_scrollbar_y)
			size_auto_fit.x += style.ScrollbarSize;
		return size_auto_fit;
	}
}

static ImRect GetViewportRect()
{
	ImGuiContext& g = *GImGui;
	return ImRect(0.0f, 0.0f, g.IO.DisplaySize.x, g.IO.DisplaySize.y);
}

static inline void ClampWindowRect(ImGuiWindow* window, const ImRect& rect, const ImVec2& padding)
{
	ImGuiContext& g = *GImGui;
	ImVec2 size_for_clamping = (g.IO.ConfigWindowsMoveFromTitleBarOnly && !(window->Flags & ImGuiWindowFlags_NoTitleBar)) ? ImVec2(window->Size.x, window->TitleBarHeight()) : window->Size;
	window->Pos = ImMin(rect.Max - padding, ImMax(window->Pos + size_for_clamping, rect.Min + padding) - size_for_clamping);
}

static void CalcResizePosSizeFromAnyCorner(ImGuiWindow* window, const ImVec2& corner_target, const ImVec2& corner_norm, ImVec2* out_pos, ImVec2* out_size)
{
	ImVec2 pos_min = ImLerp(corner_target, window->Pos, corner_norm);                // Expected window upper-left
	ImVec2 pos_max = ImLerp(window->Pos + window->Size, corner_target, corner_norm); // Expected window lower-right
	ImVec2 size_expected = pos_max - pos_min;
	ImVec2 size_constrained = CalcSizeAfterConstraint(window, size_expected);
	*out_pos = pos_min;
	if (corner_norm.x == 0.0f)
		out_pos->x -= (size_constrained.x - size_expected.x);
	if (corner_norm.y == 0.0f)
		out_pos->y -= (size_constrained.y - size_expected.y);
	*out_size = size_constrained;
}

static ImRect GetResizeBorderRect(ImGuiWindow* window, int border_n, float perp_padding, float thickness)
{
	ImRect rect = window->Rect();
	if (thickness == 0.0f) rect.Max -= ImVec2(1, 1);
	if (border_n == 0) return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness);      // Top
	if (border_n == 1) return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding);   // Right
	if (border_n == 2) return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness);      // Bottom
	if (border_n == 3) return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding);   // Left
	IM_ASSERT(0);
	return ImRect();
}

// Handle resize for: Resize Grips, Borders, Gamepad
// Return true when using auto-fit (double click on resize grip)
static bool ImGui::UpdateManualResize(ImGuiWindow* window, const ImVec2& size_auto_fit, int* border_held, int resize_grip_count, ImU32 resize_grip_col[4])
{
	ImGuiContext& g = *GImGui;
	ImGuiWindowFlags flags = window->Flags;

	if ((flags & ImGuiWindowFlags_NoResize) || (flags & ImGuiWindowFlags_AlwaysAutoResize) || window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		return false;
	if (window->WasActive == false) // Early out to avoid running this code for e.g. an hidden implicit/fallback Debug window.
		return false;

	bool ret_auto_fit = false;
	const int resize_border_count = g.IO.ConfigWindowsResizeFromEdges ? 4 : 0;
	const float grip_draw_size = (float)(int)ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f);
	const float grip_hover_inner_size = (float)(int)(grip_draw_size * 0.500f);
	const float grip_hover_outer_size = g.IO.ConfigWindowsResizeFromEdges ? WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS : 0.0f;

	ImVec2 pos_target(FLT_MAX, FLT_MAX);
	ImVec2 size_target(FLT_MAX, FLT_MAX);

	// Resize grips and borders are on layer 1
	window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
	window->DC.NavLayerCurrentMask = (1 << ImGuiNavLayer_Menu);

	// Manual resize grips
	PushID("#RESIZE");
	for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
	{
		const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
		const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);

		// Using the FlattenChilds button flag we make the resize button accessible even if we are hovering over a child window
		ImRect resize_rect(corner - grip.InnerDir * grip_hover_outer_size, corner + grip.InnerDir * grip_hover_inner_size);
		if (resize_rect.Min.x > resize_rect.Max.x) ImSwap(resize_rect.Min.x, resize_rect.Max.x);
		if (resize_rect.Min.y > resize_rect.Max.y) ImSwap(resize_rect.Min.y, resize_rect.Max.y);
		bool hovered, held;
		ButtonBehavior(resize_rect, window->GetID((void*)(intptr_t)resize_grip_n), &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_NoNavFocus);
		//GetForegroundDrawList(window)->AddRect(resize_rect.Min, resize_rect.Max, IM_COL32(255, 255, 0, 255));
		if (hovered || held)
			g.MouseCursor = (resize_grip_n & 1) ? ImGuiMouseCursor_ResizeNESW : ImGuiMouseCursor_ResizeNWSE;

		if (held && g.IO.MouseDoubleClicked[0] && resize_grip_n == 0)
		{
			// Manual auto-fit when double-clicking
			size_target = CalcSizeAfterConstraint(window, size_auto_fit);
			ret_auto_fit = true;
			ClearActiveID();
		}
		else if (held)
		{
			// Resize from any of the four corners
			// We don't use an incremental MouseDelta but rather compute an absolute target size based on mouse position
			ImVec2 corner_target = g.IO.MousePos - g.ActiveIdClickOffset + ImLerp(grip.InnerDir * grip_hover_outer_size, grip.InnerDir * -grip_hover_inner_size, grip.CornerPosN); // Corner of the window corresponding to our corner grip
			CalcResizePosSizeFromAnyCorner(window, corner_target, grip.CornerPosN, &pos_target, &size_target);
		}
		if (resize_grip_n == 0 || held || hovered)
			resize_grip_col[resize_grip_n] = GetColorU32(held ? ImGuiCol_ResizeGripActive : hovered ? ImGuiCol_ResizeGripHovered : ImGuiCol_ResizeGrip);
	}
	for (int border_n = 0; border_n < resize_border_count; border_n++)
	{
		bool hovered, held;
		ImRect border_rect = GetResizeBorderRect(window, border_n, grip_hover_inner_size, WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS);
		ButtonBehavior(border_rect, window->GetID((void*)(intptr_t)(border_n + 4)), &hovered, &held, ImGuiButtonFlags_FlattenChildren);
		//GetForegroundDrawLists(window)->AddRect(border_rect.Min, border_rect.Max, IM_COL32(255, 255, 0, 255));
		if ((hovered && g.HoveredIdTimer > WINDOWS_RESIZE_FROM_EDGES_FEEDBACK_TIMER) || held)
		{
			g.MouseCursor = (border_n & 1) ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS;
			if (held)
				*border_held = border_n;
		}
		if (held)
		{
			ImVec2 border_target = window->Pos;
			ImVec2 border_posn;
			if (border_n == 0) { border_posn = ImVec2(0, 0); border_target.y = (g.IO.MousePos.y - g.ActiveIdClickOffset.y + WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS); } // Top
			if (border_n == 1) { border_posn = ImVec2(1, 0); border_target.x = (g.IO.MousePos.x - g.ActiveIdClickOffset.x + WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS); } // Right
			if (border_n == 2) { border_posn = ImVec2(0, 1); border_target.y = (g.IO.MousePos.y - g.ActiveIdClickOffset.y + WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS); } // Bottom
			if (border_n == 3) { border_posn = ImVec2(0, 0); border_target.x = (g.IO.MousePos.x - g.ActiveIdClickOffset.x + WINDOWS_RESIZE_FROM_EDGES_HALF_THICKNESS); } // Left
			CalcResizePosSizeFromAnyCorner(window, border_target, border_posn, &pos_target, &size_target);
		}
	}
	PopID();

	// Navigation resize (keyboard/gamepad)
	if (g.NavWindowingTarget && g.NavWindowingTarget->RootWindow == window)
	{
		ImVec2 nav_resize_delta;
		if (g.NavInputSource == ImGuiInputSource_NavKeyboard && g.IO.KeyShift)
			nav_resize_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard, ImGuiInputReadMode_Down);
		if (g.NavInputSource == ImGuiInputSource_NavGamepad)
			nav_resize_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_Down);
		if (nav_resize_delta.x != 0.0f || nav_resize_delta.y != 0.0f)
		{
			const float NAV_RESIZE_SPEED = 600.0f;
			nav_resize_delta *= ImFloor(NAV_RESIZE_SPEED * g.IO.DeltaTime * ImMin(g.IO.DisplayFramebufferScale.x, g.IO.DisplayFramebufferScale.y));
			g.NavWindowingToggleLayer = false;
			g.NavDisableMouseHover = true;
			resize_grip_col[0] = GetColorU32(ImGuiCol_ResizeGripActive);
			// FIXME-NAV: Should store and accumulate into a separate size buffer to handle sizing constraints properly, right now a constraint will make us stuck.
			size_target = CalcSizeAfterConstraint(window, window->SizeFull + nav_resize_delta);
		}
	}

	// Apply back modified position/size to window
	if (size_target.x != FLT_MAX)
	{
		window->SizeFull = size_target;
		MarkIniSettingsDirty(window);
	}
	if (pos_target.x != FLT_MAX)
	{
		window->Pos = ImFloor(pos_target);
		MarkIniSettingsDirty(window);
	}

	// Resize nav layer
	window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
	window->DC.NavLayerCurrentMask = (1 << ImGuiNavLayer_Main);

	window->Size = window->SizeFull;
	return ret_auto_fit;
}

static ImVec2 CalcNextScrollFromScrollTargetAndClamp(ImGuiWindow* window, bool snap_on_edges)
{
	ImGuiContext& g = *GImGui;
	ImVec2 scroll = window->Scroll;
	if (window->ScrollTarget.x < FLT_MAX)
	{
		float cr_x = window->ScrollTargetCenterRatio.x;
		float target_x = window->ScrollTarget.x;
		if (snap_on_edges && cr_x <= 0.0f && target_x <= window->WindowPadding.x)
			target_x = 0.0f;
		else if (snap_on_edges && cr_x >= 1.0f && target_x >= window->ContentSize.x + window->WindowPadding.x + GImGui->Style.ItemSpacing.x)
			target_x = window->ContentSize.x + window->WindowPadding.x * 2.0f;
		scroll.x = target_x - cr_x * window->InnerRect.GetWidth();
	}
	if (window->ScrollTarget.y < FLT_MAX)
	{
		// 'snap_on_edges' allows for a discontinuity at the edge of scrolling limits to take account of WindowPadding so that scrolling to make the last item visible scroll far enough to see the padding.
		float cr_y = window->ScrollTargetCenterRatio.y;
		float target_y = window->ScrollTarget.y;
		if (snap_on_edges && cr_y <= 0.0f && target_y <= window->WindowPadding.y)
			target_y = 0.0f;
		if (snap_on_edges && cr_y >= 1.0f && target_y >= window->ContentSize.y + window->WindowPadding.y + g.Style.ItemSpacing.y)
			target_y = window->ContentSize.y + window->WindowPadding.y * 2.0f;
		scroll.y = target_y - cr_y * window->InnerRect.GetHeight();
	}
	scroll = ImMax(scroll, ImVec2(0.0f, 0.0f));
	if (!window->Collapsed && !window->SkipItems)
	{
		scroll.x = ImMin(scroll.x, window->ScrollMax.x);
		scroll.y = ImMin(scroll.y, window->ScrollMax.y);
	}
	return scroll;
}

static ImGuiCol GetWindowBgColorIdxFromFlags(ImGuiWindowFlags flags)
{
	if (flags & (ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup))
		return ImGuiCol_PopupBg;
	if (flags & ImGuiWindowFlags_ChildWindow)
		return ImGuiCol_ChildBg;
	return ImGuiCol_WindowBg;
}

static std::string g_name;

// Push a new Dear ImGui window to add widgets to.
// - A default window called "Debug" is automatically stacked at the beginning of every frame so you can use widgets without explicitly calling a Begin/End pair.
// - Begin/End can be called multiple times during the frame with the same window name to append content.
// - The window name is used as a unique identifier to preserve window information across frames (and save rudimentary information to the .ini file).
//   You can use the "##" or "###" markers to use the same label with different id, or same id with different label. See documentation at the top of this file.
// - Return false when window is collapsed, so you can early out in your code. You always need to call ImGui::End() even if false is returned.
// - Passing 'bool* p_open' displays a Close button on the upper-right corner of the window, the pointed value will be set to false when the button is pressed.
bool ImGui::BeginCustom(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	IM_ASSERT(name != NULL && name[0] != '\0');     // Window name required
	IM_ASSERT(g.FrameScopeActive);                  // Forgot to call ImGui::NewFrame()
	IM_ASSERT(g.FrameCountEnded != g.FrameCount);   // Called ImGui::Render() or ImGui::EndFrame() and haven't called ImGui::NewFrame() again yet

	// Find or create
	ImGuiWindow* window = FindWindowByName(name);
	const bool window_just_created = (window == NULL);
	if (window_just_created)
	{
		ImVec2 size_on_first_use = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize) ? g.NextWindowData.SizeVal : ImVec2(0.0f, 0.0f); // Any condition flag will do since we are creating a new window here.
		window = CreateNewWindow(name, size_on_first_use, flags);
	}

	// Automatically disable manual moving/resizing when NoInputs is set
	if ((flags & ImGuiWindowFlags_NoInputs) == ImGuiWindowFlags_NoInputs)
		flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

	if (flags & ImGuiWindowFlags_NavFlattened)
		IM_ASSERT(flags & ImGuiWindowFlags_ChildWindow);

	const int current_frame = g.FrameCount;
	const bool first_begin_of_the_frame = (window->LastFrameActive != current_frame);

	// Update the Appearing flag
	bool window_just_activated_by_user = (window->LastFrameActive < current_frame - 1);   // Not using !WasActive because the implicit "Debug" window would always toggle off->on
	const bool window_just_appearing_after_hidden_for_resize = (window->HiddenFramesCannotSkipItems > 0);
	if (flags & ImGuiWindowFlags_Popup)
	{
		ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
		window_just_activated_by_user |= (window->PopupId != popup_ref.PopupId); // We recycle popups so treat window as activated if popup id changed
		window_just_activated_by_user |= (window != popup_ref.Window);
	}
	window->Appearing = (window_just_activated_by_user || window_just_appearing_after_hidden_for_resize);
	if (window->Appearing)
		SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, true);

	// Update Flags, LastFrameActive, BeginOrderXXX fields
	if (first_begin_of_the_frame)
	{
		window->Flags = (ImGuiWindowFlags)flags;
		window->LastFrameActive = current_frame;
		window->BeginOrderWithinParent = 0;
		window->BeginOrderWithinContext = (short)(g.WindowsActiveCount++);
	}
	else
	{
		flags = window->Flags;
	}

	// Parent window is latched only on the first call to Begin() of the frame, so further append-calls can be done from a different window stack
	ImGuiWindow* parent_window_in_stack = g.CurrentWindowStack.empty() ? NULL : g.CurrentWindowStack.back();
	ImGuiWindow* parent_window = first_begin_of_the_frame ? ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup)) ? parent_window_in_stack : NULL) : window->ParentWindow;
	IM_ASSERT(parent_window != NULL || !(flags & ImGuiWindowFlags_ChildWindow));

	// Add to stack
	// We intentionally set g.CurrentWindow to NULL to prevent usage until when the viewport is set, then will call SetCurrentWindow()
	g.CurrentWindowStack.push_back(window);
	g.CurrentWindow = NULL;
	CheckStacksSize(window, true);
	if (flags & ImGuiWindowFlags_Popup)
	{
		ImGuiPopupData& popup_ref = g.OpenPopupStack[g.BeginPopupStack.Size];
		popup_ref.Window = window;
		g.BeginPopupStack.push_back(popup_ref);
		window->PopupId = popup_ref.PopupId;
	}

	if (window_just_appearing_after_hidden_for_resize && !(flags & ImGuiWindowFlags_ChildWindow))
		window->NavLastIds[0] = 0;

	// Process SetNextWindow***() calls
	bool window_pos_set_by_api = false;
	bool window_size_x_set_by_api = false, window_size_y_set_by_api = false;
	if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasPos)
	{
		window_pos_set_by_api = (window->SetWindowPosAllowFlags & g.NextWindowData.PosCond) != 0;
		if (window_pos_set_by_api && ImLengthSqr(g.NextWindowData.PosPivotVal) > 0.00001f)
		{
			// May be processed on the next frame if this is our first frame and we are measuring size
			// FIXME: Look into removing the branch so everything can go through this same code path for consistency.
			window->SetWindowPosVal = g.NextWindowData.PosVal;
			window->SetWindowPosPivot = g.NextWindowData.PosPivotVal;
			window->SetWindowPosAllowFlags &= ~(ImGuiCond_Once | ImGuiCond_FirstUseEver | ImGuiCond_Appearing);
		}
		else
		{
			SetWindowPos(window, g.NextWindowData.PosVal, g.NextWindowData.PosCond);
		}
	}
	if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSize)
	{
		window_size_x_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.x > 0.0f);
		window_size_y_set_by_api = (window->SetWindowSizeAllowFlags & g.NextWindowData.SizeCond) != 0 && (g.NextWindowData.SizeVal.y > 0.0f);
		SetWindowSize(window, g.NextWindowData.SizeVal, g.NextWindowData.SizeCond);
	}
	if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasContentSize)
		window->ContentSizeExplicit = g.NextWindowData.ContentSizeVal;
	else if (first_begin_of_the_frame)
		window->ContentSizeExplicit = ImVec2(0.0f, 0.0f);
	if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasCollapsed)
		SetWindowCollapsed(window, g.NextWindowData.CollapsedVal, g.NextWindowData.CollapsedCond);
	if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasFocus)
		FocusWindow(window);
	if (window->Appearing)
		SetWindowConditionAllowFlags(window, ImGuiCond_Appearing, false);

	// When reusing window again multiple times a frame, just append content (don't need to setup again)
	if (first_begin_of_the_frame)
	{
		// Initialize
		const bool window_is_child_tooltip = (flags & ImGuiWindowFlags_ChildWindow) && (flags & ImGuiWindowFlags_Tooltip); // FIXME-WIP: Undocumented behavior of Child+Tooltip for pinned tooltip (#1345)
		UpdateWindowParentAndRootLinks(window, flags, parent_window);

		window->Active = true;
		window->HasCloseButton = (p_open != NULL);
		window->ClipRect = ImVec4(-FLT_MAX, -FLT_MAX, +FLT_MAX, +FLT_MAX);
		window->IDStack.resize(1);

		// Update stored window name when it changes (which can _only_ happen with the "###" operator, so the ID would stay unchanged).
		// The title bar always display the 'name' parameter, so we only update the string storage if it needs to be visible to the end-user elsewhere.
		bool window_title_visible_elsewhere = false;
		if (g.NavWindowingList != NULL && (window->Flags & ImGuiWindowFlags_NoNavFocus) == 0)   // Window titles visible when using CTRL+TAB
			window_title_visible_elsewhere = true;
		if (window_title_visible_elsewhere && !window_just_created && strcmp(name, window->Name) != 0)
		{
			size_t buf_len = (size_t)window->NameBufLen;
			window->Name = ImStrdupcpy(window->Name, &buf_len, name);
			window->NameBufLen = (int)buf_len;
		}

		// UPDATE CONTENTS SIZE, UPDATE HIDDEN STATUS

		// Update contents size from last frame for auto-fitting (or use explicit size)
		window->ContentSize = CalcContentSize(window);
		if (window->HiddenFramesCanSkipItems > 0)
			window->HiddenFramesCanSkipItems--;
		if (window->HiddenFramesCannotSkipItems > 0)
			window->HiddenFramesCannotSkipItems--;

		// Hide new windows for one frame until they calculate their size
		if (window_just_created && (!window_size_x_set_by_api || !window_size_y_set_by_api))
			window->HiddenFramesCannotSkipItems = 1;

		// Hide popup/tooltip window when re-opening while we measure size (because we recycle the windows)
		// We reset Size/ContentSize for reappearing popups/tooltips early in this function, so further code won't be tempted to use the old size.
		if (window_just_activated_by_user && (flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) != 0)
		{
			window->HiddenFramesCannotSkipItems = 1;
			if (flags & ImGuiWindowFlags_AlwaysAutoResize)
			{
				if (!window_size_x_set_by_api)
					window->Size.x = window->SizeFull.x = 0.f;
				if (!window_size_y_set_by_api)
					window->Size.y = window->SizeFull.y = 0.f;
				window->ContentSize = ImVec2(0.f, 0.f);
			}
		}

		// FIXME-VIEWPORT: In the docking/viewport branch, this is the point where we select the current viewport (which may affect the style)
		SetCurrentWindow(window);

		// LOCK BORDER SIZE AND PADDING FOR THE FRAME (so that altering them doesn't cause inconsistencies)

		if (flags & ImGuiWindowFlags_ChildWindow)
			window->WindowBorderSize = style.ChildBorderSize;
		else
			window->WindowBorderSize = ((flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupBorderSize : style.WindowBorderSize;
		window->WindowPadding = style.WindowPadding;
		if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & (ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_Popup)) && window->WindowBorderSize == 0.0f)
			window->WindowPadding = ImVec2(0.0f, (flags & ImGuiWindowFlags_MenuBar) ? style.WindowPadding.y : 0.0f);
		window->DC.MenuBarOffset.x = ImMax(ImMax(window->WindowPadding.x, style.ItemSpacing.x), g.NextWindowData.MenuBarOffsetMinVal.x);
		window->DC.MenuBarOffset.y = g.NextWindowData.MenuBarOffsetMinVal.y;

		// Collapse window by double-clicking on title bar
		// At this point we don't have a clipping rectangle setup yet, so we can use the title bar area for hit detection and drawing
		if (!(flags & ImGuiWindowFlags_NoTitleBar) && !(flags & ImGuiWindowFlags_NoCollapse))
		{
			// We don't use a regular button+id to test for double-click on title bar (mostly due to legacy reason, could be fixed), so verify that we don't have items over the title bar.
			ImRect title_bar_rect = window->TitleBarRect();
			if (g.HoveredWindow == window && g.HoveredId == 0 && g.HoveredIdPreviousFrame == 0 && IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max) && g.IO.MouseDoubleClicked[0])
				window->WantCollapseToggle = true;
			if (window->WantCollapseToggle)
			{
				window->Collapsed = !window->Collapsed;
				MarkIniSettingsDirty(window);
				FocusWindow(window);
			}
		}
		else
		{
			window->Collapsed = false;
		}
		window->WantCollapseToggle = false;

		// SIZE

		// Calculate auto-fit size, handle automatic resize
		const ImVec2 size_auto_fit = CalcSizeAutoFit(window, window->ContentSize);
		bool use_current_size_for_scrollbar_x = window_just_created;
		bool use_current_size_for_scrollbar_y = window_just_created;
		if ((flags & ImGuiWindowFlags_AlwaysAutoResize) && !window->Collapsed)
		{
			// Using SetNextWindowSize() overrides ImGuiWindowFlags_AlwaysAutoResize, so it can be used on tooltips/popups, etc.
			if (!window_size_x_set_by_api)
			{
				window->SizeFull.x = size_auto_fit.x;
				use_current_size_for_scrollbar_x = true;
			}
			if (!window_size_y_set_by_api)
			{
				window->SizeFull.y = size_auto_fit.y;
				use_current_size_for_scrollbar_y = true;
			}
		}
		else if (window->AutoFitFramesX > 0 || window->AutoFitFramesY > 0)
		{
			// Auto-fit may only grow window during the first few frames
			// We still process initial auto-fit on collapsed windows to get a window width, but otherwise don't honor ImGuiWindowFlags_AlwaysAutoResize when collapsed.
			if (!window_size_x_set_by_api && window->AutoFitFramesX > 0)
			{
				window->SizeFull.x = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.x, size_auto_fit.x) : size_auto_fit.x;
				use_current_size_for_scrollbar_x = true;
			}
			if (!window_size_y_set_by_api && window->AutoFitFramesY > 0)
			{
				window->SizeFull.y = window->AutoFitOnlyGrows ? ImMax(window->SizeFull.y, size_auto_fit.y) : size_auto_fit.y;
				use_current_size_for_scrollbar_y = true;
			}
			if (!window->Collapsed)
				MarkIniSettingsDirty(window);
		}

		// Apply minimum/maximum window size constraints and final size
		window->SizeFull = CalcSizeAfterConstraint(window, window->SizeFull);
		window->Size = window->Collapsed && !(flags & ImGuiWindowFlags_ChildWindow) ? window->TitleBarRect().GetSize() : window->SizeFull;

		// Decoration size
		const float decoration_up_height = window->TitleBarHeight() + window->MenuBarHeight();

		// POSITION

		// Popup latch its initial position, will position itself when it appears next frame
		if (window_just_activated_by_user)
		{
			window->AutoPosLastDirection = ImGuiDir_None;
			if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api)
				window->Pos = g.BeginPopupStack.back().OpenPopupPos;
		}

		// Position child window
		if (flags & ImGuiWindowFlags_ChildWindow)
		{
			IM_ASSERT(parent_window && parent_window->Active);
			window->BeginOrderWithinParent = (short)parent_window->DC.ChildWindows.Size;
			parent_window->DC.ChildWindows.push_back(window);
			if (!(flags & ImGuiWindowFlags_Popup) && !window_pos_set_by_api && !window_is_child_tooltip)
				window->Pos = parent_window->DC.CursorPos;
		}

		const bool window_pos_with_pivot = (window->SetWindowPosVal.x != FLT_MAX && window->HiddenFramesCannotSkipItems == 0);
		if (window_pos_with_pivot)
			SetWindowPos(window, window->SetWindowPosVal - window->SizeFull * window->SetWindowPosPivot, 0); // Position given a pivot (e.g. for centering)
		else if ((flags & ImGuiWindowFlags_ChildMenu) != 0)
			window->Pos = FindBestWindowPosForPopup(window);
		else if ((flags & ImGuiWindowFlags_Popup) != 0 && !window_pos_set_by_api && window_just_appearing_after_hidden_for_resize)
			window->Pos = FindBestWindowPosForPopup(window);
		else if ((flags & ImGuiWindowFlags_Tooltip) != 0 && !window_pos_set_by_api && !window_is_child_tooltip)
			window->Pos = FindBestWindowPosForPopup(window);

		// Clamp position/size so window stays visible within its viewport or monitor

		// Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
		ImRect viewport_rect(GetViewportRect());
		if (!window_pos_set_by_api && !(flags & ImGuiWindowFlags_ChildWindow) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
		{
			if (g.IO.DisplaySize.x > 0.0f && g.IO.DisplaySize.y > 0.0f) // Ignore zero-sized display explicitly to avoid losing positions if a window manager reports zero-sized window when initializing or minimizing.
			{
				ImVec2 clamp_padding = ImMax(style.DisplayWindowPadding, style.DisplaySafeAreaPadding);
				ClampWindowRect(window, viewport_rect, clamp_padding);
			}
		}
		window->Pos = ImFloor(window->Pos);

		// Lock window rounding for the frame (so that altering them doesn't cause inconsistencies)
		window->WindowRounding = (flags & ImGuiWindowFlags_ChildWindow) ? style.ChildRounding : ((flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiWindowFlags_Modal)) ? style.PopupRounding : style.WindowRounding;

		// Apply window focus (new and reactivated windows are moved to front)
		bool want_focus = false;
		if (window_just_activated_by_user && !(flags & ImGuiWindowFlags_NoFocusOnAppearing))
		{
			if (flags & ImGuiWindowFlags_Popup)
				want_focus = true;
			else if ((flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Tooltip)) == 0)
				want_focus = true;
		}

		// Handle manual resize: Resize Grips, Borders, Gamepad
		int border_held = -1;
		ImU32 resize_grip_col[4] = { 0 };
		const int resize_grip_count = g.IO.ConfigWindowsResizeFromEdges ? 2 : 1; // 4
		const float resize_grip_draw_size = (float)(int)ImMax(g.FontSize * 1.35f, window->WindowRounding + 1.0f + g.FontSize * 0.2f);
		if (!window->Collapsed)
			if (UpdateManualResize(window, size_auto_fit, &border_held, resize_grip_count, &resize_grip_col[0]))
				use_current_size_for_scrollbar_x = use_current_size_for_scrollbar_y = true;
		window->ResizeBorderHeld = (signed char)border_held;

		// SCROLLBAR VISIBILITY

		// Update scrollbar visibility (based on the Size that was effective during last frame or the auto-resized Size).
		if (!window->Collapsed)
		{
			// When reading the current size we need to read it after size constraints have been applied.
			// When we use InnerRect here we are intentionally reading last frame size, same for ScrollbarSizes values before we set them again.
			ImVec2 avail_size_from_current_frame = ImVec2(window->SizeFull.x, window->SizeFull.y - decoration_up_height);
			ImVec2 avail_size_from_last_frame = window->InnerRect.GetSize() + window->ScrollbarSizes;
			ImVec2 needed_size_from_last_frame = window_just_created ? ImVec2(0, 0) : window->ContentSize + window->WindowPadding * 2.0f;
			float size_x_for_scrollbars = use_current_size_for_scrollbar_x ? avail_size_from_current_frame.x : avail_size_from_last_frame.x;
			float size_y_for_scrollbars = use_current_size_for_scrollbar_y ? avail_size_from_current_frame.y : avail_size_from_last_frame.y;
			//bool scrollbar_y_from_last_frame = window->ScrollbarY; // FIXME: May want to use that in the ScrollbarX expression? How many pros vs cons?
			window->ScrollbarY = (flags & ImGuiWindowFlags_AlwaysVerticalScrollbar) || ((needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar));
			window->ScrollbarX = (flags & ImGuiWindowFlags_AlwaysHorizontalScrollbar) || ((needed_size_from_last_frame.x > size_x_for_scrollbars - (window->ScrollbarY ? style.ScrollbarSize : 0.0f)) && !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar));
			if (window->ScrollbarX && !window->ScrollbarY)
				window->ScrollbarY = (needed_size_from_last_frame.y > size_y_for_scrollbars) && !(flags & ImGuiWindowFlags_NoScrollbar);
			window->ScrollbarSizes = ImVec2(window->ScrollbarY ? style.ScrollbarSize : 0.0f, window->ScrollbarX ? style.ScrollbarSize : 0.0f);
		}

		// UPDATE RECTANGLES (1- THOSE NOT AFFECTED BY SCROLLING)
		// Update various regions. Variables they depends on should be set above in this function.
		// We set this up after processing the resize grip so that our rectangles doesn't lag by a frame.

		// Outer rectangle
		// Not affected by window border size. Used by:
		// - FindHoveredWindow() (w/ extra padding when border resize is enabled)
		// - Begin() initial clipping rect for drawing window background and borders.
		// - Begin() clipping whole child
		const ImRect host_rect = ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip) ? parent_window->ClipRect : viewport_rect;
		const ImRect outer_rect = window->Rect();
		const ImRect title_bar_rect = window->TitleBarRect();
		window->OuterRectClipped = outer_rect;
		window->OuterRectClipped.ClipWith(host_rect);

		// Inner rectangle
		// Not affected by window border size. Used by: 
		// - InnerClipRect
		// - NavScrollToBringItemIntoView()
		// - NavUpdatePageUpPageDown()
		// - Scrollbar()
		window->InnerRect.Min.x = window->Pos.x;
		window->InnerRect.Min.y = window->Pos.y + decoration_up_height;
		window->InnerRect.Max.x = window->Pos.x + window->Size.x/* - window->ScrollbarSizes.x*/;
		window->InnerRect.Max.y = window->Pos.y + window->Size.y/* - window->ScrollbarSizes.y*/;

		// Inner clipping rectangle.
		// Will extend a little bit outside the normal work region.
		// This is to allow e.g. Selectable or CollapsingHeader or some separators to cover that space.
		// Force round operator last to ensure that e.g. (int)(max.x-min.x) in user's render code produce correct result.
		// Note that if our window is collapsed we will end up with an inverted (~null) clipping rectangle which is the correct behavior.
		// Affected by window/frame border size. Used by:
		// - Begin() initial clip rect
		float top_border_size = (((flags & ImGuiWindowFlags_MenuBar) || !(flags & ImGuiWindowFlags_NoTitleBar)) ? style.FrameBorderSize : window->WindowBorderSize);
		window->InnerClipRect.Min.x = ImFloor(0.5f + window->InnerRect.Min.x + window->WindowBorderSize);
		window->InnerClipRect.Min.y = ImFloor(0.5f + window->InnerRect.Min.y + top_border_size);
		window->InnerClipRect.Max.x = ImFloor(0.5f + window->InnerRect.Max.x - window->WindowBorderSize);
		window->InnerClipRect.Max.y = ImFloor(0.5f + window->InnerRect.Max.y - window->WindowBorderSize);
		window->InnerClipRect.ClipWithFull(host_rect);

		// Default item width. Make it proportional to window size if window manually resizes
		if (window->Size.x > 0.0f && !(flags & ImGuiWindowFlags_Tooltip) && !(flags & ImGuiWindowFlags_AlwaysAutoResize))
			window->ItemWidthDefault = (float)(int)(window->Size.x * 0.65f);
		else
			window->ItemWidthDefault = (float)(int)(g.FontSize * 16.0f);

		// SCROLLING

		// Lock down maximum scrolling
		// The value of ScrollMax are ahead from ScrollbarX/ScrollbarY which is intentionally using InnerRect from previous rect in order to accommodate
		// for right/bottom aligned items without creating a scrollbar.
		window->ScrollMax.x = ImMax(0.0f, window->ContentSize.x + window->WindowPadding.x * 2.0f - window->InnerRect.GetWidth());
		window->ScrollMax.y = ImMax(0.0f, window->ContentSize.y + window->WindowPadding.y * 2.0f - window->InnerRect.GetHeight());

		// Apply scrolling
		window->Scroll = CalcNextScrollFromScrollTargetAndClamp(window, true);
		window->ScrollTarget = ImVec2(FLT_MAX, FLT_MAX);

		// DRAWING

		// Setup draw list and outer clipping rectangle
		window->DrawList->Clear();
		window->DrawList->PushTextureID(g.Font->ContainerAtlas->TexID);
		PushClipRect(host_rect.Min, host_rect.Max, false);

		// Draw modal window background (darkens what is behind them, all viewports)
		const bool dim_bg_for_modal = (flags & ImGuiWindowFlags_Modal) && window == GetTopMostPopupModal() && window->HiddenFramesCannotSkipItems <= 0;
		const bool dim_bg_for_window_list = g.NavWindowingTargetAnim && (window == g.NavWindowingTargetAnim->RootWindow);
		if (dim_bg_for_modal || dim_bg_for_window_list)
		{
			const ImU32 dim_bg_col = GetColorU32(dim_bg_for_modal ? ImGuiCol_ModalWindowDimBg : ImGuiCol_NavWindowingDimBg, g.DimBgRatio);
			window->DrawList->AddRectFilled(viewport_rect.Min, viewport_rect.Max, dim_bg_col);
		}

		// Draw navigation selection/windowing rectangle background
		if (dim_bg_for_window_list && window == g.NavWindowingTargetAnim)
		{
			ImRect bb = window->Rect();
			bb.Expand(g.FontSize);
			if (!bb.Contains(viewport_rect)) // Avoid drawing if the window covers all the viewport anyway
				window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha * 0.25f), g.Style.WindowRounding);
		}

		// Since 1.71, child window can render their decoration (bg color, border, scrollbars, etc.) within their parent to save a draw call.
		// When using overlapping child windows, this will break the assumption that child z-order is mapped to submission order.
		// We disable this when the parent window has zero vertices, which is a common pattern leading to laying out multiple overlapping child.
		// We also disabled this when we have dimming overlay behind this specific one child.
		// FIXME: More code may rely on explicit sorting of overlapping child window and would need to disable this somehow. Please get in contact if you are affected.
		bool render_decorations_in_parent = false;
		if ((flags & ImGuiWindowFlags_ChildWindow) && !(flags & ImGuiWindowFlags_Popup) && !window_is_child_tooltip)
			if (window->DrawList->CmdBuffer.back().ElemCount == 0 && parent_window->DrawList->VtxBuffer.Size > 0)
				render_decorations_in_parent = true;
		if (render_decorations_in_parent)
			window->DrawList = parent_window->DrawList;

		// Handle title bar, scrollbar, resize grips and resize borders
		const ImGuiWindow* window_to_highlight = g.NavWindowingTarget ? g.NavWindowingTarget : g.NavWindow;
		const bool title_bar_is_highlight = want_focus || (window_to_highlight && window->RootWindowForTitleBarHighlight == window_to_highlight->RootWindowForTitleBarHighlight);

		{
			ImGuiContext& g = *GImGui;
			ImGuiStyle& style = g.Style;
			ImGuiWindowFlags flags = window->Flags;

			// Draw window + handle manual resize
			// As we highlight the title bar when want_focus is set, multiple reappearing windows will have have their title bar highlighted on their reappearing frame.
			const float window_rounding = window->WindowRounding;
			const float window_border_size = window->WindowBorderSize;
			if (window->Collapsed)
			{
				// Title bar only
				float backup_border_size = style.FrameBorderSize;
				g.Style.FrameBorderSize = window->WindowBorderSize;
				ImU32 title_bar_col = GetColorU32((title_bar_is_highlight && !g.NavDisableHighlight) ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBgCollapsed);
				RenderFrame(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, true, window_rounding);
				g.Style.FrameBorderSize = backup_border_size;
			}
			else
			{
				// Window background
				if (!(flags & ImGuiWindowFlags_NoBackground))
				{
					ImU32 bg_col = GetColorU32(GetWindowBgColorIdxFromFlags(flags));
					float alpha = 1.0f;
					if (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasBgAlpha)
						alpha = g.NextWindowData.BgAlphaVal;
					if (alpha != 1.0f)
						bg_col = (bg_col & ~IM_COL32_A_MASK) | (IM_F32_TO_INT8_SAT(alpha) << IM_COL32_A_SHIFT);
					window->DrawList->AddRectFilled(window->Pos + ImVec2(0, window->TitleBarHeight()), window->Pos + window->Size, bg_col, window_rounding, (flags & ImGuiWindowFlags_NoTitleBar) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Bot);
				}

				// Title bar
				if (!(flags & ImGuiWindowFlags_NoTitleBar))
				{
					ImU32 title_bar_col = GetColorU32(title_bar_is_highlight ? ImGuiCol_TitleBgActive : ImGuiCol_TitleBg);
					window->DrawList->AddRectFilled(title_bar_rect.Min, title_bar_rect.Max, title_bar_col, window_rounding, ImDrawCornerFlags_Top);
				}

				// Menu bar
				if (flags & ImGuiWindowFlags_MenuBar)
				{
					ImRect menu_bar_rect = window->MenuBarRect();
					menu_bar_rect.ClipWith(window->Rect());  // Soft clipping, in particular child window don't have minimum size covering the menu bar so this is useful for them.
					window->DrawList->AddRectFilled(menu_bar_rect.Min + ImVec2(window_border_size, 0), menu_bar_rect.Max - ImVec2(window_border_size, 0), GetColorU32(ImGuiCol_MenuBarBg), (flags & ImGuiWindowFlags_NoTitleBar) ? window_rounding : 0.0f, ImDrawCornerFlags_Top);
					if (style.FrameBorderSize > 0.0f && menu_bar_rect.Max.y < window->Pos.y + window->Size.y)
						window->DrawList->AddLine(menu_bar_rect.GetBL(), menu_bar_rect.GetBR(), GetColorU32(ImGuiCol_Border), style.FrameBorderSize);
				}

				// Scrollbars
				if (window->ScrollbarX)
					Scrollbar(ImGuiAxis_X);
				if (window->ScrollbarY)
					Scrollbar(ImGuiAxis_Y);

				// Render resize grips (after their input handling so we don't have a frame of latency)
				if (!(flags & ImGuiWindowFlags_NoResize))
				{
					for (int resize_grip_n = 0; resize_grip_n < resize_grip_count; resize_grip_n++)
					{
						const ImGuiResizeGripDef& grip = resize_grip_def[resize_grip_n];
						const ImVec2 corner = ImLerp(window->Pos, window->Pos + window->Size, grip.CornerPosN);
						window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(window_border_size, resize_grip_draw_size) : ImVec2(resize_grip_draw_size, window_border_size)));
						window->DrawList->PathLineTo(corner + grip.InnerDir * ((resize_grip_n & 1) ? ImVec2(resize_grip_draw_size, window_border_size) : ImVec2(window_border_size, resize_grip_draw_size)));
						window->DrawList->PathArcToFast(ImVec2(corner.x + grip.InnerDir.x * (window_rounding + window_border_size), corner.y + grip.InnerDir.y * (window_rounding + window_border_size)), window_rounding, grip.AngleMin12, grip.AngleMax12);
						window->DrawList->PathFillConvex(resize_grip_col[resize_grip_n]);
					}
				}

				// Borders
				{
					ImGuiContext& g = *GImGui;
					float rounding = window->WindowRounding;
					float border_size = window->WindowBorderSize;
					if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
					{
						const ImFont* pFont = ImGui::GetFont();
						const ImVec2 text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, name, FindRenderedTextEnd(name));

						g_name = name;

						window->DrawList->AddLine(window->Pos, ImVec2(window->Pos.x, window->Pos.y + window->Size.y - 1), GetColorU32(ImGuiCol_Border), border_size);
						window->DrawList->AddLine(ImVec2(window->Pos.x, window->Pos.y + window->Size.y) - ImVec2(0, 1), window->Pos + window->Size - 1, GetColorU32(ImGuiCol_Border), border_size);
						window->DrawList->AddLine(window->Pos + window->Size - 1, ImVec2(window->Pos.x + window->Size.x - 1, window->Pos.y), GetColorU32(ImGuiCol_Border), border_size);
						
						const char* text_display_end = FindRenderedTextEnd(name, NULL);

						if (name != text_display_end)
						{
							window->DrawList->AddLine(window->Pos, ImVec2(window->Pos.x + 15.f, window->Pos.y), GetColorU32(ImGuiCol_Border), border_size);
							window->DrawList->AddLine(ImVec2(window->Pos.x + 21.f + text_size.x, window->Pos.y), ImVec2(window->Pos.x + window->Size.x, window->Pos.y), GetColorU32(ImGuiCol_Border), border_size);
						}
						else
						{
							window->DrawList->AddLine(window->Pos, ImVec2(window->Pos.x + window->Size.x, window->Pos.y), GetColorU32(ImGuiCol_Border), border_size);
						}

						//RenderText(ImVec2(window->Pos.x + 18.f, window->Pos.y - text_size.y * 0.5f), name, NULL, true);
						//RenderText(ImVec2(window->Pos.x + 18.f, window->Pos.y - text_size.y * 0.5f), name, NULL, true);
					}
				}
			}
		}

		if (render_decorations_in_parent)
			window->DrawList = &window->DrawListInst;

		// Draw navigation selection/windowing rectangle border
		if (g.NavWindowingTargetAnim == window)
		{
			float rounding = ImMax(window->WindowRounding, g.Style.WindowRounding);
			ImRect bb = window->Rect();
			bb.Expand(g.FontSize);
			if (bb.Contains(viewport_rect)) // If a window fits the entire viewport, adjust its highlight inward
			{
				bb.Expand(-g.FontSize - 1.0f);
				rounding = window->WindowRounding;
			}
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_NavWindowingHighlight, g.NavWindowingHighlightAlpha), rounding, ~0, 3.0f);
		}

		// UPDATE RECTANGLES (2- THOSE AFFECTED BY SCROLLING)

		// Work rectangle.
		// Affected by window padding and border size. Used by:
		// - Columns() for right-most edge
		// - TreeNode(), CollapsingHeader() for right-most edge
		// - BeginTabBar() for right-most edge
		const bool allow_scrollbar_x = !(flags & ImGuiWindowFlags_NoScrollbar) && (flags & ImGuiWindowFlags_HorizontalScrollbar);
		const bool allow_scrollbar_y = !(flags & ImGuiWindowFlags_NoScrollbar);
		const float work_rect_size_x = (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : ImMax(allow_scrollbar_x ? window->ContentSize.x : 0.0f, window->Size.x - window->WindowPadding.x * 2.0f - window->ScrollbarSizes.x));
		const float work_rect_size_y = (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : ImMax(allow_scrollbar_y ? window->ContentSize.y : 0.0f, window->Size.y - window->WindowPadding.y * 2.0f - decoration_up_height - window->ScrollbarSizes.y));
		window->WorkRect.Min.x = ImFloor(window->InnerRect.Min.x - window->Scroll.x + ImMax(window->WindowPadding.x, window->WindowBorderSize));
		window->WorkRect.Min.y = ImFloor(window->InnerRect.Min.y - window->Scroll.y + ImMax(window->WindowPadding.y, window->WindowBorderSize));
		window->WorkRect.Max.x = window->WorkRect.Min.x + work_rect_size_x;
		window->WorkRect.Max.y = window->WorkRect.Min.y + work_rect_size_y;

		// [LEGACY] Contents Region
		// FIXME-OBSOLETE: window->ContentsRegionRect.Max is currently very misleading / partly faulty, but some BeginChild() patterns relies on it.
		// Used by:
		// - Mouse wheel scrolling + many other things
		window->ContentsRegionRect.Min.x = window->Pos.x - window->Scroll.x + window->WindowPadding.x;
		window->ContentsRegionRect.Min.y = window->Pos.y - window->Scroll.y + window->WindowPadding.y + decoration_up_height;
		window->ContentsRegionRect.Max.x = window->ContentsRegionRect.Min.x + (window->ContentSizeExplicit.x != 0.0f ? window->ContentSizeExplicit.x : (window->Size.x - window->WindowPadding.x * 2.0f - window->ScrollbarSizes.x));
		window->ContentsRegionRect.Max.y = window->ContentsRegionRect.Min.y + (window->ContentSizeExplicit.y != 0.0f ? window->ContentSizeExplicit.y : (window->Size.y - window->WindowPadding.y * 2.0f - decoration_up_height - window->ScrollbarSizes.y));

		// Setup drawing context
		// (NB: That term "drawing context / DC" lost its meaning a long time ago. Initially was meant to hold transient data only. Nowadays difference between window-> and window->DC-> is dubious.)
		window->DC.Indent.x = 0.0f + window->WindowPadding.x - window->Scroll.x;
		window->DC.GroupOffset.x = 0.0f;
		window->DC.ColumnsOffset.x = 0.0f;
		window->DC.CursorStartPos = window->Pos + ImVec2(window->DC.Indent.x + window->DC.ColumnsOffset.x, decoration_up_height + window->WindowPadding.y - window->Scroll.y);
		window->DC.CursorPos = window->DC.CursorStartPos;
		window->DC.CursorPosPrevLine = window->DC.CursorPos;
		window->DC.CursorMaxPos = window->DC.CursorStartPos;
		window->DC.CurrLineSize = window->DC.PrevLineSize = ImVec2(0.0f, 0.0f);
		window->DC.CurrLineTextBaseOffset = window->DC.PrevLineTextBaseOffset = 0.0f;
		window->DC.NavHideHighlightOneFrame = false;
		window->DC.NavHasScroll = (window->ScrollMax.y > 0.0f);
		window->DC.NavLayerActiveMask = window->DC.NavLayerActiveMaskNext;
		window->DC.NavLayerActiveMaskNext = 0x00;
		window->DC.MenuBarAppending = false;
		window->DC.ChildWindows.resize(0);
		window->DC.LayoutType = ImGuiLayoutType_Vertical;
		window->DC.ParentLayoutType = parent_window ? parent_window->DC.LayoutType : ImGuiLayoutType_Vertical;
		window->DC.FocusCounterAll = window->DC.FocusCounterTab = -1;
		window->DC.ItemFlags = parent_window ? parent_window->DC.ItemFlags : ImGuiItemFlags_Default_;
		window->DC.ItemWidth = window->ItemWidthDefault;
		window->DC.TextWrapPos = -1.0f; // disabled
		window->DC.ItemFlagsStack.resize(0);
		window->DC.ItemWidthStack.resize(0);
		window->DC.TextWrapPosStack.resize(0);
		window->DC.CurrentColumns = NULL;
		window->DC.TreeDepth = 0;
		window->DC.TreeStoreMayJumpToParentOnPop = 0x00;
		window->DC.StateStorage = &window->StateStorage;
		window->DC.GroupStack.resize(0);
		window->MenuColumns.Update(3, style.ItemSpacing.x, window_just_activated_by_user);

		if ((flags & ImGuiWindowFlags_ChildWindow) && (window->DC.ItemFlags != parent_window->DC.ItemFlags))
		{
			window->DC.ItemFlags = parent_window->DC.ItemFlags;
			window->DC.ItemFlagsStack.push_back(window->DC.ItemFlags);
		}

		if (window->AutoFitFramesX > 0)
			window->AutoFitFramesX--;
		if (window->AutoFitFramesY > 0)
			window->AutoFitFramesY--;

		// Apply focus (we need to call FocusWindow() AFTER setting DC.CursorStartPos so our initial navigation reference rectangle can start around there)
		if (want_focus)
		{
			FocusWindow(window);
			NavInitWindow(window, false);
		}

		// Title bar
		/*if (!(flags & ImGuiWindowFlags_NoTitleBar))
			RenderWindowTitleBarContents(window, title_bar_rect, name, p_open);*/

			// Pressing CTRL+C while holding on a window copy its content to the clipboard
			// This works but 1. doesn't handle multiple Begin/End pairs, 2. recursing into another Begin/End pair - so we need to work that out and add better logging scope.
			// Maybe we can support CTRL+C on every element?
			/*
			if (g.ActiveId == move_id)
				if (g.IO.KeyCtrl && IsKeyPressedMap(ImGuiKey_C))
					LogToClipboard();
			*/

			// We fill last item data based on Title Bar/Tab, in order for IsItemHovered() and IsItemActive() to be usable after Begin().
			// This is useful to allow creating context menus on title bar only, etc.
		window->DC.LastItemId = window->MoveId;
		window->DC.LastItemStatusFlags = IsMouseHoveringRect(title_bar_rect.Min, title_bar_rect.Max, false) ? ImGuiItemStatusFlags_HoveredRect : 0;
		window->DC.LastItemRect = title_bar_rect;
#ifdef IMGUI_ENABLE_TEST_ENGINE
		if (!(window->Flags & ImGuiWindowFlags_NoTitleBar))
			IMGUI_TEST_ENGINE_ITEM_ADD(window->DC.LastItemRect, window->DC.LastItemId);
#endif
	}
	else
	{
		// Append
		SetCurrentWindow(window);
	}

	PushClipRect(window->InnerClipRect.Min, window->InnerClipRect.Max, true);

	// Clear 'accessed' flag last thing (After PushClipRect which will set the flag. We want the flag to stay false when the default "Debug" window is unused)
	if (first_begin_of_the_frame)
		window->WriteAccessed = false;

	window->BeginCount++;
	g.NextWindowData.ClearFlags();

	if (flags & ImGuiWindowFlags_ChildWindow)
	{
		// Child window can be out of sight and have "negative" clip windows.
		// Mark them as collapsed so commands are skipped earlier (we can't manually collapse them because they have no title bar).
		IM_ASSERT((flags & ImGuiWindowFlags_NoTitleBar) != 0);
		if (!(flags & ImGuiWindowFlags_AlwaysAutoResize) && window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0)
			if (window->OuterRectClipped.Min.x >= window->OuterRectClipped.Max.x || window->OuterRectClipped.Min.y >= window->OuterRectClipped.Max.y)
				window->HiddenFramesCanSkipItems = 1;

		// Hide along with parent or if parent is collapsed
		if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCanSkipItems > 0))
			window->HiddenFramesCanSkipItems = 1;
		if (parent_window && (parent_window->Collapsed || parent_window->HiddenFramesCannotSkipItems > 0))
			window->HiddenFramesCannotSkipItems = 1;
	}

	// Don't render if style alpha is 0.0 at the time of Begin(). This is arbitrary and inconsistent but has been there for a long while (may remove at some point)
	if (style.Alpha <= 0.0f)
		window->HiddenFramesCanSkipItems = 1;

	// Update the Hidden flag
	window->Hidden = (window->HiddenFramesCanSkipItems > 0) || (window->HiddenFramesCannotSkipItems > 0);

	// Update the SkipItems flag, used to early out of all items functions (no layout required)
	bool skip_items = false;
	if (window->Collapsed || !window->Active || window->Hidden)
		if (window->AutoFitFramesX <= 0 && window->AutoFitFramesY <= 0 && window->HiddenFramesCannotSkipItems <= 0)
			skip_items = true;
	window->SkipItems = skip_items;

	return !skip_items;
}

static bool ImGui::BeginChildCustomEx(const char* name, ImGuiID id, const ImVec2& size_arg, bool border, ImGuiWindowFlags flags)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* parent_window = g.CurrentWindow;

	flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;
	flags |= (parent_window->Flags & ImGuiWindowFlags_NoMove);  // Inherit the NoMove flag

	// Size
	const ImVec2 content_avail = GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	const int auto_fit_axises = ((size.x == 0.0f) ? (1 << ImGuiAxis_X) : 0x00) | ((size.y == 0.0f) ? (1 << ImGuiAxis_Y) : 0x00);
	if (size.x <= 0.0f)
		size.x = ImMax(content_avail.x + size.x, 4.0f); // Arbitrary minimum child size (0.0f causing too much issues)
	if (size.y <= 0.0f)
		size.y = ImMax(content_avail.y + size.y, 4.0f);
	SetNextWindowSize(size);

	const float backup_border_size = g.Style.ChildBorderSize;
	if (!border)
		g.Style.ChildBorderSize = 0.0f;
	bool ret = BeginCustom(name, NULL, flags);
	g.Style.ChildBorderSize = backup_border_size;

	ImGuiWindow* child_window = g.CurrentWindow;
	child_window->ChildId = id;
	child_window->AutoFitChildAxises = auto_fit_axises;

	// Set the cursor to handle case where the user called SetNextWindowPos()+BeginChild() manually.
	// While this is not really documented/defined, it seems that the expected thing to do.
	if (child_window->BeginCount == 1)
		parent_window->DC.CursorPos = child_window->Pos;

	// Process navigation-in immediately so NavInit can run on first frame
	if (g.NavActivateId == id && !(flags & ImGuiWindowFlags_NavFlattened) && (child_window->DC.NavLayerActiveMask != 0 || child_window->DC.NavHasScroll))
	{
		FocusWindow(child_window);
		NavInitWindow(child_window, false);
		SetActiveID(id + 1, child_window); // Steal ActiveId with a dummy id so that key-press won't activate child item
		g.ActiveIdSource = ImGuiInputSource_Nav;
	}
	return ret;
}

bool ImGui::BeginChildCustom(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	return BeginChildCustomEx(str_id, window->GetID(str_id), size_arg, border, extra_flags);
}

void ImGui::EndChildCustom()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	IM_ASSERT(window->Flags & ImGuiWindowFlags_ChildWindow);   // Mismatched BeginChild()/EndChild() callss

	ImVec2 window_pos = GImGui->CurrentWindow->Pos;
	ImVec2 window_size = GImGui->CurrentWindow->Size;

	ImVec2 min_top = window_pos;
	ImVec2 max_top = min_top + ImVec2(window_size.x, min(window->Scroll.y, 20));
	ImVec2 min_bottom = min_top + ImVec2(0, window_size.y - min(window->ScrollMax.y - window->Scroll.y, 20));
	ImVec2 max_bottom = min_bottom + ImVec2(window_size.x, 20);

	if (window->ScrollbarY)
	{
		ImColor color_windowbg = GetStyleColorVec4(ImGuiCol_ChildBg);

		ImColor color_top = color_windowbg;
		ImColor color_bottom = color_top;
		color_bottom.value.w = 0.00f;

		if (window->Scroll.y > 1.0f)
			window->DrawList->AddRectFilledMultiColor(min_top, max_top, color_top, color_top, color_bottom, color_bottom);

		if ((GetScrollMaxY() - window->Scroll.y) > 1.f)
			window->DrawList->AddRectFilledMultiColor(min_bottom, max_bottom, color_bottom, color_bottom, color_top, color_top);
	}

	const ImFont* pFont = ImGui::GetFont();
	const ImVec2 text_size = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, g_name.c_str(), FindRenderedTextEnd(g_name.c_str()));
	RenderText(ImVec2(window->Pos.x + 18.f, window->Pos.y - text_size.y * 0.5f), g_name.c_str(), NULL, true);

	if (window->BeginCount > 1)
	{
		End();
	}
	else
	{
		ImVec2 sz = window->Size;
		if (window->AutoFitChildAxises & (1 << ImGuiAxis_X)) // Arbitrary minimum zero-ish child size of 4.0f causes less trouble than a 0.0f
			sz.x = ImMax(4.0f, sz.x);
		if (window->AutoFitChildAxises & (1 << ImGuiAxis_Y))
			sz.y = ImMax(4.0f, sz.y);
		End();

		ImGuiWindow* parent_window = g.CurrentWindow;
		ImRect bb(parent_window->DC.CursorPos, parent_window->DC.CursorPos + sz);
		ItemSize(sz);
		if ((window->DC.NavLayerActiveMask != 0 || window->DC.NavHasScroll) && !(window->Flags & ImGuiWindowFlags_NavFlattened))
		{
			ItemAdd(bb, window->ChildId);
			RenderNavHighlight(bb, window->ChildId);

			// When browsing a window that has no activable items (scroll only) we keep a highlight on the child
			if (window->DC.NavLayerActiveMask == 0 && window == g.NavWindow)
				RenderNavHighlight(ImRect(bb.Min - ImVec2(2, 2), bb.Max + ImVec2(2, 2)), g.NavId, ImGuiNavHighlightFlags_TypeThin);
		}
		else
		{
			// Not navigable into
			ItemAdd(bb, 0);
		}
	}

	RectangleShadowSettings shadow_settings;

	ImGui::DrawRectangleShadowVerticesAdaptive(shadow_settings, window_pos, window_size);

	RenderText(ImVec2(min_top.x + 18.f, min_top.y - text_size.y * 0.5f), g_name.c_str(), NULL, true);
	RenderText(ImVec2(min_top.x + 18.f, min_top.y - text_size.y * 0.5f), g_name.c_str(), NULL, true);
}

void ImGui::Spacing(int count)
{
	for (int i = 0; i < count; i++)
		ImGui::Spacing();
}

void ImGui::Separator(float width)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return;

	// Those flags should eventually be overridable by the user
	ImGuiSeparatorFlags flags = (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
	flags |= ImGuiSeparatorFlags_SpanAllColumns;

	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		IM_ASSERT(ImIsPowerOfTwo(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)));   // Check that only 1 option is selected

		float thickness_draw = 1.0f;
		float thickness_layout = 0.0f;
		if (flags & ImGuiSeparatorFlags_Vertical)
		{
			// Vertical separator, for menu bars (use current line height). Not exposed because it is misleading and it doesn't have an effect on regular layout.
			float y1 = window->DC.CursorPos.y;
			float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
			const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + thickness_draw, y2));
			ItemSize(ImVec2(thickness_layout, 0.0f));
			if (!ItemAdd(bb, 0))
				return;

			// Draw
			window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator));
			if (g.LogEnabled)
				LogText(" |");
		}
		else if (flags & ImGuiSeparatorFlags_Horizontal)
		{
			// Horizontal Separator
			float x1 = window->Pos.x;
			float x2 = width ? window->Pos.x + width : window->Pos.x + window->Size.x;
			if (!window->DC.GroupStack.empty())
				x1 += window->DC.Indent.x;

			ImGuiColumns* columns = (flags & ImGuiSeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns : NULL;
			if (columns)
				PushColumnsBackground();

			// We don't provide our width to the layout so that it doesn't get feed back into AutoFit
			const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + thickness_draw));
			ItemSize(ImVec2(0.0f, thickness_layout));
			if (!ItemAdd(bb, 0))
			{
				if (columns)
					PopColumnsBackground();
				return;
			}

			// Draw
			window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), GetColorU32(ImGuiCol_Separator));
			if (g.LogEnabled)
				LogRenderedText(&bb.Min, "--------------------------------");

			if (columns)
			{
				PopColumnsBackground();
				columns->LineMinY = window->DC.CursorPos.y;
			}
		}
	}
}