#include "main.h"

static constexpr float BarThickness = 2.f;

static inline int GetFontFlags(int type)
{
	if (type == 1)
		return FontFlags_Shadow;
	else if (type == 2)
		return FontFlags_Outline;

	return FontFlags_None;
}

enum BoxTypes
{
	BoxType_Default = 1,
	BoxType_Corner,
	BoxType_Round,
};

enum Positions
{
	Pos_Left = 1,
	Pos_Rigth,
	Pos_Top,
	Pos_Bottom
};

struct box_s
{
	float x, y;
	float w, h;
	ImColor color;
};

struct player_s
{
	const CBasePlayer* m_pPlayer;
	bool m_bIsFadeoutDormant;
	ImColor m_FontColor;
	FontFlag m_FontFlag;
	float m_flBarSpacing;
};

static inline auto GetColorTeam(int teamnum)
{
	if (teamnum == TERRORIST)
		return ImColor(cvar.esp_player_box_color_t);
	else if (teamnum == CT)
		return ImColor(cvar.esp_player_box_color_ct);

	return ImColor(IM_COL32_WHITE);
};

static inline float fInterpolateFadeout(const player_s& p, float f1)
{
	return g_Utils.fInterp(p.m_pPlayer->m_flHistory, static_cast<float>(client_state->time),
		p.m_pPlayer->m_flHistory + cvar.esp_player_history_time, f1, 0);
}

static inline bool CalcScreenByBoundBox(const player_s& p, box_s& box)
{
	Vector vecMins, vecMaxs;
	float flScreenBot[2], flScreenTop[2];

	vecMins = p.m_pPlayer->m_vecOrigin;
	vecMaxs = p.m_pPlayer->m_bIsInPVS ? p.m_pPlayer->m_vecHitbox[11] : vecMins;

	vecMins.z += p.m_pPlayer->m_vecBoundBoxMins.z - 1.f;
	vecMaxs.z += p.m_pPlayer->m_bIsInPVS ? 9.f : p.m_pPlayer->m_vecBoundBoxMaxs.z - 7.f;

	if (!g_Utils.CalcScreen(vecMaxs, flScreenTop) ||
		!g_Utils.CalcScreen(vecMins, flScreenBot))
		return false;

	box.h = std::floorf(flScreenBot[1] - flScreenTop[1]);
	box.w = std::floorf(box.h * 0.55f);
	box.x = std::floorf(flScreenBot[0] - box.w * 0.5f);
	box.y = std::floorf(flScreenTop[1]);

	return true;
}

static inline void DrawBoundBox(const player_s& p, const box_s& box)
{
	auto color = GetColorTeam(p.m_pPlayer->m_iTeam);

	if (p.m_bIsFadeoutDormant)
		color.Value.w = fInterpolateFadeout(p, color.Value.w);

	switch (cvar.esp_player_box)
	{
	case BoxType_Default:
		cvar.esp_player_box_outline
			? g_Drawing.RectOutlined(box.x, box.y, box.w, box.h, color)
			: g_Drawing.Rect(box.x, box.y, box.w, box.h, color);
		break;

	case BoxType_Corner:
		cvar.esp_player_box_outline
			? g_Drawing.RectCornersOutlined(box.x, box.y, box.w, box.h, color)
			: g_Drawing.RectCorners(box.x, box.y, box.w, box.h, color);
		break;

	case BoxType_Round:
		cvar.esp_player_box_outline
			? g_Drawing.RectRoundOutlined(box.x, box.y, box.w, box.h, color)
			: g_Drawing.RectRound(box.x, box.y, box.w, box.h, color);
	}
}

static inline void DrawHealthBar(const player_s& p, const box_s& box)
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];

	auto health = p.m_pPlayer->m_iHealth;
	health = ImClamp(health, 0, 100);

	const auto size = ((cvar.esp_player_health <= 2) ? box.h : box.w) / 100.f * health;
	const auto alpha = p.m_bIsFadeoutDormant ? fInterpolateFadeout(p, 1) : 1.f;

	const auto color_background = ImVec4(0.25f, 0.25f, 0.25f, alpha);
	const auto color_outline = ImVec4(0.00f, 0.00f, 0.00f, alpha);
	const auto color_health = ImVec4(
		health <= 50 ? 1.f : g_Utils.fInterp(50, (float)(health), 100, 1, 0),
		health >= 50 ? 1.f : g_Utils.fInterp(0, (float)(health), 50, 0, 1),
		0.00f, alpha);

	char szHealth[4] = { '\0' };
	sprintf(szHealth, "%d", health);

	const auto text_size = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, szHealth);

	if (cvar.esp_player_health == Pos_Left)
	{
		const auto x = box.x - p.m_flBarSpacing - BarThickness;

		g_Drawing.RectFilled(x, box.y, BarThickness, box.h, color_background);
		g_Drawing.RectFilled(x, box.y + box.h, BarThickness, -size, color_health);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(x - 1, box.y - 1, BarThickness + 2, box.h + 2, color_outline);

		if (cvar.esp_player_health_value && health && health != 100)
		{
			g_Drawing.AddText(x + BarThickness * 0.5f, box.y + box.h - size - cvar.esp_font_size * 0.25f,
				p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX | FontFlags_CenterY, szHealth);
		}
	}
	else if (cvar.esp_player_health == Pos_Rigth)
	{
		const auto x = box.x + box.w + p.m_flBarSpacing;

		g_Drawing.RectFilled(x, box.y, BarThickness, box.h, color_background);
		g_Drawing.RectFilled(x, box.y + box.h, BarThickness, -size, color_health);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(x - BarThickness * 0.5f, box.y - 1, BarThickness + 2, box.h + 2, color_outline);

		if (cvar.esp_player_health_value && health && health != 100)
		{
			g_Drawing.AddText(x, box.y + box.h - size - cvar.esp_font_size * 0.25f, p.m_FontColor,
				cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX | FontFlags_CenterY, szHealth);
		}
	}
	else if (cvar.esp_player_health == Pos_Top)
	{
		const auto y = box.y - p.m_flBarSpacing - BarThickness;

		g_Drawing.RectFilled(box.x, y, box.w, BarThickness, color_background);
		g_Drawing.RectFilled(box.x + box.w, y, -size, BarThickness, color_health);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(box.x - 1, y - 1, box.w + 2, BarThickness + 2, color_outline);

		if (cvar.esp_player_health_value && health && health != 100)
		{
			g_Drawing.AddText(box.x + box.w - size - text_size.x * 0.5f, y + BarThickness * 0.5f, p.m_FontColor,
				cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX | FontFlags_CenterY, szHealth);
		}
	}
	else if (cvar.esp_player_health >= Pos_Bottom)
	{
		const auto y = box.y + box.h + p.m_flBarSpacing;

		g_Drawing.RectFilled(box.x, y, box.w, BarThickness, color_background);
		g_Drawing.RectFilled(box.x + box.w, y, -size, BarThickness, color_health);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(box.x - 1, y - 1, box.w + 2, BarThickness + 2, color_outline);

		if (cvar.esp_player_health_value && health && health != 100)
		{
			g_Drawing.AddText(box.x + box.w - size - text_size.x * 0.5f, y - BarThickness * 0.5f, p.m_FontColor,
				cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX | FontFlags_CenterY, szHealth);
		}
	}
}

