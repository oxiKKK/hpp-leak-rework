#include "main.h"

std::unique_ptr<usercmd_s> cmd::m_cmd;
unsigned short cmd::oldbuttons;

void cmd::get_pointer(usercmd_s* cmd)
{
	m_cmd = std::move(std::unique_ptr<usercmd_s>(cmd));
}

cmd_list_s* cmd::get_list()
{
	static cmd_list_s cmd_list[] =
	{
		{"attack", IN_ATTACK},
		{"jump", IN_JUMP},
		{"duck", IN_DUCK},
		{"forward", IN_FORWARD},
		{"back", IN_BACK},
		{"use", IN_USE},
		{"cancel", IN_CANCEL},
		{"left", IN_LEFT},
		{"right", IN_RIGHT},
		{"moveleft", IN_MOVELEFT},
		{"moveright", IN_MOVERIGHT},
		{"attack2", IN_ATTACK2},
		{"run", IN_RUN},
		{"reload", IN_RELOAD},
		{"alt1", IN_ALT1},
		{"score", IN_SCORE}
	};

	return cmd_list;
}

unsigned short cmd::get_button(const char* cmd_name)
{
	const auto* const cmd_list = get_list();

	for (size_t i = 0; i < cmd_list_size; i++)
		if (strcmp(cmd_name, cmd_list[i].name) == 0)
			return cmd_list[i].button;

	return 0;
}

const char* cmd::get_name(const unsigned short button)
{
	const auto* const cmd_list = get_list();

	for (size_t i = 0; i < cmd_list_size; i++)
		if (button == cmd_list[i].button)
			return cmd_list[i].name;

	return (const char*)0;
}

usercmd_s* cmd::get()
{
	return m_cmd.get();
}

void cmd::execute(const unsigned short button)
{
	m_cmd->buttons |= button;
}

void cmd::execute(const char* cmd_name)
{
	execute(get_button(cmd_name));
}

void cmd::block(const unsigned short button)
{
	m_cmd->buttons &= ~button;
}

void cmd::block(const char* cmd_name)
{
	block(get_button(cmd_name));
}

bool cmd::button_is(const unsigned short button)
{
	return m_cmd->buttons & button;
}

bool cmd::button_is(const char* cmd_name)
{
	return button_is(get_button(cmd_name));
}

bool cmd::oldbutton_is(const unsigned short button)
{
	return oldbuttons & button;
}

bool cmd::oldbutton_is(const char* cmd_name)
{
	return oldbutton_is(get_button(cmd_name));
}

void cmd::sidemove(const float value)
{
	m_cmd->sidemove = value;
}

void cmd::forwardmove(const float value)
{
	m_cmd->forwardmove = value;
}

void cmd::upmove(const float value)
{
	m_cmd->upmove = value;
}

void cmd::viewangles(const Vector& angles)
{
	m_cmd->viewangles = angles;
}

void cmd::clear()
{
	m_cmd->buttons = 0;
	m_cmd->sidemove = 0.0F;
	m_cmd->forwardmove = 0.0F;
	m_cmd->upmove = 0.0F;
}

void cmd::release_pointer()
{
	oldbuttons = m_cmd->buttons;
	m_cmd.release();
}