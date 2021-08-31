struct HitPositionData
{
	cl_entity_s entity;
	player_info_s playerinfo;
	float timestamp;
};

class CChamsHitPosition : private CRenderModels
{
public:
	bool Draw(cl_entity_s* pGameEntity);

	void CreateHitPositionPlayers();

private:
	std::vector<HitPositionData> m_HitPositionData[MAX_CLIENTS];

	bool IsPlayer(int index);

	void DrawHitPosition(int index);
};