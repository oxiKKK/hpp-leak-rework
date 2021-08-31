#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
// Only keep 1 second of data
#define LAG_COMPENSATION_DATA_TIME	1.0f
 // for smoke grenades
#define smokeRadius 115.0f

struct CIncomingSequence
{
	int incoming_sequence;
	double time;
};

class CUtils;
class CUtils
{
private:
	DWORD			_dwOldPageProtection;
	Vector			_vecPreviousAngles;

public:
	bool			EnablePageWrite(const DWORD dwAddress, const SIZE_T dwSize);
	bool			RestorePageProtection(const DWORD dwAddress, const SIZE_T dwSize);
	bool			FileExists(const char* _FileName);
	bool			CalcScreen(Vector a_vecOrigin, float* a_pflVecScreen);
	bool			IsBoxIntersectingRay(const Vector& vecBoundBoxMin, const Vector& vecBoundingBoxMax, const Vector& vecOrigin, const Vector& vecDelta);
	bool			IsLineBlockedBySmoke(const Vector &from, const Vector &to);

	int				LookupSequence(model_s* a_pModel, const char** a_pcszLabel, const int a_nSize);
	int				SimulateFireBullet(Vector vecStart, Vector vecEnd, float fDistance, int iOrigPenetration, int iBulletType, int iDamage, float fRangeModifier);
	int				ExceptionFilter(const char* func, const int code, const _EXCEPTION_POINTERS* ep);

	void			LagCompensation(int index, short lerp_msec, Vector &origin);
	void			UpdateSequences();
	void			SetFakeLatency(const double latency);
	void			SendCommand(const bool bStatus);
	void			AdjustSpeed(const double speed);
	void			AngleMatrix(const float *angles, float(*matrix)[4]);
	void			SpoofPhysent(const int a_iIndex, const bool a_bDisableTraces = false);
	void			VectorTransform(Vector in1, float in2[3][4], float* out, int xyz, float multi);
	void			VectorTransform(Vector in1, float in2[3][4], float* out);
	void			NormalizeAngles(float* angles);
	void			FixMoveStart(usercmd_s* cmd);
	void			FixMoveEnd(usercmd_s* cmd);
	void			VectorAngles(const float* forward, float* angles);
	void			MakeAngle(bool bAddAngles, float* fAngles, usercmd_s* cmd);
	void			memwrite(uintptr_t adr, uintptr_t ptr, SIZE_T size);
	void			GetHitboxes(const cl_entity_s* a_pGameEntity);
	void			StringReplace(char* buf, const char* search, const char* replace);
	void			UTIL_TextureHit(Vector vecSrc, Vector vecEnd, Vector vecDir, int ignore, pmtrace_t& tr);
	void			CreateRandomString(char* pszDest, int nLength);
	void			ConvertToASCII(int* keynum);
	void			RotateInvisible(const float fixed_yaw, const float fixed_pitch, usercmd_s* cmd);
	void			ReplaceCall(uintptr_t callPtr, uintptr_t destPtr);

	float			fInterp(const float s1, const float s2, const float s3, const float f1, const float f3);
	float			FinalFallVelocity(const int usehull);

	const char*		GetHitboxNameByIndex(const int hitbox);
	const char*		GetWeaponNameByIndex(int nWeaponIndex);
	const char*		KeyToString(int keynum);

	std::vector<std::string> explode(std::string const& s, char delim);
};

extern CUtils g_Utils;

void TraceLog(const char *fmt, ...);

class CSkipFrames
{
public:
	CSkipFrames();
	~CSkipFrames() {}

	bool	IsActive() { return m_norefresh_active; }

	void	Start(const int value);
	void	End();

private:
	enum cvar
	{
		hud_draw,
		r_drawentities,
		net_graph,
		cl_showfps,
		r_norefresh,
		MAX_COUNT
	};

	bool	m_norefresh_active;
	int		m_frames;

	cvar_s*	m_cvar[MAX_COUNT];
	float	m_saved[MAX_COUNT];

	bool	m_init;

	void	InitCvars();

	void	NoRefreshEnable();
	void	NoRefreshDisable();
};