static inline void DrawArmorBar(const player_s& p, const box_s& box)
{
	auto armor = p.m_pPlayer->m_iArmorType;
	armor = ImClamp(armor, 0, 2);

	if (armor == ARMOR_NONE)
		return;

	const auto armor_size = ((cvar.esp_player_armor <= 2) ? box.h : box.w) * 0.5f * armor;
	const auto alpha = p.m_bIsFadeoutDormant ? fInterpolateFadeout(p, 1) : 1.f;

	auto spacing = p.m_flBarSpacing;

	if (cvar.esp_player_armor == cvar.esp_player_health)
		spacing += spacing + BarThickness;

	const auto color_background = ImVec4(0.25f, 0.25f, 0.25f, alpha);
	const auto color_outline = ImVec4(0.00f, 0.00f, 0.00f, alpha);
	const auto color_armor = ImVec4(0.75f, 0.75f, 1.00f, alpha);

	if (cvar.esp_player_armor == Pos_Left)
	{
		const auto x = box.x - spacing - BarThickness;

		g_Drawing.RectFilled(x, box.y, BarThickness, box.h, color_background);
		g_Drawing.RectFilled(x, box.y + box.h, BarThickness, -armor_size, color_armor);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(x - 1, box.y - 1, BarThickness + 2, box.h + 2, color_outline);
	}
	else if (cvar.esp_player_armor == Pos_Rigth)
	{
		const auto x = box.x + box.w + spacing;

		g_Drawing.RectFilled(x, box.y, BarThickness, box.h, color_background);
		g_Drawing.RectFilled(x, box.y + box.h, BarThickness, -armor_size, color_armor);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(x - 1, box.y - 1, BarThickness + 2, box.h + 2, color_outline);
	}
	else if (cvar.esp_player_armor == Pos_Top)
	{
		const auto y = box.y - spacing - BarThickness;

		g_Drawing.RectFilled(box.x, y, box.w, BarThickness, color_background);
		g_Drawing.RectFilled(box.x, y, armor_size, BarThickness, color_armor);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(box.x - 1, y - 1, box.w + 2, BarThickness + 2, color_outline);
	}
	else if (cvar.esp_player_armor >= Pos_Bottom)
	{
		const auto y = box.y + box.h + spacing;

		g_Drawing.RectFilled(box.x, y, box.w, BarThickness, color_background);
		g_Drawing.RectFilled(box.x, y, armor_size, BarThickness, color_armor);

		if (cvar.esp_player_box_outline)
			g_Drawing.Rect(box.x - 1, y - 1, box.w + 2, BarThickness + 2, color_outline);
	}
}

static inline void PrintName(const player_s& p, const box_s& box)
{
	if (!strlen(p.m_pPlayer->m_szPrintName))
		return;

	std::string sName(p.m_pPlayer->m_szPrintName);

	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];
	const auto text_size = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, sName.c_str());

	auto spacing = 2.0f;

	if (cvar.esp_player_name == cvar.esp_player_health)
		spacing += p.m_flBarSpacing + BarThickness;

	if (cvar.esp_player_name == cvar.esp_player_armor && p.m_pPlayer->m_iArmorType)
		spacing += p.m_flBarSpacing + BarThickness;

	ImColor background = cvar.esp_player_background_color;

	if (p.m_bIsFadeoutDormant)
		background.Value.w = fInterpolateFadeout(p, background.Value.w);

	if (cvar.esp_player_name == Pos_Left)
	{
		const auto x = box.x - spacing - text_size.x;

		if (background.Value.w)
			g_Drawing.RectFilled(x, box.y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, box.y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, u8"%s", sName.c_str());
	}
	else if (cvar.esp_player_name == Pos_Rigth)
	{
		const auto x = box.x + box.w + spacing;

		if (background.Value.w)
			g_Drawing.RectFilled(x, box.y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, box.y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, u8"%s", sName.c_str());
	}
	else if (cvar.esp_player_name == Pos_Top)
	{
		const auto x = box.x + box.w * 0.5f;
		const auto y = box.y - spacing - cvar.esp_font_size;

		if (background.Value.w)
			g_Drawing.RectFilled(x - text_size.x * 0.5f, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX, u8"%s", sName.c_str());
	}
	else if (cvar.esp_player_name >= Pos_Bottom)
	{
		const auto x = box.x + box.w * 0.5f;
		const auto y = box.y + box.h + spacing;

		if (background.Value.w)
			g_Drawing.RectFilled(x - text_size.x * 0.5f, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX, u8"%s", sName.c_str());
	}
}

