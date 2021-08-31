#include "Main.h"

void ImGui::Spacing(int count)
{
	for (int i = 0; i < count; ++i)
		Spacing();
}

static auto vector_getter = [](void* vec, int idx, const char** out_text)
{
	if (out_text == nullptr)
		return false;

	auto& vector = *static_cast<std::vector<std::string>*>(vec);

	if (idx < 0 || idx >= gsl::narrow_cast<int>(vector.size()))
		return false;

	*out_text = vector.at(idx).c_str();

	return true;
};

bool ImGui::ListBoxArray(const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items)
{
	return ListBox(label, currIndex, vector_getter, static_cast<void*>(&values), values.size(), height_in_items);
}

float fInterp(float s1, float s2, float s3, float f1, float f3) noexcept
{
	if (s2 == s1) return f1;
	if (s2 == s3) return f3;
	if (s3 == s1) return f1;

	return f1 + ((s2 - s1) / (s3 - s1)) * (f3 - f1);
}

CWindow::CWindow() noexcept :
	_nCurrentWindow (NULL), 
	_nNextIndex(NULL),
	_nAnimationState(NULL),
	_nSwapWindowIndex(NULL),
	_dwLastTime(NULL)
{
	_WindowInfo.clear();
}

std::deque<WindowInfo>& CWindow::GetWindow() noexcept
{
	return _WindowInfo;
}

int CWindow::GetCurrentWindow() noexcept
{
	return _nCurrentWindow;
}

WindowInfo CWindow::PushWindow(const char* a_szName, void(*a_pFunc)())
{
	WindowInfo windowInfo;

	windowInfo.m_szName = new char[strlen(a_szName) + 1];
	strcpy(windowInfo.m_szName, a_szName);

	windowInfo.m_nIndex = _nNextIndex;
	windowInfo.m_pFormFunc = a_pFunc;

	_WindowInfo.push_back(windowInfo);
	++_nNextIndex;

	return windowInfo;
}

WindowInfo CWindow::RenderWindow()
{
	const DWORD dwAnimateTime = 100;
	ImGuiStyle& style = ImGui::GetStyle();

	if (_nAnimationState & AnimState_Open)
	{	
		style.Alpha = fInterp(static_cast<float>(_dwLastTime), static_cast<float>(GetTickCount()), 
			static_cast<float>(_dwLastTime + dwAnimateTime), 0.f, 1.f);
		style.Alpha = ImMax(style.Alpha, 0.f);
		style.Alpha = ImMin(style.Alpha, 1.f);

		if (GetTickCount() >= _dwLastTime + dwAnimateTime)
		{
			_nAnimationState = AnimState_None;
			_nSwapWindowIndex = -1;
		}
	}
	else if (_nAnimationState & AnimState_Close)
	{
		style.Alpha = fInterp(static_cast<float>(_dwLastTime), static_cast<float>(GetTickCount()),
			static_cast<float>(_dwLastTime + dwAnimateTime), 1.f, 0.f);
		style.Alpha = ImMax(style.Alpha, 0.f);
		style.Alpha = ImMin(style.Alpha, 1.f);

		if (GetTickCount() >= _dwLastTime + dwAnimateTime)
		{
			_nAnimationState = AnimState_Open;
			_nCurrentWindow = _nSwapWindowIndex;
			_dwLastTime = GetTickCount();
		}
	}

	auto windowInfo = _WindowInfo[_nCurrentWindow];
	const auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

	ImGui::Begin(windowInfo.m_szName, static_cast<bool*>(nullptr), flags);

	if (ImGui::CloseButton(ImGui::GetID(windowInfo.m_szName), ImVec2(WinWidth - 14, 14), 9))
		Shutdown();

	ImGui::Spacing(2);
	ImGui::SameLine(6);

	ImGui::BeginGroup();
	ImGui::Image(g_pD3DTexture_TitleLogo, ImVec2(17, 20));
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Spacing();
	ImGui::PushFont(Lucida11px);
	ImGui::TextColored(ImColor(40, 40, 40), "Hpp Loader");
	ImGui::PopFont();
	ImGui::EndGroup();

	ImGui::BeginChild("##Child");
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.00f, 0.00f, 0.00f, 0.08f));
	ImGui::Separator();
	ImGui::PopStyleColor();

	windowInfo.m_pFormFunc();

	ImGui::EndChild();
	ImGui::End();

	return windowInfo;
}

void CWindow::SwapWindow(const char* a_szName)
{
	for (size_t i = 0; i < _WindowInfo.size(); ++i)
	{
		if (strcmp(_WindowInfo[i].m_szName, a_szName))
			continue;

		_nSwapWindowIndex = _WindowInfo[i].m_nIndex;
		break;
	}

	if (_nAnimationState & AnimState_None)
		_dwLastTime = GetTickCount();

	if (_nSwapWindowIndex != _nCurrentWindow)
		_nAnimationState = AnimState_Close;
}

void CWindow::Clear() noexcept
{
	_WindowInfo.clear();
	_nCurrentWindow = NULL;
	_nNextIndex = NULL;
	_nAnimationState = NULL;
	_nSwapWindowIndex = NULL;
	_dwLastTime = NULL;
}

std::unique_ptr<CWindow> g_pWindow = std::make_unique<CWindow>();