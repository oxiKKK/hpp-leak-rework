class CChamsPlayers : private CRenderModels
{
public:
	bool Draw(cl_entity_s* pGameEntity);

	void CreatePlayers();
	void CreateDormantPlayers();

private:
	bool IsPlayer(int index);

	void DrawPlayer(int index);
};