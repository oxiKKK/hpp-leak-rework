#define MAX_ENTITIES			1365
#define HITBOX_POINTS_MAX		8

#define HULL_PLAYER_REGULAR		0
#define HULL_PLAYER_DUCKED		1
#define HULL_POINT				2

class CBaseLocalPlayer
{
public:
	int			m_iIndex;
	int			m_iTeam;
	int			m_iHealth;
	int			m_iArmor;
	int			m_iFOV;
	int			m_iMessageNum;
	int			m_iLastIncomingSequence;

	bool		m_bIsDead;
	bool		m_bIsDucked;
	bool		m_bIsScoped;
	bool		m_bIsOnGround;
	bool		m_bIsOnLadder;

	float		m_flVelocity;
	float		m_flGroundAngle;
	float		m_flHeight;
	float		m_flHeightGround;
	float		m_flHeightDucked;
	float		m_flHeightPlane;
	float		m_flEdgeDist;

	QAngle		m_QAngles;

	Vector		m_vecEye;
	Vector		m_vecForward;
	Vector		m_vecUp;
	Vector		m_vecRight;
	Vector		m_vecBoundBoxMins;
	Vector		m_vecBoundBoxMaxs;

	char		m_szModelName[MAX_MODEL_NAME];
};

class CBasePlayer
{
public:
	int			m_iIndex;
	int			m_iTeam;
	int			m_iHealth;
	int			m_iArmorType;
	int			m_iPlayerListType;
	int			m_iScoreAttribFlags;

	bool		m_bIsConnected;
	bool		m_bIsInPVS;
	bool		m_bIsDead;
	bool		m_bIsDucked;
	bool		m_bSoundUpdated;
	bool		m_bHasC4;

	float		m_flVelocity;
	float		m_flHeight;
	float		m_flGroundAngle;	
	float		m_flFrametime;
	float		m_flDistance;
	float		m_flForwardMove;
	float		m_flSideMove;
	float		m_flHistory;
	float		m_flLastTimeInPVS;

	Vector		m_vecAngles;
	Vector		m_vecVelocity;
	Vector		m_vecOrigin;
	Vector		m_vecPreviousOrigin;
	Vector		m_vecEye;
	Vector		m_vecHitbox[HITBOX_MAX];
	Vector		m_vecHitboxMin[HITBOX_MAX];
	Vector		m_vecHitboxMax[HITBOX_MAX];
	Vector		m_vecHitboxPoints[HITBOX_MAX][HITBOX_POINTS_MAX];
	Vector		m_vecBoundBoxMins;
	Vector		m_vecBoundBoxMaxs;

	char		m_szPrintName[MAX_PLAYER_NAME_LENGTH];
	char		m_szModelName[MAX_MODEL_NAME];
	char		m_szWeaponModelName[MAX_MODEL_NAME];
	char		m_szWeaponSoundName[MAX_MODEL_NAME];
};

class CBaseEntity 
{
public:
	int			m_iIndex;

	bool		m_bIsInPVS;
	bool		m_bIsPlayer;
	bool		m_bIsSmoke;

	Vector		m_vecOrigin;
	Vector		m_vecDetonatedOrigin;
	Vector		m_vecAngles;
	Vector		m_vecVelocity;
	Vector		m_vecBoundBoxMins;
	Vector		m_vecBoundBoxMaxs;

	char		m_szModelName[MAX_MODEL_NAME];
};

typedef struct 
{
	int			m_iOriginalIndex;
	int			m_iResolvedIndex;
} sound_resolver_t;

class CWorld
{
public:
	int			m_iBombFlag;
	double		m_dbBombPlantedTime;
	Vector		m_vecBombOrigin;

	CBasePlayer* GetPlayer(const int index = 0);
	CBaseEntity* GetEntity(const int index = 0);

	void		Update();	
	void		Reset();
	void		Clear();

private:
	CBasePlayer m_Player[MAX_CLIENTS - 1];
	CBaseEntity m_Entity[MAX_ENTITIES];

	void		UpdateLocalPlayer();
	void		UpdatePlayers();	
	void		UpdateEntities();
};

extern std::deque<sound_resolver_t> g_SoundResolver;
extern CBaseLocalPlayer g_Local;
extern CWorld g_World;