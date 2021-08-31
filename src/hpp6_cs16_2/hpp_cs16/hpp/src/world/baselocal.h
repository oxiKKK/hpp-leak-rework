class CBaseLocal
{
public:
	void AllocateMemory();
	void FreeMemory();

	void Update(float frametime, usercmd_s* cmd);

	CBaseEntInfo* GetBaseInfo();
	CBaseEntInfo* operator->();

private:
	std::unique_ptr<CBaseEntInfo> pBaseInfo;

	bool IsDead(cl_entity_s* pGameEntity);

	float GetEdgeDistance();
};

extern CBaseLocal g_Local;