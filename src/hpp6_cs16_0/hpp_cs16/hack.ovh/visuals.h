constexpr auto MAX_HEALTH = 100;
constexpr auto MAX_ARMOR = 2;

class CPlayerESP;
class CWorldESP;
class CSoundESP;
class CMiscESP;
class CVisuals;

typedef struct
{
	float			x, y;
	float			w, h;
	ImColor			color;
} player_box_t;

class CPlayerESP
{
private:
	player_box_t	_box;
	CBasePlayer*	_pPlayer;
	FontFlag		_fontflag;
	ImColor			_fontcolor;
	float			_flBarThickness;
	float			_flBarSpacing;
	bool			_bIsFadeoutDormant;

	bool			GetScreenByBoundingBox(Vector& a_vecBoundingBoxMins, Vector& a_vecBoundingBoxMaxs,
						float* a_pflScreenBot, float* a_pflScreenTop);
	ImColor			GetColorTeam();

	void			BoundingBox();
	void			HealthBar();
	void			ArmorBar();
	void			Name();
	void			Weapon();
	void			Distance();
	void			Skeleton();
	void			DebugInfo();
	void			OffScreen();

	void			DrawPlayer(bool a_bIsFadeoutDormant = false);

public:
	CPlayerESP() noexcept;
	~CPlayerESP();

	void			Run();
};

class CWorldESP
{
private:
	CBaseEntity*	_pEntity;
	float*			_pflScreen;

	bool			IsGrenade(const std::string& a_sModelName) noexcept;

	void			DroppedWeapons();
	void			ThrownGrenades();
	void			GrenadeTrail(const cl_entity_s* a_pGameEntity);

	void			DrawWorldItems();

public:
	CWorldESP() noexcept;
	~CWorldESP();

	void			Run();
};

enum SoundTypes
{
	SoundType_None = 1 << 0,
	SoundType_Steps = 1 << 1,
	SoundType_Shooting = 1 << 2,
	SoundType_Buys = 1 << 3,
	SoundType_Hits = 1 << 4
};

class CSoundESP
{
private:
	char*			_szSoundName;
	int				_iSoundIndex;
	int				_iSoundType;

	ImColor			GetSoundColor();
	int				GetSoundType();

public:
	CSoundESP() noexcept;
	~CSoundESP();

	void			Run();
};

class CMiscESP
{
private:
	ImVec2			_vecPunchPosition;

	ImVec2			GetPunchPosition();

	void			Statuses();
	void			Circles();
	void			Points();

public:
	CMiscESP() noexcept;
	~CMiscESP();

	void			Run();
};

class CVisuals
{
private:
	CPlayerESP		_PlayerESP;
	CWorldESP		_WorldESP;
	CSoundESP		_SoundESP;
	CMiscESP		_MiscESP;

	void			DrawMisc();
	void			Bomb();

public:
	CVisuals() noexcept;
	~CVisuals();

	void			Run();

	void			ColoredModels();
	void			Brightness();
};

extern CVisuals g_Visuals;

typedef int FontFlag;

enum PlayerBoxTypes
{
	PlayerBoxType_Box = 1,
	PlayerBoxType_CornerBox,
	PlayerBoxType_RoundedBox,
};

enum Positions
{
	Pos_Left = 1,
	Pos_Rigth,
	Pos_Top,
	Pos_Bottom
};

enum ChamsRenderType
{
	RenderType_Flat = 1,
	RenderType_Wireframe,
	RenderType_Material,
	RenderType_Texture
};