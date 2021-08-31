class CBomb
{
public:
	CBomb() { RtlSecureZeroMemory(this, sizeof(*this)); }

	Vector		m_vOrigin;
	int			m_iFlag;
	float		m_flTime;	//unknown
};

class CBaseLocalPlayer
{
public:
	CBaseLocalPlayer() { RtlSecureZeroMemory(this, sizeof(*this)); }

	int			m_iIndex;
	int			m_iTeam;
	int			m_iHealth;
	int			m_iArmor;
	int			m_iFOV;
	int			m_iOutPing;
	int			m_iMessageNum;

	bool		m_bIsDead;
	bool		m_bIsScoped;

	float		m_flFrametime;
	float		m_flVelocity;
	float		m_flHeightGround;
	float		m_flGroundAngle;
	float		m_flHeight;
	float		m_flHeightPlane;
	float		m_flLowerBodyYaw;

	Vector		m_vecAngles;
	Vector		m_vecOrigin;
	Vector		m_vecEye;
	Vector		m_vecForward;
	Vector		m_vecUp;
	Vector		m_vecRight;
	Vector		m_vecBoundBoxMins;
	Vector		m_vecBoundBoxMaxs;
};

extern CBaseLocalPlayer g_Local;

class CBasePlayer 
{
public:
	CBasePlayer() { RtlSecureZeroMemory(this, sizeof(*this)); }

	int			m_iIndex;
	int			m_iTeam;
	int			m_iHealth;
	int			m_iArmorType;

	bool		m_bIsInPVS;
	bool		m_bIsDead;
	bool		m_bIsDucked;
	bool		m_IsTransparent;

	float		m_flVelocity;
	float		m_flHeightGround;
	float		m_flGroundAngle;
	float		m_flHeight;
	float		m_flHeightPlane;
	float		m_flFrametime;
	float		m_flDistance;
	float		m_flLowerBodyYaw;
	float		m_flForwardMove;
	float		m_flSideMove;

	Vector		m_vecAngles;
	Vector		m_vecVelocity;
	Vector		m_vecOrigin;
	Vector		m_vecPreviousOrigin;
	Vector		m_vecHitbox[21];
	Vector		m_vecHitboxMin[21];
	Vector		m_vecHitboxMax[21];
	Vector		m_vecHitboxPoints[21][8];
	Vector		m_vecBoundBoxMins;
	Vector		m_vecBoundBoxMaxs;
	Vector		m_vecEye;

	DWORD		m_dwHistory;

	std::string	m_sPrintName;
	std::string	m_sPlayerModel;
};

extern std::array<CBasePlayer, MAX_CLIENTS> g_Player;

constexpr auto MAX_ENTITIES = 512;

class CBaseEntity
{
public:
	CBaseEntity() { RtlSecureZeroMemory(this, sizeof(*this)); }

	int			m_iIndex;

	bool		m_bIsInPVS;
	bool		m_bIsPlayer;

	Vector		m_vecOrigin;
	Vector		m_vecAngles;
	Vector		m_vecVelocity;
	Vector		m_vecBoundBoxMins;
	Vector		m_vecBoundBoxMaxs;

	std::string	m_sPrintName;
};

extern std::array<CBaseEntity, MAX_ENTITIES> g_Entity;

class CPlayerInfo
{
private:
	bool		_IsLocalAlive(const CBaseLocalPlayer* pPlayer, const usercmd_s* cmd, const cl_entity_s* pGameEntity);
	bool		_IsPlayerDead(const cl_entity_s* pGameEntity);

protected:
	void		UpdateLocalPlayer(CBaseLocalPlayer* pPlayer, float frametime, const usercmd_s* cmd);
	void		UpdatePlayers(std::array<CBasePlayer, MAX_CLIENTS>& Player);

	void		ClearLocalPlayer(CBaseLocalPlayer* pPlayer);
	void		ClearPlayers(std::array<CBasePlayer, MAX_CLIENTS>& Player);
};

class CEntityInfo
{
protected:
	void		UpdateEntities(std::array<CBaseEntity, MAX_ENTITIES>& Entity);

	void		ClearEntities(std::array<CBaseEntity, MAX_ENTITIES>& Entity);
};

class CWorld : public CPlayerInfo, public CEntityInfo
{
public:
	CBomb		m_Bomb;

	void		Update(float frametime, const usercmd_s* cmd);
	void		Clear() noexcept;
	//void		UpdateMapInfo();
};

extern CWorld g_World;

class CInfoMapParameters 
{
public:
	CInfoMapParameters() { RtlSecureZeroMemory(this, sizeof(*this)); }

	float		m_flBombRadius;

	std::string	m_szLevelName;
};

extern CInfoMapParameters g_MapInfo;

extern std::deque<my_sound_t> g_Sounds;

class CSoundResolver 
{
public:
	CSoundResolver() { RtlSecureZeroMemory(this, sizeof(*this)); }

	int			m_iOriginalIndex;
	int			m_iResolvedIndex;
};

extern std::deque<CSoundResolver> g_SoundResolver;