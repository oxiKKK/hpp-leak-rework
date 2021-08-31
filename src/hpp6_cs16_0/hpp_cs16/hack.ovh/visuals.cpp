#include "main.h"

CVisuals g_Visuals;

int GetFontFlags(int type) noexcept
{
	if (type == 1)
		return FontFlags_Shadow;
	else if (type == 2)
		return FontFlags_Outline;

	return FontFlags_None;
}

CPlayerESP::CPlayerESP() noexcept :
	_fontflag(FontFlags_None),
	_pPlayer(nullptr),
	_flBarThickness(0.f),
	_flBarSpacing(0.f),
	_bIsFadeoutDormant(false)
{
	RtlSecureZeroMemory(&_box, sizeof(_box));
}

CPlayerESP::~CPlayerESP()
{
	RtlSecureZeroMemory(this, sizeof(*this));
}

ImColor CPlayerESP::GetColorTeam()
{
	if (_pPlayer->m_iTeam == TERRORIST)
		return ImColor(cvar.esp_player_box_color_t);
	else if (_pPlayer->m_iTeam == CT)
		return ImColor(cvar.esp_player_box_color_ct);

	return ImColor(IM_COL32_WHITE);
};

bool CPlayerESP::GetScreenByBoundingBox(Vector& a_vecBoundingBoxMins,
	Vector& a_vecBoundingBoxMaxs, float* a_pflScreenBot, float* a_pflScreenTop)
{
	if (!a_pflScreenBot || !a_pflScreenTop)
		return false;

	a_vecBoundingBoxMins = _pPlayer->m_vecOrigin;
	a_vecBoundingBoxMaxs = _pPlayer->m_bIsInPVS ? _pPlayer->m_vecHitbox[11] : _pPlayer->m_vecOrigin;

	a_vecBoundingBoxMins.z += _pPlayer->m_vecBoundBoxMins.z - 1.f;
	a_vecBoundingBoxMaxs.z += _pPlayer->m_bIsInPVS ? 9.f : _pPlayer->m_vecBoundBoxMaxs.z - 7.f;

	if (!g_Utils.CalcScreen(a_vecBoundingBoxMaxs, a_pflScreenTop) ||
		!g_Utils.CalcScreen(a_vecBoundingBoxMins, a_pflScreenBot))
		return false;

	_box.h = ImFloor(a_pflScreenBot[1] - a_pflScreenTop[1]);
	_box.w = ImFloor(_box.h / 1.8f);
	_box.x = ImFloor(a_pflScreenBot[0] - _box.w / 2.f);
	_box.y = ImFloor(a_pflScreenTop[1]);

	return true;
}

void CPlayerESP::BoundingBox()
{
	ImColor color = GetColorTeam();

	if (_bIsFadeoutDormant)
	{
		color.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
			static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), color.Value.w, 0.f);
	}

	switch (cvar.esp_player_box)
	{
	case PlayerBoxType_Box:
		cvar.esp_player_box_outline
			? g_DrawList.DrawBoxOutline(_box.x, _box.y, _box.w, _box.h, color)
			: g_DrawList.DrawBox(_box.x, _box.y, _box.w, _box.h, color);
		break;

	case PlayerBoxType_CornerBox:
		cvar.esp_player_box_outline
			? g_DrawList.DrawCornerBoxOutline(_box.x, _box.y, _box.w, _box.h, color)
			: g_DrawList.DrawCornerBox(_box.x, _box.y, _box.w, _box.h, color);
		break;

	case PlayerBoxType_RoundedBox:
		cvar.esp_player_box_outline
			? g_DrawList.DrawRoundBoxOutline(_box.x, _box.y, _box.w, _box.h, color)
			: g_DrawList.DrawRoundBox(_box.x, _box.y, _box.w, _box.h, color);
	}
}

void CPlayerESP::HealthBar()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	int iHealth = _pPlayer->m_iHealth;
	iHealth = ImMin(iHealth, MAX_HEALTH);
	iHealth = ImMax(iHealth, 0);

	const float flSize = (cvar.esp_player_health <= 2 ? _box.h : _box.w) * (1.f / MAX_HEALTH) * iHealth;
	const float flAlpha = _bIsFadeoutDormant
		? g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
			static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), 1.f, 0.f)
		: 1.f;

	const ImVec4 color_background = ImVec4(0.25f, 0.25f, 0.25f, flAlpha);
	const ImVec4 color_outline = ImVec4(0.f, 0.f, 0.f, flAlpha);
	const ImVec4 color_health = ImVec4(
		iHealth <= 50 ? 1.f : g_Utils.fInterp(50.f, static_cast<float>(iHealth), 100.f, 1.f, 0.f),
		iHealth >= 50 ? 1.f : g_Utils.fInterp(0.f, static_cast<float>(iHealth), 50.f, 0.f, 1.f),
		0.f,
		flAlpha);

	char cHealth[8] = { 0 };
	sprintf(cHealth, "%d", iHealth);

	const ImVec2 vecTextSize = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, cHealth);

	if (cvar.esp_player_health == Pos_Left)
	{
		const float x = _box.x - _flBarSpacing - _flBarThickness;

		g_DrawList.DrawFillArea(x, _box.y, _flBarThickness, _box.h, color_background);

		if (iHealth > 0)
			g_DrawList.DrawFillArea(x, _box.y + _box.h, _flBarThickness, -flSize, color_health);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(x - 1.f, _box.y - 1.f, _flBarThickness + 2.f, _box.h + 2.f, color_outline);

		if (cvar.esp_player_health_value && iHealth > 0 && iHealth < MAX_HEALTH)
		{
			g_DrawList.AddText(x + _flBarThickness * 0.5f, _box.y + _box.h - flSize - cvar.esp_font_size * 0.25f,
				_fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX | FontFlags_CenterY, cHealth);
		}
	}
	else if (cvar.esp_player_health == Pos_Rigth)
	{
		const float x = _box.x + _box.w + _flBarSpacing;

		g_DrawList.DrawFillArea(x, _box.y, _flBarThickness, _box.h, color_background);

		if (iHealth > 0)
			g_DrawList.DrawFillArea(x, _box.y + _box.h, _flBarThickness, -flSize, color_health);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(x - _flBarThickness * 0.5f, _box.y - 1.f, _flBarThickness + 2.f, _box.h + 2.f, color_outline);

		if (cvar.esp_player_health_value && iHealth > 0 && iHealth < MAX_HEALTH)
		{
			g_DrawList.AddText(x, _box.y + _box.h - flSize - cvar.esp_font_size * 0.25f, _fontcolor,
				cvar.esp_font_size, _fontflag | FontFlags_CenterX | FontFlags_CenterY, cHealth);
		}
	}
	else if (cvar.esp_player_health == Pos_Top)
	{
		const float y = _box.y - _flBarSpacing - _flBarThickness;

		g_DrawList.DrawFillArea(_box.x, y, _box.w, _flBarThickness, color_background);

		if (iHealth > 0)
			g_DrawList.DrawFillArea(_box.x + _box.w, y, -flSize, _flBarThickness, color_health);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(_box.x - 1.f, y - 1.f, _box.w + 2.f, _flBarThickness + 2.f, color_outline);

		if (cvar.esp_player_health_value && iHealth > 0 && iHealth < MAX_HEALTH)
		{
			g_DrawList.AddText(_box.x + _box.w - flSize - vecTextSize.x * 0.5f, y + _flBarThickness * 0.5f,
				_fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX | FontFlags_CenterY, cHealth);
		}
	}
	else if (cvar.esp_player_health >= Pos_Bottom)
	{
		const float y = _box.y + _box.h + _flBarSpacing;

		g_DrawList.DrawFillArea(_box.x, y, _box.w, _flBarThickness, color_background);

		if (iHealth > 0)
			g_DrawList.DrawFillArea(_box.x + _box.w, y, -flSize, _flBarThickness, color_health);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(_box.x - 1.f, y - 1.f, _box.w + 2.f, _flBarThickness + 2.f, color_outline);

		if (cvar.esp_player_health_value && iHealth > 0 && iHealth < MAX_HEALTH)
		{
			g_DrawList.AddText(_box.x + _box.w - flSize - vecTextSize.x * 0.5f, y - _flBarThickness * 0.5f,
				_fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX | FontFlags_CenterY, cHealth);
		}
	}
}