static inline void PrintWeapon(const player_s& p, const box_s& box)
{
	std::string weapon = "";

	if (strlen(p.m_pPlayer->m_szWeaponModelName))
	{
		weapon = p.m_pPlayer->m_szWeaponModelName;
		auto pos = weapon.find("/p_");

		if (pos == std::string::npos)
			return;

		weapon = weapon.substr(pos + 3, weapon.length() - pos - 3);
		pos = weapon.find(".mdl");

		if (pos == std::string::npos)
			return;

		weapon = weapon.substr(0, pos);
	}
	else if (strlen(p.m_pPlayer->m_szWeaponSoundName))
		weapon = p.m_pPlayer->m_szWeaponSoundName;

	if (weapon.length() < 1)
		return;

	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];

	auto spacing = 2.0f;

	if (cvar.esp_player_weapon == cvar.esp_player_health)
		spacing += p.m_flBarSpacing + BarThickness;

	if (cvar.esp_player_weapon == cvar.esp_player_armor && p.m_pPlayer->m_iArmorType)
		spacing += p.m_flBarSpacing + BarThickness;

	const auto text_size = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, weapon.c_str());
	const auto nextline = (bool)(cvar.esp_player_weapon == cvar.esp_player_name);

	ImColor background = cvar.esp_player_background_color;

	if (p.m_bIsFadeoutDormant)
		background.Value.w = fInterpolateFadeout(p, background.Value.w);

	if (cvar.esp_player_weapon == Pos_Left)
	{
		const auto x = box.x - spacing - text_size.x;
		const auto y = box.y + (nextline ? cvar.esp_font_size : 0.f);

		if (background.Value.w)
			g_Drawing.RectFilled(x, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, weapon.c_str());
	}
	else if (cvar.esp_player_weapon == Pos_Rigth)
	{
		const auto x = box.x + box.w + spacing;
		const auto y = box.y + (nextline ? cvar.esp_font_size : 0.f);

		if (background.Value.w)
			g_Drawing.RectFilled(x, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, weapon.c_str());
	}
	else if (cvar.esp_player_weapon == Pos_Top)
	{
		const auto x = box.x + box.w * 0.5f;
		const auto y = box.y - spacing - cvar.esp_font_size - (nextline ? cvar.esp_font_size : 0.f);

		if (background.Value.w)
			g_Drawing.RectFilled(x - text_size.x * 0.5f, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX, weapon.c_str());
	}
	else if (cvar.esp_player_weapon >= Pos_Bottom)
	{
		const auto x = box.x + box.w * 0.5f;
		const auto y = box.y + box.h + spacing + (nextline ? cvar.esp_font_size : 0.f);

		if (background.Value.w)
			g_Drawing.RectFilled(x - text_size.x * 0.5f, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX, weapon.c_str());
	}
}

static inline void PrintDistance(const player_s& p, const box_s& box)
{
	const auto* const pFont = ImGui::GetIO().Fonts->Fonts[0];

	auto delta = p.m_pPlayer->m_vecOrigin - pmove->origin;
	auto distance = ImFloor(delta.Length());

	if (cvar.esp_player_distance_measure)
		distance *= 0.025f;

	auto spacing = 2.0f;

	if (cvar.esp_player_distance == cvar.esp_player_health)
		spacing += p.m_flBarSpacing + BarThickness;

	if (cvar.esp_player_distance == cvar.esp_player_armor && p.m_pPlayer->m_iArmorType)
		spacing += p.m_flBarSpacing + BarThickness;

	auto nextline = 0.0f;

	if (cvar.esp_player_distance == cvar.esp_player_name)
		nextline += cvar.esp_font_size;

	if (cvar.esp_player_distance == cvar.esp_player_weapon)
		nextline += cvar.esp_font_size;

	char szDistance[16] = { "\0" };
	sprintf(szDistance, cvar.esp_player_distance_measure ? "%0.f m" : "%0.f u", distance);

	const auto text_size = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, szDistance);

	ImColor background = cvar.esp_player_background_color;

	if (p.m_bIsFadeoutDormant)
		background.Value.w = fInterpolateFadeout(p, background.Value.w);

	if (cvar.esp_player_distance == Pos_Left)
	{
		const auto x = box.x - spacing - text_size.x;
		const auto y = box.y + nextline;

		if (background.Value.w)
			g_Drawing.RectFilled(x, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, szDistance);
	}
	else if (cvar.esp_player_distance == Pos_Rigth)
	{
		const auto x = box.x + box.w + spacing;
		const auto y = box.y + nextline;

		if (background.Value.w)
			g_Drawing.RectFilled(x, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, szDistance);
	}
	else if (cvar.esp_player_distance == Pos_Top)
	{
		const auto x = box.x + box.w * 0.5f;
		const auto y = box.y - spacing - cvar.esp_font_size - nextline;

		if (background.Value.w)
			g_Drawing.RectFilled(x - text_size.x * 0.5f, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX, szDistance);
	}
	else if (cvar.esp_player_distance >= Pos_Bottom)
	{
		const auto x = box.x + box.w * 0.5f;
		const auto y = box.y + box.h + spacing + nextline;

		if (background.Value.w)
			g_Drawing.RectFilled(x - text_size.x * 0.5f, y, text_size.x, cvar.esp_font_size, background);

		g_Drawing.AddText(x, y, p.m_FontColor, cvar.esp_font_size, p.m_FontFlag | FontFlags_CenterX, szDistance);
	}
}

