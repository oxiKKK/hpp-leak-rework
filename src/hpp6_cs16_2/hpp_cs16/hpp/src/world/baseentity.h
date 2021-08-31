class CBaseEntity
{
public:
	void AllocateMemory();
	void FreeMemory();

	void Update();

	CBaseEntInfo* operator[](int index);

private:
	std::unique_ptr<CBaseEntInfo> pBaseInfo[MAX_EDICTS];

	void UpdateEntity(cl_entity_s* pGameEntity);
};

extern CBaseEntity g_Entity;