void CPlayerESP::ArmorBar()
{
	if (!_pPlayer->m_iArmorType)
		return;

	const float flArmorSize = (cvar.esp_player_armor <= 2 ? _box.h : _box.w) * (1.f / MAX_ARMOR) * _pPlayer->m_iArmorType;
	const float flAlpha = _bIsFadeoutDormant
		? g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
			static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), 1.f, 0.f)
		: 1.f;

	float flSpacing = _flBarSpacing;

	if (cvar.esp_player_armor == cvar.esp_player_health)
		flSpacing += flSpacing + _flBarThickness;

	const ImVec4 color_background = ImVec4(0.25f, 0.25f, 0.25f, flAlpha);
	const ImVec4 color_outline = ImVec4(0.f, 0.f, 0.f, flAlpha);
	const ImVec4 color_armor = ImVec4(0.75f, 0.75f, 1.f, flAlpha);

	if (cvar.esp_player_armor == Pos_Left)
	{
		const float x = _box.x - flSpacing - _flBarThickness;

		g_DrawList.DrawFillArea(x, _box.y, _flBarThickness, _box.h, color_background);
		g_DrawList.DrawFillArea(x, _box.y + _box.h, _flBarThickness, -flArmorSize, color_armor);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(x - 1.f, _box.y - 1.f, _flBarThickness + 2.f, _box.h + 2.f, color_outline);
	}
	else if (cvar.esp_player_armor == Pos_Rigth)
	{
		const float x = _box.x + _box.w + flSpacing;

		g_DrawList.DrawFillArea(x, _box.y, _flBarThickness, _box.h, color_background);
		g_DrawList.DrawFillArea(x, _box.y + _box.h, _flBarThickness, -flArmorSize, color_armor);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(x - 1.f, _box.y - 1.f, _flBarThickness + 2.f, _box.h + 2.f, color_outline);
	}
	else if (cvar.esp_player_armor == Pos_Top)
	{
		const float y = _box.y - flSpacing - _flBarThickness;

		g_DrawList.DrawFillArea(_box.x, y, _box.w, _flBarThickness, color_background);
		g_DrawList.DrawFillArea(_box.x, y, flArmorSize, _flBarThickness, color_armor);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(_box.x - 1.f, y - 1.f, _box.w + 2.f, _flBarThickness + 2.f, color_outline);
	}
	else if (cvar.esp_player_armor >= Pos_Bottom)
	{
		const float y = _box.y + _box.h + flSpacing;

		g_DrawList.DrawFillArea(_box.x, y, _box.w, _flBarThickness, color_background);
		g_DrawList.DrawFillArea(_box.x, y, flArmorSize, _flBarThickness, color_armor);

		if (cvar.esp_player_box_outline)
			g_DrawList.DrawBox(_box.x - 1.f, y - 1.f, _box.w + 2.f, _flBarThickness + 2.f, color_outline);
	}
}

void CPlayerESP::Name()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	const ImVec2 vecTextSize = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, _pPlayer->m_sPrintName.c_str());

	float flSpacing = 2.f;

	if (cvar.esp_player_name == cvar.esp_player_health)
		flSpacing += _flBarSpacing + _flBarThickness;

	if (cvar.esp_player_name == cvar.esp_player_armor && _pPlayer->m_iArmorType > 0)
		flSpacing += _flBarSpacing + _flBarThickness;

	ImColor background = cvar.esp_player_background_color;

	if (_bIsFadeoutDormant)
	{
		background.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
			static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), background.Value.w, 0.f);
	}

	float bg_offset = -2.f;

	if (cvar.esp_player_name == Pos_Left)
	{
		const float x = _box.x - flSpacing - vecTextSize.x;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - bg_offset, _box.y, vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, _box.y, _fontcolor, cvar.esp_font_size, _fontflag, u8"%s", _pPlayer->m_sPrintName.c_str());
	}
	else if (cvar.esp_player_name == Pos_Rigth)
	{
		const float x = _box.x + _box.w + flSpacing;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - bg_offset, _box.y, vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, _box.y, _fontcolor, cvar.esp_font_size, _fontflag, u8"%s", _pPlayer->m_sPrintName.c_str());
	}
	else if (cvar.esp_player_name == Pos_Top)
	{
		const float x = _box.x + _box.w * 0.5f;
		const float y = _box.y - flSpacing - cvar.esp_font_size;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - vecTextSize.x * 0.5f - bg_offset, y,
					vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag |
			FontFlags_CenterX, u8"%s", _pPlayer->m_sPrintName.c_str());
	}
	else if (cvar.esp_player_name >= Pos_Bottom)
	{
		const float x = _box.x + _box.w * 0.5f;
		const float y = _box.y + _box.h + flSpacing;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - vecTextSize.x * 0.5f - bg_offset, y,
					vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag |
			FontFlags_CenterX, u8"%s", _pPlayer->m_sPrintName.c_str());
	}
}

