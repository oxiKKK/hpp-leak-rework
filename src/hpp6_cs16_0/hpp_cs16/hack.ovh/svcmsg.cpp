#include "main.h"

svc_func_t *g_pEngineMessages = nullptr;

sizebuf_t* pMSG_Buffer = nullptr;

std::map<svc_commands_e, pfnEngineMessage> g_EngineMessagesMap;

void HookEngineMessages()
{
	HookEngineMsg(svc_sound, SVC_Sound);
	HookEngineMsg(svc_spawnstaticsound, SVC_SpawnStaticSound);
}

void UnHookEngineMessages()
{
	UnHookEngineMsg(svc_sound);
	UnHookEngineMsg(svc_spawnstaticsound);
}

void SVC_Sound()
{
	/*int field_mask, volume, channel, ent, sound_num, pitch;
	float attenuation, origin[3] = { 0 };

	BitBuffer buf(pMSG_Buffer->data, pMSG_Buffer->cursize);

	buf.StartBitMode();

	buf.ReadBits(13); //xz otkuda beretsy

	field_mask = buf.ReadBits(9);

	if (field_mask & SND_FL_VOLUME)
		volume = buf.ReadBits(8) / 255.0;
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;

	if (field_mask & SND_FL_ATTENUATION)
		attenuation = buf.ReadBits(8) / 64.0;

	channel = buf.ReadBits(3);
	ent = buf.ReadBits(MAX_EDICT_BITS);

	if (field_mask & SND_FL_LARGE_INDEX)
		sound_num = buf.ReadBits(16);
	else
		sound_num = buf.ReadBits(8);

	buf.ReadBitVec3Coord(origin);

	if (field_mask & SND_FL_PITCH)
		pitch = buf.ReadBits(8);

	buf.EndBitMode();*/

	g_EngineMessagesMap[svc_sound]();
}

void SVC_SpawnStaticSound()
{
	/*int sound_num, ent, pitch, flags;
	float volume, attenuation, origin[3] = { 0 };
	
	BitBuffer buf(pMSG_Buffer->data, pMSG_Buffer->cursize);

	buf.StartBitMode();

	buf.ReadBits(8); //xz

	origin[0] = buf.ReadCoord();
	origin[1] = buf.ReadCoord();
	origin[2] = buf.ReadCoord();

	sound_num = buf.ReadShort();
	volume = buf.ReadByte() / 255.0;
	attenuation = buf.ReadByte() / 64.0;
	ent = buf.ReadShort();
	pitch = buf.ReadByte();
	flags = buf.ReadByte();

	buf.EndBitMode();*/

	g_EngineMessagesMap[svc_spawnstaticsound]();
}

bool HookEngineMsg(svc_commands_e cmd, pfnEngineMessage pfn)
{
	if (g_pEngineMessages)
	{
		g_EngineMessagesMap[cmd] = g_pEngineMessages[cmd].pfnParse;
		g_pEngineMessages[cmd].pfnParse = pfn; 
		return true;
	}

	TraceLog("> %s: failed to hook svc %i.\n", __FUNCTION__, cmd);

	return false;
}

bool UnHookEngineMsg(svc_commands_e cmd)
{
	if (g_pEngineMessages && g_EngineMessagesMap[cmd] != nullptr)
	{
		g_pEngineMessages[cmd].pfnParse = g_EngineMessagesMap[cmd];
		g_EngineMessagesMap[cmd] = nullptr;
		return true;
	}

	TraceLog("> %s: failed to unhook svc %i.\n", __FUNCTION__, cmd);

	return false;
}