#define Skeleton() {\
	g_Drawing.LineByOrigin(hitbox[0], hitbox[7], color);\
	g_Drawing.LineByOrigin(hitbox[7], hitbox[8], color);\
	g_Drawing.LineByOrigin(hitbox[8], hitbox[9], color);\
	g_Drawing.LineByOrigin(hitbox[9], hitbox[11], color);\
	g_Drawing.LineByOrigin(hitbox[0], hitbox[1], color);\
	g_Drawing.LineByOrigin(hitbox[1], hitbox[2], color);\
	g_Drawing.LineByOrigin(hitbox[2], hitbox[3], color);\
	g_Drawing.LineByOrigin(hitbox[0], hitbox[4], color);\
	g_Drawing.LineByOrigin(hitbox[4], hitbox[5], color);\
	g_Drawing.LineByOrigin(hitbox[5], hitbox[6], color);\
	g_Drawing.LineByOrigin(hitbox[9], hitbox[12], color);\
	g_Drawing.LineByOrigin(hitbox[12], hitbox[13], color);\
	g_Drawing.LineByOrigin(hitbox[13], hitbox[14], color);\
	g_Drawing.LineByOrigin(hitbox[14], hitbox[15], color);\
	g_Drawing.LineByOrigin(hitbox[9], hitbox[16], color);\
	g_Drawing.LineByOrigin(hitbox[16], hitbox[17], color);\
	g_Drawing.LineByOrigin(hitbox[17], hitbox[18], color);\
	g_Drawing.LineByOrigin(hitbox[18], hitbox[19], color);\
}

static inline void DrawSkeleton(const player_s& p)
{
	if (!p.m_pPlayer->m_bIsInPVS)
		return;

	const auto* const hitbox = p.m_pPlayer->m_vecHitbox;

	ImColor color = cvar.esp_player_skeleton_color;

	if (p.m_bIsFadeoutDormant)
		color.Value.w = fInterpolateFadeout(p, color.Value.w);

	Skeleton();

		for (int i = 0; i < 21; i++)
		{
			float flScreen[2];

			if (g_Utils.CalcScreen(hitbox[i], flScreen))
			{
				g_Drawing.AddText(flScreen[0], flScreen[1], p.m_FontColor, cvar.esp_font_size, p.m_FontFlag, "%i", i);
			}
		}

	if (cvar.debug_visuals)
	{
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][0], p.m_pPlayer->m_vecHitboxPoints[11][2], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][0], p.m_pPlayer->m_vecHitboxPoints[11][3], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][0], p.m_pPlayer->m_vecHitboxPoints[11][4], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][6], p.m_pPlayer->m_vecHitboxPoints[11][1], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][6], p.m_pPlayer->m_vecHitboxPoints[11][2], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][6], p.m_pPlayer->m_vecHitboxPoints[11][4], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][5], p.m_pPlayer->m_vecHitboxPoints[11][1], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][5], p.m_pPlayer->m_vecHitboxPoints[11][3], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][5], p.m_pPlayer->m_vecHitboxPoints[11][4], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][7], p.m_pPlayer->m_vecHitboxPoints[11][1], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][7], p.m_pPlayer->m_vecHitboxPoints[11][2], ImColor(IM_COL32_WHITE));
		g_Drawing.LineByOrigin(p.m_pPlayer->m_vecHitboxPoints[11][7], p.m_pPlayer->m_vecHitboxPoints[11][3], ImColor(IM_COL32_WHITE));

		float flScreen[2];

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][0], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(0, 255, 0, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][1], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(255, 0, 0, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][6], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(0, 0, 255, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][3], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(255, 255, 0, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][7], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(0, 255, 255, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][4], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(255, 0, 255, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][2], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(0, 0, 0, 255));

		if (g_Utils.CalcScreen(p.m_pPlayer->m_vecHitboxPoints[11][5], flScreen))
			g_Drawing.AddRectFilled(ImVec2(flScreen[0], flScreen[1]), ImVec2(flScreen[0] + 5, flScreen[1] + 5), ImColor(255, 255, 255, 255));
	}
}

static inline void FindPoint(float* point, int screen_width, int screen_height, int degrees)
{
	const auto x = (float)(screen_width) * 0.5f;
	const auto y = (float)(screen_height) * 0.5f;

	const auto d = sqrtf(powf((point[0] - x), 2.f) + powf((point[1] - y), 2.f));
	const auto r = (float)(degrees) / d;

	point[0] = r * point[0] + (1.f - r) * x;
	point[1] = r * point[1] + (1.f - r) * y;
}

static inline bool WorldToScreen(Vector origin, float* screen, bool& behind)
{
	behind = (bool)(g_pEngine->pTriAPI->WorldToScreen(origin, screen));

	const auto screen_width = (float)(g_Screen.iWidth);
	const auto screen_height = (float)(g_Screen.iHeight);

	screen[0] = screen_width * 0.5f + 0.5f * screen[0] * screen_width + 0.5f;
	screen[1] = screen_height * 0.5f - 0.5f * screen[1] * screen_height + 0.5f;

	if (screen[0] > screen_width || screen[0] < 0.f || screen[1] > screen_height || screen[1] < 0.f || behind)
	{
		FindPoint(screen, g_Screen.iWidth, g_Screen.iHeight, g_Screen.iHeight / 2);
		return false;
	}

	return true;
}

static inline void DrawOffScreen(const player_s& p)
{
	bool behind = false;
	float screen[2];

	WorldToScreen(p.m_pPlayer->m_vecOrigin, screen, behind);

	const auto screen_width = (float)(g_Screen.iWidth);
	const auto screen_height = (float)(g_Screen.iHeight);

	if (behind)
	{
		screen[0] = screen_width - screen[0];
		screen[1] = screen_height - screen[1];
	}

	const auto screen_center_x = screen_width * 0.5f;
	const auto screen_center_y = screen_height * 0.5f;
	const auto forward = Vector(screen[0] - screen_center_x, screen_center_y - screen[1], 0);

	Vector angles;
	g_Utils.VectorAngles(forward, angles);

	const auto yaw = DEG2RAD(-angles.y);
	const auto new_point_x = screen_center_x + cvar.esp_player_out_of_pov_arrow_radius * cosf(yaw);
	const auto new_point_y = screen_center_y + cvar.esp_player_out_of_pov_arrow_radius * sinf(yaw);

	std::array<ImVec2, 3>aPoints
	{
		ImVec2(new_point_x - 10, new_point_y - 10),
		ImVec2(new_point_x + 12, new_point_y),
		ImVec2(new_point_x - 10, new_point_y + 10)
	};

	const auto points_center = ImVec2(
		(aPoints.at(0).x + aPoints.at(1).x + aPoints.at(2).x) / 3.f,
		(aPoints.at(0).y + aPoints.at(1).y + aPoints.at(2).y) / 3.f);

	for (auto& point : aPoints)
	{
		point = ImVec2(point.x - points_center.x, point.y - points_center.y);

		const auto temp_x = point.x;
		const auto temp_y = point.y;

		const auto theta = DEG2RAD(-angles.y);
		const auto c = cosf(theta);
		const auto s = sinf(theta);

		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;

		point = ImVec2(point.x + points_center.x, point.y + points_center.y);
	}

	auto color = GetColorTeam(p.m_pPlayer->m_iTeam);

	if (p.m_bIsFadeoutDormant)
		color.Value.w = fInterpolateFadeout(p, color.Value.w);

	g_Drawing.AddTriangleFilled(aPoints.at(0), aPoints.at(1), aPoints.at(2), color);
}

