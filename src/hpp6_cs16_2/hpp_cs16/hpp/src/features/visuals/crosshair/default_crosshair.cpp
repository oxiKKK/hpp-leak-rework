#include "framework.h"

void CDefaultCrosshair::Draw()
{
	float flDistance, flDeltaDistance;
	float flCrosshairDistance;

	switch (g_Weapon->m_iWeaponID)
	{
	case WEAPON_P228:
	case WEAPON_HEGRENADE:
	case WEAPON_SMOKEGRENADE:
	case WEAPON_FIVESEVEN:
	case WEAPON_USP:
	case WEAPON_GLOCK18:
	case WEAPON_AWP:
	case WEAPON_FLASHBANG:
	case WEAPON_DEAGLE:
		flDistance = 8.f;
		flDeltaDistance = 3.f;
		break;
	case WEAPON_MP5N:
		flDistance = 6.f;
		flDeltaDistance = 2.f;
		break;
	case WEAPON_M3:
		flDistance = 8.f;
		flDeltaDistance = 6.f;
		break;
	case WEAPON_G3SG1:
	case WEAPON_UMP45:
	case WEAPON_M249:
		flDistance = 6.f;
		flDeltaDistance = 4.f;
		break;
	case WEAPON_AK47:
		flDistance = 4.f;
		flDeltaDistance = 4.f;
		break;
	case WEAPON_KNIFE:
	case WEAPON_TMP:
	case WEAPON_P90:
		flDistance = 7.f;
		flDeltaDistance = 3.f;
		break;
	case WEAPON_XM1014:
		flDistance = 9.f;
		flDeltaDistance = 4.f;
		break;
	case WEAPON_MAC10:
		flDistance = 9.f;
		flDeltaDistance = 3.f;
		break;
	case WEAPON_AUG:
		flDistance = 3.f;
		flDeltaDistance = 3.f;
		break;
	case WEAPON_C4:
		flDistance = 6.f;
		flDeltaDistance = 3.f;
		break;
	case WEAPON_SCOUT:
	case WEAPON_SG550:
	case WEAPON_SG552:
		flDistance = 5.f;
		flDeltaDistance = 3.f;
		break;
	default:
		flDistance = 4.f;
		flDeltaDistance = 3.f;
	}

	if (g_Weapon->m_iAccuracyFlags && g_ClientCvarsMap["cl_dynamiccrosshair"]->value)
	{
		if (g_Local->m_bIsOnGround || ~g_Weapon->m_iAccuracyFlags & ACCURACY_AIR)
		{
			if (g_Local->m_bIsDucked && g_Weapon->m_iAccuracyFlags & ACCURACY_DUCK)
			{
				flDistance *= 0.5f;

				if (g_Weapon->m_iAccuracyFlags & ACCURACY_MULTIPLY_BY_14_2)
					flDeltaDistance *= 2.f;
			}
			else
			{
				float flLimitSpeed;

				switch (g_Weapon->m_iWeaponID)
				{
				case WEAPON_MAC10:
				case WEAPON_SG550:
				case WEAPON_GALIL:
				case WEAPON_MP5N:
				case WEAPON_M3:
				case WEAPON_SG552:
				case WEAPON_AK47:
				case WEAPON_AUG:
				case WEAPON_M249:
				case WEAPON_FAMAS:
				case WEAPON_M4A1:
					flLimitSpeed = 140.f;
					break;
				case WEAPON_KNIFE:
				case WEAPON_P90:
					flLimitSpeed = 170.f;
					break;
				default:
					flLimitSpeed = 0.f;
					break;
				}

				if (g_Local->m_flVelocity > flLimitSpeed && g_Weapon->m_iAccuracyFlags & ACCURACY_SPEED)
					flDistance *= 1.5f;
			}
		}
		else
			flDistance *= 2.f;

		if (g_Weapon->m_iAccuracyFlags & ACCURACY_MULTIPLY_BY_14)
			flDistance *= 1.4f;

		if (g_Weapon->m_iAccuracyFlags & ACCURACY_MULTIPLY_BY_14_2)
			flDistance *= 1.4f;
	}

	if (m_iAmmoLastCheck >= g_Weapon->m_iShotsFired)
	{
		m_flCrosshairDistance -= m_flCrosshairDistance * 0.013f + 0.1f;
	}
	else
	{
		m_flCrosshairDistance = min(m_flCrosshairDistance + flDeltaDistance, 15.f);
	}

	m_iAmmoLastCheck = g_Weapon->m_iShotsFired;
	m_flCrosshairDistance = max(m_flCrosshairDistance, flDistance);

	int iBarSize = static_cast<int>((m_flCrosshairDistance - flDistance) * 0.5f) + 5;

	CalcCrosshairColor();
	CalcCrosshairSize();

	if (m_iCrosshairScaleBase != GImGui->IO.DisplaySize.x)
	{
		flCrosshairDistance = GImGui->IO.DisplaySize.x * m_flCrosshairDistance / m_iCrosshairScaleBase;
		iBarSize = static_cast<int>(GImGui->IO.DisplaySize.x* iBarSize / m_iCrosshairScaleBase);
	}
	else
		flCrosshairDistance = m_flCrosshairDistance;

	DrawCrosshairEx(iBarSize, flCrosshairDistance);
}

