enum ChamsRenderType
{
	RenderType_Flat = 1,
	RenderType_Wireframe,
	RenderType_Material,
	RenderType_Texture
};

class CESP
{
public:
	void	DrawPlayers();
	void	DrawEntities();
	void	DrawSounds();	
};

class CMisc
{
public:
	void	DrawStatuses();
	void	DrawMiscellaneous();
	void	DrawBomb();
};

struct screenlogs_s
{
	std::string message;
	ImColor color;
	float timestamp;
};

class CScreenLogs
{
private:
	std::deque<screenlogs_s> _logs;

public:
	auto&	GetLogs();
	void	Log(const ImColor& color, const char* message, ...);
	void	Show();
	void	Clear();
};

extern CScreenLogs g_ScreenLog;

class CVisuals
{
public:
	void	Run();
};