static inline void DrawPlayer(const CBasePlayer* pPlayer, bool bIsFadeoutDormant = false)
{
	player_s p =
	{
		pPlayer,
		bIsFadeoutDormant,
		cvar.esp_font_color,
		GetFontFlags(cvar.esp_font_outline),
		cvar.esp_player_box_outline ? 3.f : 2.f
	};

	box_s box = {};

	if (CalcScreenByBoundBox(p, box))
	{
		if (bIsFadeoutDormant)
			p.m_FontColor.Value.w = fInterpolateFadeout(p, p.m_FontColor.Value.w);

		if (cvar.esp_player_box)
			DrawBoundBox(p, box);

		if (cvar.esp_player_health)
			DrawHealthBar(p, box);

		if (cvar.esp_player_armor)
			DrawArmorBar(p, box);

		if (cvar.esp_player_skeleton)
			DrawSkeleton(p);

		if (cvar.esp_player_name)
			PrintName(p, box);

		if (cvar.esp_player_weapon)
			PrintWeapon(p, box);

		if (cvar.esp_player_distance)
			PrintDistance(p, box);
	}
	else
	{
		if (cvar.esp_player_out_of_pov_arrow)
			DrawOffScreen(p);
	}
}

void CESP::DrawPlayers()
{
	if (!cvar.esp_player_enabled)
		return;

	const auto* const pGameEntity = g_Engine.GetLocalPlayer();

	if (!pGameEntity || !pGameEntity->model)
		return;

	if (!pGameEntity->player)
		return;

	if (!pGameEntity->index)
		return;

	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		if (g_Local.m_iIndex == i)
			continue;

		const auto* const pPlayer = g_World.GetPlayer(i);

		if (!cvar.esp_player_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (pPlayer->m_vecOrigin.IsZero())
			continue;

		if (!pPlayer->m_bIsInPVS)
		{
			if (static_cast<float>(client_state->time) - pPlayer->m_flHistory < cvar.esp_player_history_time)
				DrawPlayer(pPlayer, cvar.esp_player_fadeout_dormant);
			continue;
		}

		if (g_Local.m_bIsDead && pGameEntity->curstate.iuser1 == OBS_IN_EYE &&
			pGameEntity->curstate.iuser2 == pPlayer->m_iIndex)
			continue;

		if (pPlayer->m_bIsDead)
			continue;

		DrawPlayer(pPlayer);
	}
}

static inline bool IsGrenade(const CBaseEntity* pEntity)
{
	return strstr(pEntity->m_szModelName, "hegrenade") || strstr(pEntity->m_szModelName, "flashbang") || strstr(pEntity->m_szModelName, "smokegrenade");
}

static inline void DrawDroppedWeapons(const CBaseEntity* pEntity, const float* pflScreen)
{
	if (!strlen(pEntity->m_szModelName))
		return;

	std::string weapon = pEntity->m_szModelName;
	auto pos = weapon.find("/w_", 0);

	if (pos == std::string::npos)
		return;

	weapon = weapon.substr(pos + 3, weapon.length() - pos - 3);
	pos = weapon.find(".mdl", 0);

	if (pos == std::string::npos)
		return;

	weapon = weapon.substr(0, pos);

	g_Drawing.AddText(pflScreen[0], pflScreen[1] + 6, cvar.esp_font_color, cvar.esp_font_size,
		GetFontFlags(cvar.esp_font_outline) | FontFlags_CenterX | FontFlags_CenterY, weapon.c_str());
}

static inline void DrawThrownGrenades(const CBaseEntity* pEntity, const float* pflScreen)
{
	auto color = ImColor(IM_COL32_WHITE);

	if (strstr(pEntity->m_szModelName, "hegrenade"))
		color = ImColor(255, 125, 125);
	else if (strstr(pEntity->m_szModelName, "smokegrenade"))
		color = ImColor(125, 255, 125);

	g_Drawing.AddCircle(ImVec2(pflScreen[0], pflScreen[1]), 5, color);
	g_Drawing.AddCircle(ImVec2(pflScreen[0], pflScreen[1]), 6, color);
}

static inline void DrawEntity(const CBaseEntity* pEntity)
{
	float flScreen[2];

	if (!g_Utils.CalcScreen(pEntity->m_vecOrigin, flScreen))
		return;

	const auto* const pGameEntity = g_Engine.GetEntityByIndex(pEntity->m_iIndex);

	if (!pGameEntity || !pGameEntity->model)
		return;

	if (!pGameEntity->index)
		return;

	const bool bEntityGrenade = IsGrenade(pEntity);
	const bool bEntityWeapon = !bEntityGrenade || (bEntityGrenade && !pGameEntity->curstate.owner);

	if (cvar.world_dropped_weapons && bEntityWeapon && !strstr(pEntity->m_szModelName, "c4"))
		DrawDroppedWeapons(pEntity, flScreen);

	if (cvar.world_thrown_grenades && bEntityGrenade && pGameEntity->curstate.owner)
		DrawThrownGrenades(pEntity, flScreen);
}

void CESP::DrawEntities()
{
	for (int i = 1; i <= MAX_ENTITIES; ++i)
	{
		const auto* const pEntity = g_World.GetEntity(i);

		if (pEntity->m_bIsPlayer)
			continue;

		if (!pEntity->m_bIsInPVS)
			continue;

		if (pEntity->m_vecOrigin.IsZero())
			continue;

		DrawEntity(pEntity);
	}
}