void CPlayerESP::Weapon()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	std::string sWeapon = _pPlayer->m_sPlayerModel;
	std::size_t pos = sWeapon.find("/p_");

	if (pos != std::string::npos)
	{
		sWeapon = sWeapon.substr(pos + 3, sWeapon.length() - pos - 3);
		pos = sWeapon.find(".mdl");

		if (pos != std::string::npos)
		{
			sWeapon = sWeapon.substr(0, pos);

			const ImVec2 vecTextSize = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, sWeapon.c_str());

			float flSpacing = 2.f;

			if (cvar.esp_player_weapon == cvar.esp_player_health)
				flSpacing += _flBarSpacing + _flBarThickness;

			if (cvar.esp_player_weapon == cvar.esp_player_armor && _pPlayer->m_iArmorType > 0)
				flSpacing += _flBarSpacing + _flBarThickness;

			const bool bIsNextLine = (cvar.esp_player_weapon == cvar.esp_player_name);

			ImColor background = cvar.esp_player_background_color;

			if (_bIsFadeoutDormant)
			{
				background.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
					static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), background.Value.w, 0.f);
			}

			float bg_offset = -2.f;

			if (cvar.esp_player_weapon == Pos_Left)
			{
				const float x = _box.x - flSpacing - vecTextSize.x;
				const float y = _box.y + (bIsNextLine ? cvar.esp_font_size : 0.f);

				if (background.Value.w)
				{
					for (int i = 0; i < 6; ++i)
					{
						bg_offset += 1.f;
						background.Value.w *= 0.75f;

						g_DrawList.DrawFillArea(x - bg_offset, y, vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
					}
				}

				g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag, sWeapon.c_str());
			}
			else if (cvar.esp_player_weapon == Pos_Rigth)
			{
				const float x = _box.x + _box.w + flSpacing;
				const float y = _box.y + (bIsNextLine ? cvar.esp_font_size : 0.f);

				if (background.Value.w)
				{
					for (int i = 0; i < 6; ++i)
					{
						bg_offset += 1.f;
						background.Value.w *= 0.75f;

						g_DrawList.DrawFillArea(x - bg_offset, y, vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
					}
				}

				g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag, sWeapon.c_str());
			}
			else if (cvar.esp_player_weapon == Pos_Top)
			{
				const float x = _box.x + _box.w * 0.5f;
				const float y = _box.y - flSpacing - cvar.esp_font_size - (bIsNextLine ? cvar.esp_font_size : 0.f);

				if (background.Value.w)
				{
					for (int i = 0; i < 6; ++i)
					{
						bg_offset += 1.f;
						background.Value.w *= 0.75f;

						g_DrawList.DrawFillArea(x - vecTextSize.x * 0.5f - bg_offset, y,
							vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
					}
				}

				g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX, sWeapon.c_str());
			}
			else if (cvar.esp_player_weapon >= Pos_Bottom)
			{
				const float x = _box.x + _box.w * 0.5f;
				const float y = _box.y + _box.h + flSpacing + (bIsNextLine ? cvar.esp_font_size : 0.f);

				if (background.Value.w)
				{
					for (int i = 0; i < 6; ++i)
					{
						bg_offset += 1.f;
						background.Value.w *= 0.75f;

						g_DrawList.DrawFillArea(x - vecTextSize.x * 0.5f - bg_offset, y,
							vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
					}
				}

				g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX, sWeapon.c_str());
			}
		}
	}
}

void CPlayerESP::Distance()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	float flDistance = ImFloor(pmove->origin.Distance(_pPlayer->m_vecOrigin) - 32.f);

	if (cvar.esp_player_distance_measure)
		flDistance *= 0.025f;

	float flSpacing = 2.f;

	if (cvar.esp_player_distance == cvar.esp_player_health)
		flSpacing += _flBarSpacing + _flBarThickness;

	if (cvar.esp_player_distance == cvar.esp_player_armor && _pPlayer->m_iArmorType > 0)
		flSpacing += _flBarSpacing + _flBarThickness;

	float flNextLine = 0.f;

	if (cvar.esp_player_distance == cvar.esp_player_name)
		flNextLine += cvar.esp_font_size;

	if (cvar.esp_player_distance == cvar.esp_player_weapon)
		flNextLine += cvar.esp_font_size;

	char cBuff[32] = { "\0" };
	sprintf(cBuff, cvar.esp_player_distance_measure ? "%0.f m" : "%0.f u", flDistance);

	const ImVec2 vecTextSize = pFont->CalcTextSizeA(cvar.esp_font_size, FLT_MAX, 0, cBuff);

	ImColor background = cvar.esp_player_background_color;

	if (_bIsFadeoutDormant)
	{
		background.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
			static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), background.Value.w, 0.f);
	}

	float bg_offset = -2.f;

	if (cvar.esp_player_distance == Pos_Left)
	{
		const float x = _box.x - flSpacing - vecTextSize.x;
		const float y = _box.y + flNextLine;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - bg_offset, y, vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag, cBuff);
	}
	else if (cvar.esp_player_distance == Pos_Rigth)
	{
		const float x = _box.x + _box.w + flSpacing;
		const float y = _box.y + flNextLine;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - bg_offset, y, vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag, cBuff);
	}
	else if (cvar.esp_player_distance == Pos_Top)
	{
		const float x = _box.x + _box.w * 0.5f;
		const float y = _box.y - flSpacing - cvar.esp_font_size - flNextLine;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - vecTextSize.x * 0.5f - bg_offset, y,
					vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX, cBuff);
	}
	else if (cvar.esp_player_distance >= Pos_Bottom)
	{
		const float x = _box.x + _box.w * 0.5f;
		const float y = _box.y + _box.h + flSpacing + flNextLine;

		if (background.Value.w)
		{
			for (int i = 0; i < 6; ++i)
			{
				bg_offset += 1.f;
				background.Value.w *= 0.75f;

				g_DrawList.DrawFillArea(x - vecTextSize.x * 0.5f - bg_offset, y,
					vecTextSize.x + bg_offset * 2.f, cvar.esp_font_size, background);
			}
		}

		g_DrawList.AddText(x, y, _fontcolor, cvar.esp_font_size, _fontflag | FontFlags_CenterX, cBuff);
	}
}

