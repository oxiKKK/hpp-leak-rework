class CWindow
{
private:
	static int _iGlobalIndex;

	bool	_bIsVisible;
	char*	_pszName;
	int		_iIndex;
	void	(*_Fn)();

public:
	CWindow();

	void	Create(const char* pcszName, void(*fn)());

	bool	IsVisible();
	char*	GetName();
	int		GetIndex();

	void	Show();
	void	Hide();
	void	Toggle();

	void	Run();
};

class CDrawGUI
{
private:
	std::deque<CWindow> _window;
	bool	_bIsDrawing;
	bool	_bIsFadeout;

public:
	CDrawGUI();

	void	Create(const char* pcszName, void(*fn)());

	std::deque<CWindow>&	GetWindow();
	CWindow&	GetWindowByName(const char* pcszName);
	CWindow&	GetWindowByIndex(int iIndex);

	bool	IsDrawing();
	bool	IsFadeout();

	void	Draw();
	void	Hide();
	void	Toggle();

	void	FadeRunning();
	void	FadeStopping();

	void	Run();
};

extern CDrawGUI g_DrawGUI;