enum SoundTypes
{
	SoundType_None = 1 << 0,
	SoundType_Steps = 1 << 1,
	SoundType_Shooting = 1 << 2,
	SoundType_Buys = 1 << 3,
	SoundType_Hits = 1 << 4
};

static inline int GetSoundType(const my_sound_t& s)
{
	if (strstr(s.name, "bhit_helmet") || strstr(s.name, "bhit_kevlar") ||
		strstr(s.name, "bhit_flesh") || strstr(s.name, "headshot"))
	{
		return SoundType_Hits;
	}
	else if (strstr(s.name, "player"))
	{
		return SoundType_Steps;
	}
	else if (s.id > 0 && s.id <= MAX_CLIENTS)
	{
		if (strstr(s.name, "weapons"))
			return SoundType_Shooting;
		else if (strstr(s.name, "items"))
			return SoundType_Buys;
	}

	return SoundType_None;
}

static inline auto GetSoundColor(const my_sound_t& s)
{
	if (s.id > 0 && s.id <= MAX_CLIENTS)
	{
		const auto* const pPlayer = g_World.GetPlayer(s.id);

		if (pPlayer->m_iTeam == TERRORIST)
			return ImColor(cvar.esp_sound_color_t);
		else if (pPlayer->m_iTeam == CT)
			return ImColor(cvar.esp_sound_color_ct);
	}
	else if (GetSoundType(s) & (SoundType_Steps | SoundType_Shooting | SoundType_Hits))
	{
		if (g_Local.m_iTeam == TERRORIST)
			return ImColor(cvar.esp_sound_color_ct);
		else if (g_Local.m_iTeam == CT)
			return ImColor(cvar.esp_sound_color_t);
	}

	return ImColor(IM_COL32_WHITE);
}

#define SoundPosX flScreen[0] - g_Utils.fInterp(s1, static_cast<float>(client_state->time), s3, flSize, 0) * 0.5f
#define SoundPosY flScreen[1] - g_Utils.fInterp(s1, static_cast<float>(client_state->time), s3, flSize, 0) * 0.5f
#define SoundBoxSize g_Utils.fInterp(s1, static_cast<float>(client_state->time), s3, flSize, 0)
#define SoundAlpha g_Utils.fInterp(s1, static_cast<float>(client_state->time), s3, 1, 0)
#define SoundRadius g_Utils.fInterp(s1, static_cast<float>(client_state->time), s3, cvar.esp_sound_circle_size, 0)

static inline void DrawSound(const my_sound_t& s)
{
	const auto type = GetSoundType(s);
	auto origin = s.origin;

	if (s.id > 0 && s.id <= MAX_CLIENTS)
	{
		const auto* const pPlayer = g_World.GetPlayer(s.id);

		if (!cvar.esp_player_teammates && g_Local.m_iTeam == pPlayer->m_iTeam)
			return;

		if (type & SoundType_Steps)
			origin.z += pPlayer->m_vecBoundBoxMins.z;
	}
	else
	{
		if (type & SoundType_Steps)
			origin.z -= 36.f;
	}

	float flScreen[2];

	if (!g_Utils.CalcScreen(origin, flScreen))
		return;

	const auto s1 = s.timestamp;
	const auto s3 = s1 + cvar.esp_sound_fadeout_time;

	if (type & SoundType_Steps)
	{
		static auto step = IM_PI * 0.075f;

		auto prev_point = Vector();

		for (float lat = 0.f; lat <= IM_PI * 3.f; lat += step)
		{
			const auto sin1 = sinf(lat);
			const auto cos1 = cosf(lat);
			const auto sin3 = sinf(0);
			const auto cos3 = cosf(0);

			const auto point1 = Vector(sin1 * cos3, cos1, sin1 * sin3) * SoundRadius;
			const auto point3 = origin + point1;

			auto color = GetSoundColor(s);
			color.Value.w = SoundAlpha;

			float flPoint[2], flPrevious[2];

			if (g_Utils.CalcScreen(point3, flPoint) && g_Utils.CalcScreen(prev_point, flPrevious) && lat > 0.f)
				g_Drawing.AddLine(ImVec2(flPrevious[0], flPrevious[1]), ImVec2(flPoint[0], flPoint[1]), color);

			prev_point = point3;
		}
	}
	else if (type & (SoundType_Shooting | SoundType_Hits | SoundType_Buys))
	{
		g_Drawing.Rect(flScreen[0] - 5, flScreen[1] - 5, 10, 10, GetSoundColor(s));
	}
	else if (type & SoundType_None)
	{
		auto delta = origin - g_Local.m_vecEye;
		const auto flSize = 5000.f / delta.Length();

		auto color = GetSoundColor(s);
		color.Value.w = SoundAlpha;

		g_Drawing.Rect(SoundPosX, SoundPosY, SoundBoxSize, SoundBoxSize, color);
	}
}

void CESP::DrawSounds()
{
	if (!cvar.esp_sound)
		return;

	const auto* const pGameEntity = g_Engine.GetLocalPlayer();

	if (!pGameEntity || !pGameEntity->model)
		return;

	if (!pGameEntity->player)
		return;

	if (!pGameEntity->index)
		return;

	for (auto& sound : g_Sounds)
	{
		if (g_Local.m_bIsDead && pGameEntity->curstate.iuser1 == OBS_IN_EYE &&
			pGameEntity->curstate.iuser2 == sound.id)
			continue;

		DrawSound(sound);
	}
}