void CPlayerESP::Skeleton()
{
	if (!g_Utils.IsTeleported(_pPlayer->m_vecHitbox[0], _pPlayer->m_vecOrigin))
	{
		const auto hitbox = _pPlayer->m_vecHitbox;

		if (hitbox == nullptr)
			return;

		ImColor color = cvar.esp_player_skeleton_color;

		if (_bIsFadeoutDormant)
		{
			color.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
				static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), color.Value.w, 0);
		}

		g_DrawList.DrawHitbox(hitbox[0], hitbox[7], color);
		g_DrawList.DrawHitbox(hitbox[7], hitbox[8], color);
		g_DrawList.DrawHitbox(hitbox[8], hitbox[9], color);
		g_DrawList.DrawHitbox(hitbox[9], hitbox[11], color);
		g_DrawList.DrawHitbox(hitbox[0], hitbox[1], color);
		g_DrawList.DrawHitbox(hitbox[1], hitbox[2], color);
		g_DrawList.DrawHitbox(hitbox[2], hitbox[3], color);
		g_DrawList.DrawHitbox(hitbox[0], hitbox[4], color);
		g_DrawList.DrawHitbox(hitbox[4], hitbox[5], color);
		g_DrawList.DrawHitbox(hitbox[5], hitbox[6], color);
		g_DrawList.DrawHitbox(hitbox[9], hitbox[12], color);
		g_DrawList.DrawHitbox(hitbox[12], hitbox[13], color);
		g_DrawList.DrawHitbox(hitbox[13], hitbox[14], color);
		g_DrawList.DrawHitbox(hitbox[14], hitbox[15], color);
		g_DrawList.DrawHitbox(hitbox[9], hitbox[16], color);
		g_DrawList.DrawHitbox(hitbox[16], hitbox[17], color);
		g_DrawList.DrawHitbox(hitbox[17], hitbox[18], color);
		g_DrawList.DrawHitbox(hitbox[18], hitbox[19], color);
	}
}

void CPlayerESP::DebugInfo()
{
	int iSoundIndex = 0;

	if (g_SoundResolver.size())
	{
		for (size_t i = 0; i < g_SoundResolver.size(); ++i)
		{
			if (g_SoundResolver[i].m_iResolvedIndex != _pPlayer->m_iIndex)
				continue;

			iSoundIndex = g_SoundResolver[i].m_iOriginalIndex;
			break;
		}
	}

	float flSpacing = 2.f;

	if (cvar.esp_player_health == Pos_Rigth)
		flSpacing += _flBarSpacing + _flBarThickness;

	if (cvar.esp_player_armor == Pos_Rigth && _pPlayer->m_iArmorType > 0)
		flSpacing += _flBarSpacing + _flBarThickness;

	float flNextLine = 0.f;

	if (cvar.esp_player_name == Pos_Rigth)
		flNextLine += cvar.esp_font_size;

	if (cvar.esp_player_weapon == Pos_Rigth)
		flNextLine += cvar.esp_font_size;

	if (cvar.esp_player_distance == Pos_Rigth)
		flNextLine += cvar.esp_font_size;

	g_DrawList.AddText(_box.x + _box.w + flSpacing, _box.y + flNextLine, _fontcolor,
		cvar.esp_font_size, _fontflag, "idx: %d", _pPlayer->m_iIndex);

	flNextLine += cvar.esp_font_size;

	g_DrawList.AddText(_box.x + _box.w + flSpacing, _box.y + flNextLine, _fontcolor, cvar.esp_font_size,
		_fontflag, iSoundIndex ? "snd_idx: %d" : "snd_idx: not resolved", iSoundIndex);

	flNextLine += cvar.esp_font_size;

	g_DrawList.AddText(_box.x + _box.w + flSpacing, _box.y + flNextLine, _fontcolor,
		cvar.esp_font_size, _fontflag, "%i", _pPlayer->m_iTeam);

	flNextLine += cvar.esp_font_size;

	g_DrawList.AddText(_box.x + _box.w + flSpacing, _box.y + flNextLine, _fontcolor,
		cvar.esp_font_size, _fontflag, "%i", g_Local.m_iTeam); 
	
	flNextLine += cvar.esp_font_size;

	g_DrawList.AddText(_box.x + _box.w + flSpacing, _box.y + flNextLine, _fontcolor,
		cvar.esp_font_size, _fontflag, "%i", g_Local.m_iIndex);
}

void FindPoint(float* point, int screen_width, int screen_height, int degrees)
{
	const float x2 = static_cast<float>(screen_width / 2);
	const float y2 = static_cast<float>(screen_height / 2);

	const float d = sqrtf(powf((point[0] - x2), 2.f) + (powf((point[1] - y2), 2.f))); // distance
	const float r = static_cast<float>(degrees / d); // segment ratio

	point[0] = r * point[0] + (1.f - r) * x2; // find point that divides the segment
	point[1] = r * point[1] + (1.f - r) * y2; // into the ratio (1-r):r
}

bool WorldToScreen(float* origin, float* screen, bool& behind)
{
	const auto result = g_pEngine->pTriAPI->WorldToScreen(origin, screen);
	behind = result ? true : false;

	screen[0] = static_cast<float>(g_Screen.iWidth * 0.5f + 0.5f * screen[0] * g_Screen.iWidth + 0.5f);
	screen[1] = static_cast<float>(g_Screen.iHeight * 0.5f - 0.5f * screen[1] * g_Screen.iHeight + 0.5f);

	if (screen[0] > g_Screen.iWidth || screen[0] < 0.f || screen[1] > g_Screen.iHeight || screen[1] < 0.f || behind)
	{
		FindPoint(screen, g_Screen.iWidth, g_Screen.iHeight, g_Screen.iHeight / 2);
		return false;
	}

	return true;
}

void CPlayerESP::OffScreen()
{
	float flScreen[2] = {};
	bool bIsBehind = false;

	WorldToScreen(_pPlayer->m_vecOrigin, flScreen, bIsBehind);

	if (bIsBehind)
	{
		flScreen[0] = g_Screen.iWidth - flScreen[0];
		flScreen[1] = g_Screen.iHeight - flScreen[1];
	}

	const auto screen_center_x = static_cast<float>(g_Screen.iWidth / 2);
	const auto screen_center_y = static_cast<float>(g_Screen.iHeight / 2);
	const auto forward = Vector(flScreen[0] - screen_center_x, screen_center_y - flScreen[1], 0.f);

	auto angles = Vector();
	g_Utils.VectorAngles(forward, angles);

	const auto yaw = DEG2RAD(-angles.y);
	const auto new_point_x = screen_center_x + cvar.esp_player_off_screen_radius * cosf(yaw);
	const auto new_point_y = screen_center_y + cvar.esp_player_off_screen_radius * sinf(yaw);

	std::array<ImVec2, 3>aPoints
	{
		ImVec2(new_point_x - 10.f, new_point_y - 10.f),
		ImVec2(new_point_x + 12.f, new_point_y),
		ImVec2(new_point_x - 10.f, new_point_y + 10.f)
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

	ImColor color = GetColorTeam();

	if (_bIsFadeoutDormant)
	{
		color.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
			static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), color.Value.w, 0.f);
	}

	g_DrawList.AddTriangleFilled(aPoints.at(0), aPoints.at(1), aPoints.at(2), color);
}

