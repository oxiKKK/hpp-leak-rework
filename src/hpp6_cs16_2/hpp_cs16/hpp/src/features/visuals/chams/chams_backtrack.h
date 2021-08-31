class CChamsBacktrack : private CRenderModels
{
public:
	bool Draw(cl_entity_s* pGameEntity);

	void CreateBacktrackPlayers();

private:
	bool IsPlayer(int index);

	void DrawBacktrack(int index);
};