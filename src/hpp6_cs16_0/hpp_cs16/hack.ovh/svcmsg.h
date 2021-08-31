void SVC_Sound();
void SVC_SpawnStaticSound();

extern svc_func_t *g_pEngineMessages;

extern sizebuf_t* pMSG_Buffer;

bool HookEngineMsg(svc_commands_e cmd, pfnEngineMessage pfn);
bool UnHookEngineMsg(svc_commands_e cmd);
void HookEngineMessages();
void UnHookEngineMessages();

const int DEFAULT_SOUND_PACKET_VOLUME = 255;
const float DEFAULT_SOUND_PACKET_ATTENUATION = 1.0f;
const int DEFAULT_SOUND_PACKET_PITCH = 100;

// Sound flags
enum
{
	SND_FL_VOLUME = (1 << 0),	// send volume
	SND_FL_ATTENUATION = (1 << 1),	// send attenuation
	SND_FL_LARGE_INDEX = (1 << 2),	// send sound number as short instead of byte
	SND_FL_PITCH = (1 << 3),	// send pitch
	SND_FL_SENTENCE = (1 << 4),	// set if sound num is actually a sentence num
	SND_FL_STOP = (1 << 5),	// stop the sound
	SND_FL_CHANGE_VOL = (1 << 6),	// change sound vol
	SND_FL_CHANGE_PITCH = (1 << 7),	// change sound pitch
	SND_FL_SPAWNING = (1 << 8)	// we're spawning, used in some cases for ambients (not sent across network)
};