void CPlayerESP::DrawPlayer(bool a_bIsFadeoutDormant)
{
	Vector vecBoundingBoxMins = Vector();
	Vector vecBoundingBoxMaxs = Vector();

	float flScreenTop[2] = {};
	float flScreenBot[2] = {};

	if (GetScreenByBoundingBox(vecBoundingBoxMins, vecBoundingBoxMaxs, flScreenBot, flScreenTop))
	{
		_fontcolor = cvar.esp_font_color;
		_fontflag = GetFontFlags(cvar.esp_font_outline);
		_flBarThickness = 2.f;
		_flBarSpacing = cvar.esp_player_box_outline ? 3.f : 2.f;
		_bIsFadeoutDormant = a_bIsFadeoutDormant;

		if (_bIsFadeoutDormant)
		{
			_fontcolor.Value.w = g_Utils.fInterp(static_cast<float>(_pPlayer->m_dwHistory), static_cast<float>(GetTickCount()),
				static_cast<float>(_pPlayer->m_dwHistory + cvar.esp_player_history_time), _fontcolor.Value.w, 0.f);
		}

		if (cvar.esp_player_box) BoundingBox();
		if (cvar.esp_player_health) HealthBar();
		if (cvar.esp_player_armor) ArmorBar();
		if (cvar.esp_player_name) Name();
		if (cvar.esp_player_weapon) Weapon();
		if (cvar.esp_player_distance) Distance();
		if (cvar.esp_player_skeleton) Skeleton();
		if (cvar.esp_debug_info) DebugInfo();
	}
	else
	{
		if (cvar.esp_player_off_screen) OffScreen();
	}
}

void CPlayerESP::Run()
{
	for (int i = 1; i <= g_Engine.GetMaxClients(); ++i)
	{
		_pPlayer = &g_Player[i - 1];

		if (_pPlayer == nullptr)
			continue;

		if (!cvar.esp_player_teammates && _pPlayer->m_iTeam == g_Local.m_iTeam)
			continue;

		if (!_pPlayer->m_bIsInPVS)
		{
			if (GetTickCount() - _pPlayer->m_dwHistory < gsl::narrow_cast<ulong>(cvar.esp_player_history_time))
				DrawPlayer(cvar.esp_player_fadeout_dormant);

			continue;
		}

		if (g_Local.m_bIsDead)
		{
			const auto pLocal = g_Engine.GetLocalPlayer();

			if (pLocal->curstate.iuser1 == OBS_IN_EYE && pLocal->curstate.iuser2 == i)
				continue;
		}

		if (_pPlayer->m_bIsDead)
			continue;

		DrawPlayer();
	}
}

CWorldESP::CWorldESP() noexcept :
	_pEntity(nullptr),
	_pflScreen(nullptr)
{
}

CWorldESP::~CWorldESP()
{
	RtlSecureZeroMemory(this, sizeof(*this));
}

bool CWorldESP::IsGrenade(const std::string& a_sModelName) noexcept
{
	return a_sModelName.find("hegrenade", 0) != std::string::npos ||
		a_sModelName.find("flashbang", 0) != std::string::npos ||
		a_sModelName.find("smokegrenade", 0) != std::string::npos;
}

void CWorldESP::DroppedWeapons()
{
	if (!strstr(_pEntity->m_sPrintName.c_str(), "c4"))
	{
		std::string sWeaponName = _pEntity->m_sPrintName;
		std::size_t pos = sWeaponName.find("/w_", 0);

		if (pos != std::string::npos)
		{
			sWeaponName = sWeaponName.substr(pos + 3, sWeaponName.length() - pos - 3);
			pos = sWeaponName.find(".mdl", 0);

			if (pos != std::string::npos)
			{
				sWeaponName = sWeaponName.substr(0, pos);

				g_DrawList.AddText(_pflScreen[0], _pflScreen[1] + 6.f, cvar.esp_font_color, cvar.esp_font_size,
					GetFontFlags(cvar.esp_font_outline) | FontFlags_CenterX | FontFlags_CenterY, sWeaponName.c_str());
			}
		}
	}
}

void CWorldESP::ThrownGrenades()
{
	ImColor color = ImColor(IM_COL32_WHITE);

	if (_pEntity->m_sPrintName.find("hegrenade", 0) != std::string::npos)
		color = ImColor(255, 125, 125);
	else if (_pEntity->m_sPrintName.find("smokegrenade", 0) != std::string::npos)
		color = ImColor(125, 255, 125);

	g_DrawList.AddCircle(ImVec2(_pflScreen[0], _pflScreen[1]), 5.f, color);
}

void CWorldESP::GrenadeTrail(const cl_entity_s* a_pGameEntity)
{
	if (a_pGameEntity && a_pGameEntity->index && a_pGameEntity->curstate.owner)
	{
		float flScreenStart[2] = {};
		float flScreenEnd[2] = {};

		for (auto i = 1; i <= a_pGameEntity->current_position; ++i)
		{
			if (a_pGameEntity->ph[i].origin.IsZero())
				continue;

			if (a_pGameEntity->ph[i - 1].origin.IsZero())
				continue;

			if (a_pGameEntity->ph[i].origin == a_pGameEntity->ph[i - 1].origin)
				continue;

			if (g_Utils.CalcScreen(a_pGameEntity->ph[i].origin, flScreenStart) && g_Utils.CalcScreen(a_pGameEntity->ph[i - 1].origin, flScreenEnd))
				g_DrawList.AddLine(ImVec2(flScreenStart[0], flScreenStart[1]), ImVec2(flScreenEnd[0], flScreenEnd[1]), cvar.grenade_trail_color);
		}

		for (auto i = a_pGameEntity->current_position + 1; i < HISTORY_MAX; ++i)
		{
			int index_end = i + 1;

			if (i == HISTORY_MAX - 1)
				index_end = 0;

			if (a_pGameEntity->ph[i].origin.IsZero())
				continue;

			if (a_pGameEntity->ph[index_end].origin.IsZero())
				continue;

			if (a_pGameEntity->ph[i].origin == a_pGameEntity->ph[index_end].origin)
				continue;

			if (g_Utils.CalcScreen(a_pGameEntity->ph[i].origin, flScreenStart) && g_Utils.CalcScreen(a_pGameEntity->ph[index_end].origin, flScreenEnd))
				g_DrawList.AddLine(ImVec2(flScreenStart[0], flScreenStart[1]), ImVec2(flScreenEnd[0], flScreenEnd[1]), cvar.grenade_trail_color);
		}
	}
}

