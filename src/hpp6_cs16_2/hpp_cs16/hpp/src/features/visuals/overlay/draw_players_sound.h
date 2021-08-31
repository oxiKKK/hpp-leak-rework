typedef struct
{
	Vector origin;
	double time;
	int index;
} player_sound_t;

class CDrawPlayersSound
{
public:
	void Draw();

private:
	ImColor GetSoundColor(int index);

	void DrawSound(const player_sound_t& sound);
};

extern std::vector<player_sound_t> g_PlayerSounds;