void CDefaultCrosshair::DrawCrosshairEx(int iBarSize, float flCrosshairDistance)
{
	const auto x1 = floor(GImGui->IO.DisplaySize.x * 0.5f - flCrosshairDistance - iBarSize + 1);
	const auto y1 = floor(GImGui->IO.DisplaySize.y * 0.5f);

	const auto x2 = floor(GImGui->IO.DisplaySize.x * 0.5f + flCrosshairDistance);
	const auto y2 = floor(GImGui->IO.DisplaySize.y * 0.5f);

	const auto x3 = floor(GImGui->IO.DisplaySize.x * 0.5f);
	const auto y3 = floor(GImGui->IO.DisplaySize.y * 0.5f - flCrosshairDistance - iBarSize + 1);

	const auto x4 = floor(GImGui->IO.DisplaySize.x * 0.5f);
	const auto y4 = floor(GImGui->IO.DisplaySize.y * 0.5f + flCrosshairDistance);

	const auto color = ImColor(m_iColors[0], m_iColors[1], m_iColors[2]);

	g_pRenderer->AddRectFilled(ImVec2(x1, y1), ImVec2(x1 + iBarSize, y1 + 1), color);
	g_pRenderer->AddRectFilled(ImVec2(x2, y2), ImVec2(x2 + iBarSize, y2 + 1), color);
	g_pRenderer->AddRectFilled(ImVec2(x3, y3), ImVec2(x3 + 1, y3 + iBarSize), color);
	g_pRenderer->AddRectFilled(ImVec2(x4, y4), ImVec2(x4 + 1, y4 + iBarSize), color);
}

void CDefaultCrosshair::CalcCrosshairSize()
{
	const char* size = g_ClientCvarsMap["cl_crosshair_size"]->string;

	if (!_stricmp(size, "auto"))
	{
		if (GImGui->IO.DisplaySize.x <= 640.f)
		{
			m_iCrosshairScaleBase = 1024;
		}
		else if (GImGui->IO.DisplaySize.x <= 1024.f)
		{
			m_iCrosshairScaleBase = 800;
		}
		else
		{
			m_iCrosshairScaleBase = 640;
		}
	}
	else if (!_stricmp(size, "small"))
	{
		m_iCrosshairScaleBase = 1024;
	}
	else if (!_stricmp(size, "medium"))
	{
		m_iCrosshairScaleBase = 800;
	}
	else if (!_stricmp(size, "large"))
	{
		m_iCrosshairScaleBase = 640;
	}
}

void CDefaultCrosshair::CalcCrosshairColor()
{
	const char* colors = g_ClientCvarsMap["cl_crosshair_color"]->string;

	sscanf(colors, "%i %i %i", &m_iColors[0], &m_iColors[1], &m_iColors[2]);

	m_iColors[0] = std::clamp(m_iColors[0], 0, 255);
	m_iColors[1] = std::clamp(m_iColors[1], 0, 255);
	m_iColors[2] = std::clamp(m_iColors[2], 0, 255);
}