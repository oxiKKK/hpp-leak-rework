class CWindow;

namespace ImGui
{
	void Spacing(int count);
	bool ListBoxArray(const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items);
}

enum AnimStates 
{
	AnimState_None = 1 << 0,
	AnimState_Open = 1 << 1,
	AnimState_Close = 1 << 2
};

struct WindowInfo
{
	char*	m_szName;
	int		m_nIndex;
	void(*m_pFormFunc)();
};

class CWindow 
{
private:
	std::deque<WindowInfo> _WindowInfo;
	int		_nCurrentWindow;
	int		_nNextIndex;
	int		_nAnimationState;
	int		_nSwapWindowIndex;
	DWORD	_dwLastTime;

public:
	CWindow() noexcept;

	std::deque<WindowInfo>& GetWindow() noexcept;
	int GetCurrentWindow() noexcept;

	WindowInfo PushWindow(const char* a_szName, void(*a_pFunc)());
	WindowInfo RenderWindow();

	void SwapWindow(const char* a_szName);
	void Clear() noexcept;
};

extern std::unique_ptr<CWindow> g_pWindow;

float fInterp(float s1, float s2, float s3, float f1, float f3) noexcept;