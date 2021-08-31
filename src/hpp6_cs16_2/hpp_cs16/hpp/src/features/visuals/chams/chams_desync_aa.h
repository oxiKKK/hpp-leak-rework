struct DesyncEntityData
{
	cl_entity_s entity;
	player_info_s playerinfo;
};

class CChamsDesyncAA : private CRenderModels
{
public:
	bool Draw(cl_entity_s* pGameEntity);

	void CreateDesyncAA();

private:
	DesyncEntityData m_DesyncData;

	bool IsPlayer();

	void DrawDesyncAA();
};