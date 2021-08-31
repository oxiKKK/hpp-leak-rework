enum DlightOriginList
{
	DLIGHT_LEGS,
	DLIGHT_BODY,
	DLIGHT_HEAD
};

class CDlight
{
public:
	void Draw();

private:
	bool IsPlayer(int index);

	void DrawLight(cl_entity_s* pGameEntity);
};