void CWorldESP::DrawWorldItems()
{
	float flScreen[2] = {};

	if (g_Utils.CalcScreen(_pEntity->m_vecOrigin, flScreen))
	{
		_pflScreen = flScreen;//mb srazy viwe??

		const auto pEntityOwner = g_Engine.GetEntityByIndex(_pEntity->m_iIndex)->curstate.owner;
		const auto bIsGrenade = IsGrenade(_pEntity->m_sPrintName);

		if (cvar.esp_world_dropped_weapons && (!bIsGrenade || (bIsGrenade && !pEntityOwner)))
			DroppedWeapons();

		if (cvar.esp_world_thrown_grenades && bIsGrenade && pEntityOwner)
			ThrownGrenades();
	}
}

void CWorldESP::Run()
{
	for (auto i = 0; i < MAX_ENTITIES; ++i)
	{
		_pEntity = &g_Entity[i];
	
		if (_pEntity == nullptr)
			continue;

		if (!_pEntity->m_bIsInPVS)
			continue;

		if (_pEntity->m_bIsPlayer)
			continue;

		if (cvar.esp_enabled)
			DrawWorldItems();

		if (cvar.grenade_trail && IsGrenade(_pEntity->m_sPrintName))
		{
			const auto* pGameEntity = g_Engine.GetEntityByIndex(i);
			GrenadeTrail(pGameEntity);
		}
	}
}

CSoundESP::CSoundESP() noexcept :
	_szSoundName(nullptr),
	_iSoundIndex(0),
	_iSoundType(0)
{
}

CSoundESP::~CSoundESP()
{
	RtlSecureZeroMemory(this, sizeof(*this));
}

ImColor CSoundESP::GetSoundColor()
{
	if (_iSoundIndex > 0 && _iSoundIndex <= MAX_CLIENTS)
	{
		const int index = _iSoundIndex - 1;

		if (g_Player[index].m_iTeam == TERRORIST)
			return ImColor(cvar.esp_sound_color_t);
		else if (g_Player[index].m_iTeam == CT)
			return ImColor(cvar.esp_sound_color_ct);
	}
	else if (_iSoundType & (SoundType_Steps | SoundType_Shooting | SoundType_Hits))
	{
		if (g_Local.m_iTeam == TERRORIST)
			return ImColor(cvar.esp_sound_color_ct);
		else if (g_Local.m_iTeam == CT)
			return ImColor(cvar.esp_sound_color_t);
	}

	return ImColor(IM_COL32_WHITE);
}

int CSoundESP::GetSoundType()
{
	if (strstr(_szSoundName, "bhit_helmet") || strstr(_szSoundName, "bhit_kevlar") ||
		strstr(_szSoundName, "bhit_flesh") || strstr(_szSoundName, "headshot"))
	{
		return SoundType_Hits;
	}
	else if (strstr(_szSoundName, "player"))
	{
		return SoundType_Steps;
	}
	else if (_iSoundIndex > 0 && _iSoundIndex <= MAX_CLIENTS)
	{
		if (strstr(_szSoundName, "weapons"))
		{
			return SoundType_Shooting;
		}
		else if (strstr(_szSoundName, "items"))
		{
			return SoundType_Buys;
		}
	}

	return SoundType_None;
}

#define SoundPosX flScreen[0] - g_Utils.fInterp(static_cast<float>(sound.timestamp), static_cast<float>(GetTickCount ()), static_cast<float>(dwTimeEnd), flSize, 0.f) * 0.5f
#define SoundPosY flScreen[1] - g_Utils.fInterp(static_cast<float>(sound.timestamp), static_cast<float>(GetTickCount ()), float(dwTimeEnd), flSize, 0.f) * 0.5f
#define SoundBoxSize g_Utils.fInterp(static_cast<float>(sound.timestamp), static_cast<float>(GetTickCount ()), static_cast<float>(dwTimeEnd), flSize, 0.f)
#define SoundAlpha g_Utils.fInterp(static_cast<float>(sound.timestamp), static_cast<float>(GetTickCount ()), static_cast<float>(dwTimeEnd), 1.f, 0.f)
#define SoundRadius g_Utils.fInterp(static_cast<float>(sound.timestamp), static_cast<float>(GetTickCount ()), static_cast<float>(dwTimeEnd), cvar.esp_sound_circle_size, 0.f)

void CSoundESP::Run()
{
	for (my_sound_t sound : g_Sounds)
	{
		_szSoundName = sound.name;
		_iSoundIndex = sound.id;
		_iSoundType = GetSoundType();

		const bool bIsPlayerSound = (_iSoundIndex > 0 && _iSoundIndex <= MAX_CLIENTS);

		if (!cvar.esp_player_teammates && bIsPlayerSound)
		{
			if (g_Local.m_iTeam == g_Player[_iSoundIndex - 1].m_iTeam)
				continue;
		}

		Vector vecOrigin = sound.origin;

		if (_iSoundType & SoundType_Steps)
		{
			vecOrigin.z += bIsPlayerSound
				? g_Player[_iSoundIndex - 1].m_vecBoundBoxMins.z
				: (-36.f);
		}

		float flScreen[2] = {};

		if (g_Utils.CalcScreen(vecOrigin, flScreen))
		{
			//g_DrawList.AddText(flScreen[0], flScreen[1], GetSoundColor(), cvar.esp_font_size, 0,"%i %s", _iSoundIndex, _szSoundName);

			const DWORD dwTimeEnd = sound.timestamp + cvar.esp_sound_fadeout_time;

			if (_iSoundType & SoundType_Steps)
			{
				static float flStep = static_cast<float>(M_PI * 3.f / 40.f);

				auto prev_point = Vector();

				for (float lat = 0.f; lat <= static_cast<float>(M_PI * 3.f); lat += flStep)
				{
					const float sin1 = sinf(lat);
					const float cos1 = cosf(lat);
					const float sin3 = sinf(0.f);
					const float cos3 = cosf(0.f);

					const auto point1 = Vector(sin1 * cos3, cos1, sin1 * sin3) * SoundRadius;
					const auto point3 = vecOrigin + point1;

					ImColor color = GetSoundColor();
					color.Value.w = SoundAlpha;

					float flPoint[2] = {};
					float flPrevious[2] = {};

					if (g_Utils.CalcScreen(point3, flPoint) && g_Utils.CalcScreen(prev_point, flPrevious))
					{
						if (lat > 0.f)
							g_DrawList.AddLine(ImVec2(flPrevious[0], flPrevious[1]), ImVec2(flPoint[0], flPoint[1]), color);
					}

					prev_point = point3;
				}
			}
			else if (_iSoundType & (SoundType_Shooting | SoundType_Hits | SoundType_Buys))
			{
				const ImColor color = GetSoundColor();

				g_DrawList.DrawBox(flScreen[0] - 5.f, flScreen[1] - 5.f, 10.f, 10.f, color);
			}
			else if (_iSoundType & SoundType_None)
			{
				const float flSize = 5000.f / g_Local.m_vecEye.Distance(vecOrigin);
				ImColor color = GetSoundColor();
				color.Value.w = SoundAlpha;

				g_DrawList.DrawBox(SoundPosX, SoundPosY, SoundBoxSize, SoundBoxSize, color);
			}
		}
	}
}