static inline auto GetPunchPosition()
{
	auto punch_pos = ImVec2(g_Screen.iWidth * 0.5f, g_Screen.iHeight * 0.5f);

	if (client_state && !client_state->punchangle.IsZero2D())
	{
		Vector vecForward;
		g_Local.m_QAngles.AngleVectors(&vecForward, NULL, NULL);

		auto start = g_Local.m_vecEye;
		auto end = start + (vecForward * g_Weapon.GetDistance());

		pmtrace_s tr;
		g_Engine.pEventAPI->EV_SetTraceHull(2);
		g_Engine.pEventAPI->EV_PlayerTrace(start, end, PM_NORMAL, -1, &tr);

		float flScreen[2];

		if (g_Utils.CalcScreen(tr.endpos, flScreen))
		{
			punch_pos.x = g_Screen.iWidth - flScreen[0];
			punch_pos.y = g_Screen.iHeight - flScreen[1];
		}
	}

	return punch_pos;
}

static inline void DrawCircles(const ImVec2& pos)
{
	if (g_Local.m_iFOV > 0 && cvar.aimbot_fov_circle)
	{
		auto aim_fov = 0.f;

		if (cvar.ragebot_active)
			aim_fov = cvar.ragebot_aim_fov;
		else if (cvar.legitbot_active && cvar.legitbot[g_Weapon.data.m_iWeaponID].aim)
			aim_fov = g_Aimbot.m_flLegitBotFOV;

		if (aim_fov > 0.f && aim_fov < (g_Local.m_iFOV / 0.5f))
		{
			const auto radius = tanf(DEG2RAD(aim_fov) * 0.5f) / tanf(DEG2RAD(g_Local.m_iFOV) * 0.5f) * g_Screen.iWidth;

			g_Drawing.AddCircleFilled(pos, radius, cvar.aimbot_fov_circle_color);
		}
	}

	if (cvar.spread_circle)
	{
		auto radius = 1000.f * g_Weapon.data.m_flSpread;

		g_Drawing.AddCircleFilled(pos, radius, cvar.spread_circle_color);
	}
}

static inline void DrawPoints(const ImVec2& pos)
{
	if (cvar.punch_recoil)
	{
		g_Drawing.RectFilled(pos.x - 1, pos.y - 1, 3, 3, cvar.punch_recoil_color);
	}
}

static inline void GrenadeTrajectory()
{

}

void CMisc::DrawMiscellaneous()
{
	__try
	{
		DrawStatuses();

		if (!g_Local.m_bIsDead && !g_pIGameUI->IsGameUIActive() && g_Weapon.IsGun())
		{
			auto punch_pos = GetPunchPosition();

			if (isfinite(punch_pos.x) && isfinite(punch_pos.y) && punch_pos.x > 0 && punch_pos.y > 0 && punch_pos.x < g_Screen.iWidth && punch_pos.y < g_Screen.iHeight)
			{
				DrawCircles(punch_pos);
				DrawPoints(punch_pos);
			}
		}
	}
	__except (g_Utils.ExceptionFilter(__FUNCTION__, GetExceptionCode(), GetExceptionInformation()))
	{
	}
}

void CMisc::DrawBomb()
{
	if (!cvar.bomb)
		return;

	const auto origin = g_World.m_vecBombOrigin;
	const auto flag = g_World.m_iBombFlag;

	if (origin.IsZero())
		return;

	float flScreen[2];

	if (g_Utils.CalcScreen(origin, flScreen))
	{
		if (flag == BOMB_FLAG_PLANTED)
		{
			const auto time = abs(client_state->time - g_World.m_dbBombPlantedTime);

			g_Drawing.AddText(flScreen[0], flScreen[1], cvar.bomb_color, cvar.esp_font_size, GetFontFlags(cvar.esp_font_outline) | FontFlags_CenterX | FontFlags_CenterY, "Bomb (%.1f)", time);
		}
		else if (flag == BOMB_FLAG_DROPPED && g_Local.m_iTeam == TERRORIST)
		{
			bool bDraw = true;

			for (auto i = 1; i <= MAX_CLIENTS; ++i)
			{
				if (i == g_Local.m_iIndex)
					continue;

				auto* const pPlayer = g_World.GetPlayer(i);

				if (!pPlayer)
					continue;

				if (pPlayer->m_bHasC4)
				{
					bDraw = false;
					break;
				}
			}

			if (bDraw)
				g_Drawing.AddText(flScreen[0], flScreen[1], cvar.bomb_color, cvar.esp_font_size, GetFontFlags(cvar.esp_font_outline) | FontFlags_CenterX | FontFlags_CenterY, "Bomb");
		}
	}
}

