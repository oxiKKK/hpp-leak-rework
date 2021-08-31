class CUtils;
class CUtils
{
private:
	DWORD	_dwOldPageProtection;
	Vector	_vecPreviousAngles;

	physent_t NewPhysent(int a_iIndex, int a_nPhysent, bool a_bDisableTraces = false);

public:
	bool	IsBoxIntersectingRay(const Vector& vecBoundBoxMin, const Vector& vecBoundingBoxMax, const Vector& vecOrigin, const Vector& vecDelta);
	bool	IsTeleported(const Vector& v1, const Vector& v2);

	int		LookupSequence(model_s* a_Model, const char** a_pcszLabel, int a_nSize);
	void	UTIL_TextureHit(Vector vecSrc, Vector vecEnd, Vector vecDir, int ignore, pmtrace_t& tr);
	void	SendCommand(bool bStatus);
	void	ComputeMove(CBasePlayer* pPlayer);
	void	SpoofPhysent(int iIndex, bool bDisableTraces = false);
	int		DevilPlayerTrace(Vector vecStart, Vector vecEnd);

	void	VectorTransform(Vector in1, float in2[3][4], float* out, int xyz, float multi);
	void	VectorTransform(Vector in1, float in2[3][4], float* out);

	void	FixMoveStart(usercmd_s* cmd);
	void	FixMoveEnd(usercmd_s* cmd);

	void	VectorAngles(const float* forward, float* angles);
	void	MakeAngle(bool bAddAngles, float* fAngles, usercmd_s* cmd);
	void	NormalizeAngles(float* angles);

	void	GetHitboxes(cl_entity_s* ent);

	float	Armor(float fDamage, int nArmorValue);
	int		FireBullets(Vector vecStart, Vector vecEnd, float fDistance, int iOrigPenetration, int iBulletType, int iDamage, float fRangeModifier);

	bool	EnablePageWrite(DWORD dwAddress, SIZE_T dwSize);
	bool	RestorePageProtection(DWORD dwAddress, SIZE_T dwSize);
	void	memwrite(uintptr_t adr, uintptr_t ptr, SIZE_T size);

	bool	CalcScreen(Vector a_vecOrigin, float* a_pflVecScreen);
	void	StringReplace(char* buf, const char* search, const char* replace);
	float	fInterp(float s1, float s2, float s3, float f1, float f3);

	bool	FileExists(const char* _FileName);

	const char* KeyToString(int keynum);
	void	ConvertToASCII(int* keynum);
};

extern CUtils g_Utils;

qboolean BoundsIntersect(const Vector &mins1, const Vector &maxs1, const Vector &mins2, const Vector &maxs2);

void TraceLog(const char *fmt, ...);