CMiscESP::CMiscESP() noexcept :
	_vecPunchPosition(ImVec2())
{
}

CMiscESP::~CMiscESP()
{
	RtlSecureZeroMemory(this, sizeof(*this));
}

ImVec2 CMiscESP::GetPunchPosition()
{
	ImVec2 vecPunchPosition = ImVec2(g_Screen.iWidth * 0.5f, g_Screen.iHeight * 0.5f);

	if (!g_Weapon.m_vecPunchangle.IsZero2D())
	{
		Vector vecForward = Vector();
		QAngle(g_Local.m_vecAngles).AngleVectors(&vecForward, 0, 0);

		Vector vecStart = g_Local.m_vecEye;
		Vector vecEnd = g_Local.m_vecEye + (vecForward * g_Weapon.CurrentDistance());

		pmtrace_t tr;
		g_Engine.pEventAPI->EV_SetTraceHull(2);
		g_Engine.pEventAPI->EV_PlayerTrace(vecStart, vecEnd, PM_NORMAL, -1, &tr);

		float flScreen[2] = {};

		if (g_Utils.CalcScreen(tr.endpos, flScreen))
		{
			vecPunchPosition.x = g_Screen.iWidth - flScreen[0];
			vecPunchPosition.y = g_Screen.iHeight - flScreen[1];
		}
	}

	return vecPunchPosition;
}

void CMiscESP::Statuses()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	if (g_Misc.m_bFakeLatencyActive)
	{
		const ImVec4 color = ImVec4(
			g_Globals.backtrack >= 750.f ? 1.f : g_Utils.fInterp(0.f, g_Globals.backtrack, 750.f, 0.f, 1.f),
			g_Globals.backtrack <= 750.f ? 1.f : g_Utils.fInterp(750.f, g_Globals.backtrack, 1500.f, 1.f, 0.f),
			0.f,
			1.f);

		g_DrawList.AddText(10.f, g_Screen.iHeight - 60.f, color, 18.f, FontFlags_Shadow, "PING");
	}

	static DWORD dwTemporaryBlockTimer = GetTickCount();
	static bool bTriggerPreviousStatus = g_Aimbot.m_bTriggerStatus;

	if (cvar.legitbot_trigger_key)
	{
		if (g_Aimbot.m_bTriggerStatus != bTriggerPreviousStatus)
		{
			bTriggerPreviousStatus = g_Aimbot.m_bTriggerStatus;
			dwTemporaryBlockTimer = GetTickCount();
		}

		if (GetTickCount() - dwTemporaryBlockTimer < 2500)
		{
			ImColor color = g_Aimbot.m_bTriggerStatus ? ImColor(125, 255, 125) : ImColor(255, 125, 125);

			if (dwTemporaryBlockTimer >= 2000)
			{
				color.Value.w = g_Utils.fInterp(static_cast<float>(dwTemporaryBlockTimer + 2000), static_cast<float>(GetTickCount()),
					static_cast<float>(dwTemporaryBlockTimer + 2500), color.Value.w, 0.f);
			}

			const auto text = g_Aimbot.m_bTriggerStatus ? "Triggerbot activated" : "Triggerbot deactivated";
			const auto text_size = pFont->CalcTextSizeA(13.f, FLT_MAX, 0, text);
			const auto x = g_Screen.iWidth - text_size.x - 15.f;

			g_DrawList.AddText(x, 50.f, color, 13.f, FontFlags_CenterY, text);
		}
	}
}

void CMiscESP::Circles()
{
	if (cvar.legitbot_active && cvar.legitbot_fov_circle && cvar.legitbot[g_Weapon.m_iWeaponID].aim && g_Local.m_iFOV > 0)
	{
		float flFOV = g_Aimbot.m_flLegitBotFOV;

		if (flFOV > 0.f)
		{
			const float flRadius = tanf(DEG2RAD(flFOV) * 0.5f) / tanf(DEG2RAD(g_Local.m_iFOV) * 0.5f) * g_Screen.iWidth;

			g_DrawList.AddCircleFilled(_vecPunchPosition, flRadius, cvar.legitbot_fov_circle_color, 1000);
		}
	}

	if (cvar.spread_fov_circle)
	{
		const float flRadius = g_Weapon.m_flSpreadSimulated * 2000.f;

		g_DrawList.AddCircleFilled(_vecPunchPosition, flRadius, cvar.spread_fov_circle_color, 1000);
	}
}

void CMiscESP::Points()
{
	if (cvar.punch_recoil)
	{
		const auto pos = ImVec2(_vecPunchPosition.x - 1.f, _vecPunchPosition.y - 1.f);
		const auto size = 3.f;

		g_DrawList.DrawFillArea(pos.x, pos.y, size, size, ImColor(cvar.punch_recoil_color));
	}
}

void CMiscESP::Run()
{
	_vecPunchPosition = GetPunchPosition();

	Statuses();

	if (!g_Local.m_bIsDead && !g_pGameUI->IsGameUIActive())
	{
		Circles();
		Points();
	}
}

CVisuals::CVisuals() noexcept
{
}

CVisuals::~CVisuals()
{
	RtlSecureZeroMemory(this, sizeof(*this));
}

void CVisuals::Bomb()
{
	if (cvar.esp_bomb && g_World.m_Bomb.m_iFlag == BOMB_FLAG_PLANTED)
	{
		float flScreen[2] = {};

		if (g_Utils.CalcScreen(g_World.m_Bomb.m_vOrigin, flScreen))
		{
			g_DrawList.AddText(flScreen[0], flScreen[1], cvar.esp_bomb_color, cvar.esp_font_size,
				GetFontFlags(cvar.esp_font_outline) | FontFlags_CenterX | FontFlags_CenterY, "Bomb");
		}
	}
}