void CMisc::DrawStatuses()
{
	const auto pFont = ImGui::GetIO().Fonts->Fonts[0];

	static const auto space_line = 10;

	int y = 50;

	static auto trigger_disappearance_time = (float)client_state->time;
	static auto trigger_prevstate = state.trigger_active;

	if (cvar.legitbot_trigger_key)
	{
		if (state.trigger_active != trigger_prevstate)
		{
			trigger_prevstate = state.trigger_active;
			trigger_disappearance_time = (float)client_state->time;
		}

		if ((float)client_state->time - trigger_disappearance_time < 2.5f)
		{
			auto color = state.trigger_active ? ImColor(125, 255, 125) : ImColor(255, 125, 125);

			if ((float)client_state->time - trigger_disappearance_time >= 2.f)
				color.Value.w = g_Utils.fInterp(trigger_disappearance_time + 2.f, (float)client_state->time, trigger_disappearance_time + 2.5f, color.Value.w, 0);

			const auto text = state.trigger_active ? "Triggerbot: activated" : "Triggerbot: deactivated";
			const auto text_size = pFont->CalcTextSizeA(13, FLT_MAX, 0, text);
			const auto x = g_Screen.iWidth - text_size.x - 15.f;

			g_Drawing.AddText(x, y, color, 13, FontFlags_CenterY, text);

			y += text_size.y + space_line;
		}
	}

	static auto knifebot_disappearance_time = (float)client_state->time;
	static auto knifebot_prevstate = state.knifebot_active;

	if (cvar.knifebot_enabled && cvar.knifebot_key_toggle)
	{
		if (state.knifebot_active != knifebot_prevstate)
		{
			knifebot_prevstate = state.knifebot_active;
			knifebot_disappearance_time = (float)client_state->time;
		}

		if ((float)client_state->time - knifebot_disappearance_time < 2.5f)
		{
			auto color = state.knifebot_active ? ImColor(125, 255, 125) : ImColor(255, 125, 125);

			if ((float)client_state->time - knifebot_disappearance_time >= 2.f)
				color.Value.w = g_Utils.fInterp(knifebot_disappearance_time + 2.f, (float)client_state->time, knifebot_disappearance_time + 2.5f, color.Value.w, 0);

			const auto text = state.knifebot_active ? "Knifebot: activated" : "Knifebot: deactivated";
			const auto text_size = pFont->CalcTextSizeA(13, FLT_MAX, 0, text);
			const auto x = g_Screen.iWidth - text_size.x - 15.f;

			g_Drawing.AddText(x, y, color, 13, FontFlags_CenterY, text);

			y += text_size.y + space_line;
		}
	}

	static auto backtrack_disappearance_time = (float)client_state->time;
	static auto backtrack_prevstate = state.fakelatency_active;

	if (cvar.fakelatency_enabled && cvar.fakelatency_key_press)
	{
		if (state.fakelatency_active != backtrack_prevstate)
		{
			backtrack_prevstate = state.fakelatency_active;
			backtrack_disappearance_time = (float)client_state->time;
		}

		if ((float)client_state->time - backtrack_disappearance_time < 2.5f)
		{
			auto color = state.fakelatency_active ? ImColor(125, 255, 125) : ImColor(255, 125, 125);

			if (static_cast<float>(client_state->time) - backtrack_disappearance_time >= 2.f)
				color.Value.w = g_Utils.fInterp(backtrack_disappearance_time + 2.f, (float)client_state->time, backtrack_disappearance_time + 2.5f, color.Value.w, 0);

			const auto text = state.fakelatency_active ? "Backtrack: activated" : "Backtrack: deactivated";
			const auto text_size = pFont->CalcTextSizeA(13, FLT_MAX, 0, text);
			const auto x = g_Screen.iWidth - text_size.x - 15.f;

			g_Drawing.AddText(x, y, color, 13, FontFlags_CenterY, text);

			y += text_size.y + space_line;
		}
	}

	static auto fakewalk_disappearance_time = (float)client_state->time;
	static auto fakewalk_prevstate = state.fakewalk_active;

	if (cvar.ragebot_active && cvar.ragebot_fake_walk_enabled && cvar.ragebot_fake_walk_key_press)
	{
		if (state.fakewalk_active != fakewalk_prevstate)
		{
			fakewalk_prevstate = state.fakewalk_active;
			fakewalk_disappearance_time = (float)client_state->time;
		}

		if ((float)client_state->time - fakewalk_disappearance_time < 2.5f)
		{
			auto color = state.fakewalk_active ? ImColor(125, 255, 125) : ImColor(255, 125, 125);

			if ((float)client_state->time - fakewalk_disappearance_time >= 2.f)
				color.Value.w = g_Utils.fInterp(fakewalk_disappearance_time + 2.f, (float)client_state->time, fakewalk_disappearance_time + 2.5f, color.Value.w, 0);

			const auto text = state.fakewalk_active ? "Fakewalk: activated" : "Fakewalk: deactivated";
			const auto text_size = pFont->CalcTextSizeA(13, FLT_MAX, 0, text);
			const auto x = g_Screen.iWidth - text_size.x - 15.f;

			g_Drawing.AddText(x, y, color, 13, FontFlags_CenterY, text);

			y += text_size.y + space_line;
		}
	}
}

auto& CScreenLogs::GetLogs()
{
	return _logs;
}

void CScreenLogs::Log(const ImColor& color, const char* message, ...)
{
	if (!message)
		return;

	char szBuff[256] = { '\0' };

	va_list vlist = nullptr;
	va_start(vlist, message);
	vsprintf_s(szBuff, message, vlist);
	va_end(vlist);

	screenlogs_s log =
	{
		szBuff,
		color,
		static_cast<float>(client_state->time)
	};

	_logs.push_back(log);
}

void CScreenLogs::Show()
{
	auto flSpacing = 4.0f;

	for (size_t i = 0; i < _logs.size(); ++i)
	{
		if (_logs[i].timestamp + 5.0f > static_cast<float>(client_state->time))
		{
			auto color = _logs[i].color;

			if (_logs[i].timestamp + 4.5f <= static_cast<float>(client_state->time))
			{
				color.Value.w = g_Utils.fInterp(_logs[i].timestamp + 4.5f,
					static_cast<float>(client_state->time), _logs[i].timestamp + 5.0f, 1.0f, 0.0f);
			}

			if (i > 0 && flSpacing > 4.0f && _logs[i - 1].timestamp + 4.5f <= static_cast<float>(client_state->time))
			{
				flSpacing = g_Utils.fInterp(_logs[i - 1].timestamp + 4.5f, static_cast<float>(client_state->time),
					_logs[i - 1].timestamp + 5.0f, flSpacing, flSpacing - 13.0f);
			}

			g_Drawing.AddText(4, flSpacing, color, 13, FontFlags_Shadow, _logs[i].message.c_str());

			flSpacing += 13.f;
		}
	}
}

void CScreenLogs::Clear()
{
	if (_logs.size() > 0)
		_logs.clear();
}

void CVisuals::Run()
{
	g_Miscellaneous.ScreenFade();

	while (g_Sounds.size() && static_cast<float>(client_state->time) -
		g_Sounds.front().timestamp >= cvar.esp_sound_fadeout_time)
		g_Sounds.pop_front();

	auto& logs = g_ScreenLog.GetLogs();

	while (logs.size() && static_cast<float>(client_state->time) -
		logs.front().timestamp >= 5.0f)
		logs.pop_front();

	static CESP esp;
	static CMisc misc;

	misc.DrawBomb();

	esp.DrawEntities();
	esp.DrawPlayers();
	esp.DrawSounds();

	misc.DrawStatuses();
	misc.DrawMiscellaneous();

	if (cvar.screen_log)
		g_ScreenLog.Show();
}

CScreenLogs g_ScreenLog;