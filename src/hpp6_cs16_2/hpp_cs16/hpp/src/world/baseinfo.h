enum EClassEntity
{
	EClassEntity_Unknown,
	EClassEntity_BaseLocal,
	EClassEntity_BasePlayer,
	EClassEntity_BaseEntity
};

class CBaseEntInfo
{
public:
	EClassEntity m_ClassId;

	bool	m_bIsConnected;
	bool	m_bIsLocal;
	bool	m_bIsPlayer;
	bool	m_bIsInPVS;
	bool	m_bIsDead;
	bool	m_bIsDucked;
	bool	m_bIsOnGround;
	bool	m_bIsOnLadder;
	bool	m_bIsInWater;
	bool	m_bIsScoped;
	bool	m_bHasC4;
	bool	m_bHasDefusalKits;
	bool	m_bHasShield;
	bool	m_bSoundUpdated;
	bool	m_bReplaceModel;

	int		m_iEntIndex;
	int		m_iMessageNum;
	int		m_iTeamNum;
	int		m_iFOV;
	int		m_iRealFOV;
	int		m_iHealth;
	int		m_iArmor;
	int		m_iArmorType;
	int		m_iMoney;
	int		m_iObserverState;
	int		m_iObserverIndex;
	int		m_iSequence;
	int		m_iSequenceFrame;

	float	m_flVelocity;
	float	m_flFallVelocity;
	float	m_flGroundAngle;
	float	m_flHeightGround;
	float	m_flHeightInDuck;
	float	m_flHeightPlane;
	float	m_flHeight;
	float	m_flEdgeDistance;
	float	m_flFrameTime;
	float	m_flDistance;
	float	m_flForwardMove;
	float	m_flSideMove;
	float	m_flHistory;
	float	m_flLastTimeInPVS;
	float	m_flLastKillTime;
	float	m_flUpdateIntervalTime;
	float	m_flGaitYaw;

	char	m_szPrintName[MAX_PLAYER_NAME_LENGTH];
	char	m_szModelName[MAX_MODEL_NAME];
	char	m_szWeaponModelName[MAX_MODEL_NAME];

	QAngle	m_QAngles;

	Vector	m_vecPrevOrigin;
	Vector	m_vecOrigin;
	Vector	m_vecEyePos;
	Vector	m_vecVelocity;
	Vector	m_vecBoundBoxMaxs;
	Vector	m_vecBoundBoxMins;
	Vector	m_vecHitbox[HITBOX_MAX];
	Vector	m_vecHitboxMin[HITBOX_MAX];
	Vector	m_vecHitboxMax[HITBOX_MAX];
	Vector	m_vecHitboxPoints[HITBOX_MAX][HITBOX_POINTS_MAX];
	Vector	m_vecOBBMin[HITBOX_MAX];
	Vector	m_vecOBBMax[HITBOX_MAX];

	matrix3x4_t	m_matHitbox[HITBOX_MAX];
};

class CBaseWpnInfo
{
public:
	bool	m_bDelayFire;

	int		m_iFlags;
	int		m_iWeaponID;
	int		m_iClip;
	int		m_iWeaponState;
	int		m_iUser1;
	int		m_iUser2;
	int		m_iUser3;
	int		m_iUser4;
	int		m_iPenetration;
	int		m_iDamage1;
	int		m_iDamage2;
	int		m_iBulletType;
	int		m_iShotsFired;
	int		m_fInReload;
	int		m_iAccuracyFlags;

	uint	m_iRandomSeed;

	float	m_flNextPrimaryAttack;
	float	m_flNextSecondaryAttack;
	float	m_flNextAttack;
	float	m_flWallPierce1;
	float	m_flWallPierce2;
	float	m_flDistance;
	float	m_flSpread;
	float	m_flAccuracy;
	float	m_flStartThrow;
	float	m_flReleaseThrow;
	float	m_flDecreaseShotsFired;

	double	m_dbTime;
};

class CBaseHitInfo
{
public:
	bool	m_bHeadshot;

	int		m_iIndex;
	int		m_iDamage;
	int		m_iWeaponID;
	int		m_iHitboxNum;
	int		m_iHitgroup;

	int		m_iHits;
	int		m_iMisses;

	float	m_flTimeStamp;
};