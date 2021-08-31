#include "framework.h"

CMenu::CMenu()
	: m_pGui(std::make_unique<CMenuGui>())
	, m_pPopupModal(std::make_unique<CMenuPopupModal>())
	, m_pHotkeys(std::make_unique<CMenuHotkeys>())
	, m_pSettingsList(std::make_unique<CMenuSettingsList>())
	, m_pFadeBg(std::make_unique<CMenuFadeBg>())
	, m_pThemes(std::make_unique<CMenuThemes>())
	, m_bIsOpened(true)
{
	m_pThemes->SetDefaultDarkTheme();
}

bool CMenu::IsOpened()
{
	return m_bIsOpened;
}

void CMenu::Open()
{
	if (!m_bIsOpened)
		m_bIsOpened = true;
}

void CMenu::Close()
{
	if (m_bIsOpened)
		m_bIsOpened = false;
}

void CMenu::Toggle()
{
	m_bIsOpened ? Close() : Open();
}

void CMenu::CursorState()
{
	static bool SetCursorState;
	static int nCenterCursorFrames;

	if (GImGui->IO.MouseDrawCursor = IsOpened())
	{
		g_pISurface->UnlockCursor();
		g_pISurface->SetCursor(Cursor::dc_arrow);

		if (!SetCursorState && (!cvars::visuals.streamer_mode || !g_pIGameUI->IsGameUIActive()))
		{
			ShowCursor(FALSE);

			nCenterCursorFrames = 2;
			SetCursorState = true;
		}

		if (!g_pIGameUI->IsGameUIActive() && !(nCenterCursorFrames--))
			g_pISurface->SurfaceSetCursorPos((int)(GImGui->IO.DisplaySize.x) / 2, (int)(GImGui->IO.DisplaySize.y) / 2);
	}
	else if (SetCursorState)
	{
		ShowCursor(TRUE);

		g_pISurface->CalculateMouseVisible();

		SetCursorState = false;
	}
}

void CMenu::DrawFadeBg()
{
	m_pFadeBg->Init();
	m_pFadeBg->Run();
}

std::unique_ptr<CMenu> g_pMenu;