void CVisuals::DrawMisc()
{
	if (g_Local.m_bIsDead)
		return;

	if (!g_Weapon.IsCurrentWeaponGun())
		return;

	if (cvar.crosshair_snipers && g_Weapon.IsCurrentWeaponSniper() && !g_Local.m_bIsScoped)
	{
		auto indentation = 10;
		auto size = 7;

		if (g_Local.m_flHeight > 0)
			indentation = 18;
		else if (pmove->flags & FL_DUCKING)
			indentation = 6;

		g_DrawList.AddLine(ImVec2(g_Screen.iWidth / 2, g_Screen.iHeight / 2 + indentation), ImVec2(g_Screen.iWidth / 2, g_Screen.iHeight / 2 + indentation + size), cvar.crosshair_snipers_color);
		g_DrawList.AddLine(ImVec2(g_Screen.iWidth / 2, g_Screen.iHeight / 2 - indentation), ImVec2(g_Screen.iWidth / 2, g_Screen.iHeight / 2 - indentation - size), cvar.crosshair_snipers_color);
		g_DrawList.AddLine(ImVec2(g_Screen.iWidth / 2 + indentation, g_Screen.iHeight / 2), ImVec2(g_Screen.iWidth / 2 + indentation + size, g_Screen.iHeight / 2), cvar.crosshair_snipers_color);
		g_DrawList.AddLine(ImVec2(g_Screen.iWidth / 2 - indentation, g_Screen.iHeight / 2), ImVec2(g_Screen.iWidth / 2 - indentation - size, g_Screen.iHeight / 2), cvar.crosshair_snipers_color);
	}
}

void CVisuals::Run()
{
	if (!g_pIRunGameEngine->IsInGame())
		return;

	while (g_Sounds.size() && GetTickCount() - g_Sounds.front().timestamp >= cvar.esp_sound_fadeout_time)
		g_Sounds.pop_front();

	_WorldESP.Run();

	if (cvar.esp_enabled && cvar.esp_player)
		_PlayerESP.Run();

	if (cvar.esp_sound)
		_SoundESP.Run();

	_MiscESP.Run();

	Bomb();
	DrawMisc();
}

void CVisuals::Brightness()
{
	static bool bOverrided = false;

	if (cvar.visuals && cvar.brightness && g_pIRunGameEngine->IsInGame() && !g_Globals.m_bUnloading)
	{
		static ImColor ImPreviousColor = {};

		if (!bOverrided || ImPreviousColor != ImColor(cvar.brightness_color[0], cvar.brightness_color[1], cvar.brightness_color[2], cvar.brightness_color[3]))
		{
			g_Engine.OverrideLightmap(1);
			g_Engine.SetLightmapColor(cvar.brightness_color[0], cvar.brightness_color[1], cvar.brightness_color[2]);
			g_Engine.SetLightmapDarkness(cvar.brightness_color[3]);

			ImPreviousColor = ImColor(cvar.brightness_color[0], cvar.brightness_color[1], cvar.brightness_color[2], cvar.brightness_color[3]);

			bOverrided = true;
		}
	}
	else if ((!cvar.brightness && bOverrided) || g_Globals.m_bUnloading)
	{
		bOverrided = false;
		g_Engine.OverrideLightmap(0);
	}
}

#define IsInvisiblePart glDepthFunc (GL_GREATER); glDisable(GL_DEPTH_TEST)
#define IsVisiblePart glDepthFunc (GL_LESS); glEnable(GL_DEPTH_TEST)
#define StudioRenderFinal() { g_Globals.m_bRenderColoredModels = true; oStudioRenderFinal(); g_Globals.m_bRenderColoredModels = false; }

__forceinline void BeginRenderType(int rendertype)
{
	switch (rendertype)
	{
	case RenderType_Wireframe:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(0.5f);
	case RenderType_Material:
	case RenderType_Flat:
		glDisable(GL_TEXTURE_2D);
	case RenderType_Texture:
		break;
	}

	g_Globals.m_iRenderType = rendertype;
}

__forceinline void EndRenderType()
{
	glEnable(GL_TEXTURE_2D);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	g_Globals.m_iRenderType = 0;
}

void CVisuals::ColoredModels()
{
	auto pGameEntity = g_Studio.GetCurrentEntity();

	if (pGameEntity && pGameEntity->model && strlen(pGameEntity->model->name))
	{
		if (pGameEntity->index > 0 && pGameEntity->index <= MAX_CLIENTS && pGameEntity->index != g_Local.m_iIndex) //Players only
		{
			auto *pPlayer = &g_Player[pGameEntity->index - 1];

			if (pPlayer)
			{
				if (cvar.disable_render_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
					return;

				if (cvar.colored_models_enabled && cvar.colored_models_player)
				{
					if (!cvar.colored_models_player_teammates && pPlayer->m_iTeam == g_Local.m_iTeam)
					{
						oStudioRenderModel();
						return;
					}

					pGameEntity->curstate.rendermode = 0;
					pGameEntity->curstate.renderfx = 0;
					pGameEntity->curstate.renderamt = 0;
					g_Studio.SetForceFaceFlags(0);

					BeginRenderType(cvar.colored_models_player);

					if (cvar.colored_models_player_invisible)
					{
						IsInvisiblePart;

						if (pPlayer->m_iTeam == TERRORIST)
							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_t_invisible, sizeof(g_Globals.m_flRenderColor));
						else if (pPlayer->m_iTeam == CT)
							RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_ct_invisible, sizeof(g_Globals.m_flRenderColor));
						else
							RtlSecureZeroMemory(g_Globals.m_flRenderColor, sizeof(g_Globals.m_flRenderColor));

						StudioRenderFinal();
					}

					IsVisiblePart;

					if (pPlayer->m_iTeam == TERRORIST)
						RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_t, sizeof(g_Globals.m_flRenderColor));
					else if (pPlayer->m_iTeam == CT)
						RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_color_ct, sizeof(g_Globals.m_flRenderColor));
					else
						RtlSecureZeroMemory(g_Globals.m_flRenderColor, sizeof(g_Globals.m_flRenderColor));

					StudioRenderFinal();

					EndRenderType();
					return;
				}
			}
		}
		else if (cvar.colored_models_enabled)
		{
			if (cvar.colored_models_hands && pGameEntity->index == g_Local.m_iIndex && strstr(pGameEntity->model->name, "/v_"))
			{
				BeginRenderType(cvar.colored_models_hands);

				IsVisiblePart;

				RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_hands_color, sizeof(g_Globals.m_flRenderColor));

				StudioRenderFinal();

				EndRenderType();
				return;
			}
			else if (cvar.colored_models_dropped_weapons && strstr(pGameEntity->model->name, "/w_"))
			{
				BeginRenderType(cvar.colored_models_dropped_weapons);

				IsVisiblePart;

				RtlCopyMemory(g_Globals.m_flRenderColor, cvar.colored_models_dropped_weapons_color, sizeof(g_Globals.m_flRenderColor));

				StudioRenderFinal();

				EndRenderType();
				return;
			}
		}
	}

	oStudioRenderModel();
}