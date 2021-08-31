enum OutFovTypesList
{
	ARROWS,
	CIRCLES,
	RHOMBUS
};

class CDrawPlayersOutFOV : private CSelectionPlayers
{
public:
	void Draw();

private:
	ImColor GetTeamColor(int index);

	bool CalcScreen(int index);

	void DrawOutFOV(int index);
	void DrawName(int index, const Vector2D& point);
	void DrawWeapon(int index, const Vector2D& point);
	void DrawDistance(int index, const Vector2D& point);
};