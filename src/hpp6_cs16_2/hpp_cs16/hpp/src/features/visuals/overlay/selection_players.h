struct SelectionData
{
	float dist;
	int index;
};

class CSelectionPlayers
{
public:
	std::vector<SelectionData>& GetSelectionPlayers(std::function<bool(int)> FilterPlayerFn);

	void InterpolateOrigin(int index, Vector& origin);
	void GetBBMaxsMins(int index, Vector& mins, Vector& maxs);
	void FadeoutDormant(int index, ImColor& color, float history_time);

private:
	void PushPlayer(std::vector<SelectionData>& data, int index);
	void SortSelectionPlayers(std::vector<SelectionData>& data);
};