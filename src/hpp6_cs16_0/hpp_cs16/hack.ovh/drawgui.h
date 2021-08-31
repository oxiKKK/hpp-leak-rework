namespace ImGui
{	
	extern bool	bKeyAssign;
}

//	render functions
void	CheatMenu();

class CWindow 
{
private:
	bool	_bIsOpened;
	void	(*_pFunction)();

public:
	CWindow() noexcept;

	void	PushWindow(void(*a_pFunction)()) noexcept;
	bool	IsOpened() noexcept;

	void	Show() noexcept;
	void	Hide() noexcept;
	void	Toggle() noexcept;

	void	Draw() noexcept;
};

class CDrawGUI 
{
private:
	std::deque<CWindow> _Window;
	bool	_bIsFadeout;
	bool	_bIsDrawing;

public:
	CDrawGUI() noexcept;

	void	Frame();
	void	Fadeout();

	bool	IsDrawing() noexcept;

	void	Draw() noexcept;
	void	Hide() noexcept;
	void	Toggle() noexcept;
	void	RunFadeout() noexcept;

	void	Render();
	
	std::deque<CWindow>& GetWindow() noexcept;
};

extern CDrawGUI g_DrawGUI;