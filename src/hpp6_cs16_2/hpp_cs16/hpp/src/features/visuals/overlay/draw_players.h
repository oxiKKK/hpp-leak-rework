enum BoxTypesList
{
	BoxDefault = 1,
	BoxCorner,
	BoxRound
};

enum PositionsList
{
	PosLeft = 1,
	PosRight,
	PosTop,
	PosBot
};

enum DistanceMeasurements
{
	InUnits,
	InMeters
};

enum DrawHitboxesTypesList
{
	DrawOnlyHead = 1,
	DrawAllHitboxes
};

struct HitboxesHitPositionData
{
	Vector hitbox_points[HITBOX_MAX][HITBOX_POINTS_MAX];
	int hitboxnum;
	float timestamp;
};

class CDrawPlayers : private CSelectionPlayers
{
public:
	CDrawPlayers();

	void Overlay();
	void LineSight();

private:
	std::unique_ptr<CDrawPlayersOutFOV> m_pOutFOV;
	std::unique_ptr<CDrawPlayersSound> m_pSound;

	ImRect m_rect;

	ImRect GetRect(const Vector2D& ScreenTop, const Vector2D& ScreenBot);
	ImVec2 GetPosition(int index, int& pos, std::string text, ImVec2 text_size = ImVec2());

	ImColor GetTeamColor(int index);

	bool CalcScreen(int index);
	void BoundBox(int index);
	void DrawBar(int index, int pos, float value, const ImVec4& color, float spacing, bool percentage = false, ImColor color_text = ImColor(IM_COL32_WHITE));
	void HealthBar(int index);
	void ArmorBar(int index);
	void Name(int index);
	void WeaponText(int index);
	void WeaponIcon(int index);
	void Money(int index);
	void Distance(int index);
	void Actions(int index);
	void HasC4(int index);
	void HasDefusalKits(int index);
	void Skeleton(int index);
	void SkeletonBacktrack(int index);
	void Hitboxes(int index);
	void HitboxesHitPosition();
};