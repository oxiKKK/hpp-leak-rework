class IBaseInterface
{
public:
	virtual			~IBaseInterface() {}
};

typedef IBaseInterface* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
typedef IBaseInterface* (*InstantiateInterfaceFn)();

extern CreateInterfaceFn CaptureFactory(HMODULE hModule);
extern PVOID CaptureInterface(CreateInterfaceFn Interface, const char* InterfaceName);

#include "common/const.h"
#include "common/in_buttons.h"
#include "common/cl_entity.h"
#include "common/ref_params.h"
#include "common/net_api.h"
#include "common/event_api.h"
#include "common/Sequence.h"
#include "common/vmodes.h"
#include "common/screenfade.h"
#include "common/dlight.h"
#include "common/r_studioint.h"
#include "common/cvardef.h"
#include "common/entity_types.h"
#include "common/qlimits.h"
#include "common/triangleapi.h"
#include "common/r_efx.h"
#include "common/IDemoPlayer.h"
#include "common/hltv.h"
#include "common/parsemsg.h"

#include "engine/archtypes.h"
#include "engine/studio.h"
#include "engine/model.h"
#include "engine/cdll_int.h"
#include "engine/sound.h"
#include "engine/event.h"
#include "engine/delta_packet.h"
#include "engine/consistency.h"
#include "engine/client.h"

#include "pm_shared/pm_shared.h"
#include "pm_shared/pm_materials.h"
#include "pm_shared/pm_defs.h"
#include "pm_shared/pm_movevars.h"

#include "public/keydefs.h"
#include "public/steam/isteamclient.h"
#include "public/steam/isteamremotestorage.h"
#include "public/steam/isteamscreenshots.h"
#include "public/steam/isteamutils.h"
#include "public/steam/isteamfriends.h"
#include "public/steam/isteamuser.h"

#include "dlls/activity.h"
#include "dlls/player.h"
#include "dlls/cdll_dll.h"

#include "vgui/VGUI_Frame.h"
#include "vgui/VGUI_Panel.h"
#include "vgui/VGUI_KeyCode.h"
#include "vgui2/IHTML.h"
#include "vgui2/IPanel.h"
#include "vgui2/ISurface.h"
#include "vgui2/IEngineVGui.h"

#include "gameui/IVGuiModuleLoader.h"
#include "gameui/IRunGameEngine.h"
#include "gameui/IGameUI.h"
#include "gameui/IGameConsole.h"
#include "gameui/IGameUIFuncs.h"

#pragma comment(lib, "vgui.lib")

#define MAX_TOTAL_CMDS		62 //if (build_num < 5971) choke_limit = 16;

#define HITBOX_POINTS_MAX	8

#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
// Only keep 1 second of data
#define LAG_COMPENSATION_DATA_TIME	1.0f

typedef enum 
{
	HULL_REGULAR,
	HULL_DUCKED,
	HULL_POINT
} HullList_t;

typedef float TransformMatrix[MAXSTUDIOBONES][3][4];
typedef float vec4_t[4];

enum Hitboxes
{
	HITBOX_STOMACH,
	HITBOX_LEFT_FOOT,
	HITBOX_LEFT_CALF,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_FOOT,
	HITBOX_RIGHT_CALF,
	HITBOX_RIGHT_THIGH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_NECK,
	HITBOX_HEAD,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_HAND,
	HITBOX_LEFT_FOREARM,
	HITBOX_LEFT_WRIST,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_HAND,
	HITBOX_RIGHT_FOREARM,
	HITBOX_RIGHT_WRIST,
	HITBOX_SHIELD,
	HITBOX_MAX
};

enum HitgroupList
{
	HITGROUP_GENERIC,
	HITGROUP_HEAD,
	HITGROUP_CHEST,
	HITGROUP_STOMACH,
	HITGROUP_LEFTARM,
	HITGROUP_RIGHTARM,
	HITGROUP_LEFTLEG,
	HITGROUP_RIGHTLEG
};

using xcommand_t = void(__cdecl*)();

typedef struct cmd_s
{
	struct cmd_s* next;
	const char* name;
	xcommand_t function;
	int flags;
} cmd_t, *pcmd_t;

struct extra_player_info_old_t
{
	short frags;
	short deaths;
	short team_id;
	int has_c4;
	int vip;
	Vector origin;
	float radarflash;
	int radarflashon;
	int radarflashes;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME_LENGTH];
	bool dead;
	float showhealth;
	int health;
	char location[32];
};

struct extra_player_info_t
{
	short frags;
	short deaths;
	short team_id;
	int has_c4;
	int vip;
	Vector origin;
	float radarflash;
	int radarflashon;
	int radarflashes;
	short playerclass;
	short teamnumber;
	char teamname[MAX_TEAM_NAME_LENGTH];
	bool dead;
	float showhealth;
	int unk2;
	char location[32];
	int health;
	int money;
	int has_defusekits;
};

typedef struct ClientUserMsg
{
	int msg;
	int size;
	char name[16];
	struct ClientUserMsg* next;
	pfnUserMsgHook pfn;
} *PClientUserMsg;

#define MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s*	next;
	char				name[MAX_ALIAS_NAME];
	char*				value;
} cmdalias_t;

typedef struct cl_enginefuncs_s
{
	// sprite handlers
	HSPRITE				(*pfnSPR_Load)					(const char* szPicName);
	int					(*pfnSPR_Frames)				(HSPRITE hPic);
	int					(*pfnSPR_Height)				(HSPRITE hPic, int frame);
	int					(*pfnSPR_Width)					(HSPRITE hPic, int frame);
	void				(*pfnSPR_Set)					(HSPRITE hPic, int r, int g, int b);
	void				(*pfnSPR_Draw)					(int frame, int x, int y, const wrect_t* prc);
	void				(*pfnSPR_DrawHoles)				(int frame, int x, int y, const wrect_t* prc);
	void				(*pfnSPR_DrawAdditive)			(int frame, int x, int y, const wrect_t* prc);
	void				(*pfnSPR_EnableScissor)			(int x, int y, int width, int height);
	void				(*pfnSPR_DisableScissor)		(void);
	client_sprite_t*	(*pfnSPR_GetList)				(char* psz, int* piCount);

	// screen handlers
	void				(*pfnFillRGBA)					(int x, int y, int width, int height, int r, int g, int b, int a);
	int					(*pfnGetScreenInfo)				(SCREENINFO* pscrinfo);
	void				(*pfnSetCrosshair)				(HSPRITE hspr, wrect_t rc, int r, int g, int b);

	// cvar handlers
	struct cvar_s*		(*pfnRegisterVariable)			(const char* szName, const char* szValue, int flags);
	float				(*pfnGetCvarFloat)				(const char* szName);
	char*				(*pfnGetCvarString)				(const char* szName);

	// command handlers
	int					(*pfnAddCommand)				(const char* cmd_name, void (*function)(void));
	int					(*pfnHookUserMsg)				(const char* szMsgName, pfnUserMsgHook pfn);
	int					(*pfnServerCmd)					(const char* szCmdString);
	int					(*pfnClientCmd)					(const char* szCmdString);

	void				(*pfnGetPlayerInfo)				(int ent_num, hud_player_info_t* pinfo);

	// sound handlers
	void				(*pfnPlaySoundByName)			(char* szSound, float volume);
	void				(*pfnPlaySoundByIndex)			(int iSound, float volume);

	// vector helpers
	void				(*pfnAngleVectors)				(const float* vecAngles, float* forward, float* right, float* up);

	// text message system
	client_textmessage_t* (*pfnTextMessageGet)			(const char* pName);
	int					(*pfnDrawCharacter)				(int x, int y, int number, int r, int g, int b);
	int					(*pfnDrawConsoleString)			(int x, int y, char* string);
	void				(*pfnDrawSetTextColor)			(float r, float g, float b);
	void				(*pfnDrawConsoleStringLen)		(const char* string, int* length, int* height);

	void				(*pfnConsolePrint)				(const char* string);
	void				(*pfnCenterPrint)				(const char* string);


	// Added for user input processing
	int					(*GetWindowCenterX)				(void);
	int					(*GetWindowCenterY)				(void);
	void				(*GetViewAngles)				(float*);
	void				(*SetViewAngles)				(float*);
	int					(*GetMaxClients)				(void);
	void				(*Cvar_SetValue)				(const char* cvar, float value);

	int					(*Cmd_Argc)						(void);
	char*				(*Cmd_Argv)						(int arg);
	void				(*Con_Printf)					(const char* fmt, ...);
	void				(*Con_DPrintf)					(const char* fmt, ...);
	void				(*Con_NPrintf)					(int pos, const char* fmt, ...);
	void				(*Con_NXPrintf)					(struct con_nprint_s* info, const char* fmt, ...);

	const char*			(*PhysInfo_ValueForKey)			(const char* key);
	const char*			(*ServerInfo_ValueForKey)		(const char* key);
	float				(*GetClientMaxspeed)			(void);
	int					(*CheckParm)					(char* parm, char** ppnext);

	void				(*Key_Event)					(int key, int down);
	void				(*GetMousePosition)				(int* mx, int* my);
	int					(*IsNoClipping)					(void);

	struct cl_entity_s* (*GetLocalPlayer)				(void);
	struct cl_entity_s* (*GetViewModel)					(void);
	struct cl_entity_s* (*GetEntityByIndex)				(int idx);

	float				(*GetClientTime)				(void);
	void				(*V_CalcShake)					(void);
	void				(*V_ApplyShake)					(float* origin, float* angles, float factor);

	int					(*PM_PointContents)				(float* point, int* truecontents);
	int					(*PM_WaterEntity)				(float* p);
	struct pmtrace_s*	(*PM_TraceLine)					(float* start, float* end, int flags, int usehull, int ignore_pe);

	struct model_s*		(*CL_LoadModel)					(const char* modelname, int* index);
	int					(*CL_CreateVisibleEntity)		(int type, struct cl_entity_s* ent);

	const struct model_s* (*GetSpritePointer)			(HSPRITE hSprite);
	void				(*pfnPlaySoundByNameAtLocation)	(char* szSound, float volume, float* origin);

	unsigned short		(*pfnPrecacheEvent)				(int type, const char* psz);
	void				(*pfnPlaybackEvent)				(int flags, const struct edict_s* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
	void				(*pfnWeaponAnim)				(int iAnim, int body);
	float				(*pfnRandomFloat)				(float flLow, float flHigh);
	int					(*pfnRandomLong)				(int lLow, int lHigh);
	void				(*pfnHookEvent)					(char* name, void (*pfnEvent)(struct event_args_s* args));
	int					(*Con_IsVisible)				(void);
	const char*			(*pfnGetGameDirectory)			(void);
	struct cvar_s*		(*pfnGetCvarPointer)			(const char* szName);
	const char*			(*Key_LookupBinding)			(const char* pBinding);
	const char*			(*pfnGetLevelName)				(void);
	void				(*pfnGetScreenFade)				(struct screenfade_s* fade);
	void				(*pfnSetScreenFade)				(struct screenfade_s* fade);
	void*				(*VGui_GetPanel)				(void);
	void				(*VGui_ViewportPaintBackground)	(int extents[4]);

	byte*				(*COM_LoadFile)					(char* path, int usehunk, int* pLength);
	char*				(*COM_ParseFile)				(char* data, char* token);
	void				(*COM_FreeFile)					(void* buffer);

	struct triangleapi_s* pTriAPI;
	struct efx_api_s* pEfxAPI;
	struct event_api_s* pEventAPI;
	struct demo_api_s* pDemoAPI;
	struct net_api_s* pNetAPI;
	struct IVoiceTweak_s* pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int					(*IsSpectateOnly)				(void);
	struct model_s*		(*LoadMapSprite)				(const char* filename);

	// file search functions
	void				(*COM_AddAppDirectoryToSearchPath) (const char* pszBaseDir, const char* appName);
	int					(*COM_ExpandFilename)			(const char* fileName, char* nameOutBuffer, int nameOutBufferSize);

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char*			(*PlayerInfo_ValueForKey)		(int playerNum, const char* key);
	void				(*PlayerInfo_SetValueForKey)	(const char* key, const char* value);

	// Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean			(*GetPlayerUniqueID)			(int iPlayer, char playerID[16]);

	// TrackerID access
	int					(*GetTrackerIDForPlayer)		(int playerSlot);
	int					(*GetPlayerForTrackerID)		(int trackerID);

	// Same as pfnServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int					(*pfnServerCmdUnreliable)		(char* szCmdString);

	void				(*pfnGetMousePos)				(struct tagPOINT* ppt);
	void				(*pfnSetMousePos)				(int x, int y);
	void				(*pfnSetMouseEnable)			(qboolean fEnable);

	// undocumented interface starts here
	struct cvar_s*		(*pfnGetCvarList)				(void);
	struct cmd_s*		(*pfnGetCmdList)				(void);

	char*				(*pfnGetCvarName)				(struct cvar_s* cvar);
	char*				(*pfnGetCmdName)				(struct cmd_s* cmd);

	float				(*pfnGetClientOldTime)			(void);
	float				(*pfnGetGravity)				(void);
	struct model_s*		(*pfnGetModelByIndex)			(int index);
	void				(*pfnSetFilterMode)				(int mode); // same as gl_texsort in original Quake
	void				(*pfnSetFilterColor)			(float red, float green, float blue);
	void				(*pfnSetFilterBrightness)		(float brightness);

	//this will always fail with the current engine
	void*				(*pfnSequenceGet)				(const char* fileName, const char* entryName);

	void				(*pfnSPR_DrawGeneric)			(int frame, int x, int y, const wrect_t* prc, int blendsrc, int blenddst, int width, int height);

	//this will always fail with engine, don't call
	//it actually has paramenters but i dunno what they do
	void*				(*pfnSequencePickSentence)		(const char* groupName, int pickMethod, int* entryPicked);

	//localizes hud string, uses Legacy font from skin def
	// also supports unicode strings
	int					(*pfnDrawString)				(int x, int y, const char* str, int r, int g, int b);

	//i can't get this to work for some reason, don't use this
	int					(*pfnDrawStringReverse)			(int x, int y, const char* str, int r, int g, int b);

	//gets keyvalue for local player, useful for querying vgui menus or autohelp
	const char*			(*LocalPlayerInfo_ValueForKey)	(const char* key);

	//another vgui2 text drawing function, i dunno how it works
	//it doesn't localize though
	int					(*pfnVGUI2DrawCharacter)		(int x, int y, int ch, unsigned int font);
	int					(*pfnVGUI2DrawCharacterAdditive) (int x, int y, int ch, int r, int g, int b, unsigned int font);

	//checks sound header of a sound file, determines if its a supported type
	unsigned int		(*pfnGetApproxWavePlayLen)		(char* filename);

	//for condition zero, returns interface from GameUI
	void*				(*GetCareerGameUI)				(void);	// g-cont. !!!! potential crash-point!

	void				(*Cvar_Set)						(char* name, char* value);

	//this actually checks for if the CareerGameInterface is found
	//and if a server is being run
	int					(*pfnIsPlayingCareerMatch)		(void);

	void				(*pfnPlaySoundVoiceByName)		(char* szSound, float volume, int pitch);

	void				(*pfnPrimeMusicStream)			(char* filename, int looping);

	//get the systems current time as a float
	double				(*pfnSys_FloatTime)				(void);

	void				(*pfnProcessTutorMessageDecayBuffer) (int* buffer, int buflen);
	void				(*pfnConstructTutorMessageDecayBuffer) (int* buffer, int buflen);
	void				(*pfnResetTutorMessageDecayData) (void);

	void				(*pfnPlaySoundByNameAtPitch)	(char* szSound, float volume, int pitch);

	void				(*pfnFillRGBABlend)				(int x, int y, int width, int height, int r, int g, int b, int a);
	int					(*pfnGetAppID)					(void);
	cmdalias_t*			(*pfnGetAliases)				(void);
	void				(*pfnVguiWrap2_GetMouseDelta)	(int* x, int* y);
} cl_enginefunc_t;

typedef struct cl_clientfuncs_s
{
	int					(*Initialize)					(struct cl_enginefuncs_s*, int iVersion);
	void				(*HUD_Init)						(void);
	int					(*HUD_VidInit)					(void);
	int					(*HUD_Redraw)					(float, int);
	int					(*HUD_UpdateClientData)			(struct client_data_t* pcldata, float flTime);
	void				(*HUD_Reset)					(void);
	void				(*HUD_PlayerMove)				(struct playermove_s* ppmove, qboolean server);
	void				(*HUD_PlayerMoveInit)			(struct playermove_s* ppmove);
	char				(*HUD_PlayerMoveTexture)		(char* name);
	void				(*IN_ActivateMouse)				(void);
	void				(*IN_DeactivateMouse)			(void);
	void				(*IN_MouseEvent)				(int mstate);
	void				(*IN_ClearStates)				(void);
	void				(*IN_Accumulate)				(void);
	void				(*CL_CreateMove)				(float frametime, struct usercmd_s* cmd, int active);
	int					(*CL_IsThirdPerson)				(void);
	void				(*CL_CameraOffset)				(float* ofs);
	struct kbutton_s*	(*KB_Find)						(const char* name);
	void				(*CAM_Think)					(void);
	void				(*V_CalcRefdef)					(struct ref_params_s* pparams);
	int					(*HUD_AddEntity)				(int type, struct cl_entity_s* ent, const char* modelname);
	void				(*HUD_CreateEntities)			(void);
	void				(*HUD_DrawNormalTriangles)		(void);
	void				(*HUD_DrawTransparentTriangles)	(void);
	void				(*HUD_StudioEvent)				(const struct mstudioevent_s* event, const struct cl_entity_s* entity);
	void				(*HUD_PostRunCmd)				(struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
	void				(*HUD_Shutdown)					(void);
	void				(*HUD_TxferLocalOverrides)		(struct entity_state_s* state, const struct clientdata_s* client);
	void				(*HUD_ProcessPlayerState)		(struct entity_state_s* dst, const struct entity_state_s* src);
	void				(*HUD_TxferPredictionData)		(struct entity_state_s* ps, const struct entity_state_s* pps, struct clientdata_s* pcd, const struct clientdata_s* ppcd, struct weapon_data_s* wd, const struct weapon_data_s* pwd);
	void				(*Demo_ReadBuffer)				(int size, unsigned char* buffer);
	int					(*HUD_ConnectionlessPacket)		(const struct netadr_s* net_from_, const char* args, char* response_buffer, int* response_buffer_size);
	int					(*HUD_GetHullBounds)			(int hullnumber, float* mins, float* maxs);
	void				(*HUD_Frame)					(double);
	int					(*HUD_Key_Event)				(int eventcode, int keynum, const char* pszCurrentBinding);
	void				(*HUD_TempEntUpdate)			(double frametime, double client_time, double cl_gravity, struct tempent_s** ppTempEntFree, struct tempent_s** ppTempEntActive, int(*Callback_AddVisibleEntity)(struct cl_entity_s* pEntity), void(*Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp));
	struct cl_entity_s* (*HUD_GetUserEntity)			(int index);
	void				(*HUD_VoiceStatus)				(int entindex, qboolean bTalking);
	void				(*HUD_DirectorMessage)			(int iSize, void* pbuf);
	int					(*HUD_GetStudioModelInterface)	(int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio);
	void				(*HUD_ChatInputPosition)		(int* x, int* y);
	int					(*HUD_GetPlayerTeam)			(int iplayer);
	void*				(*ClientFactory)				(); // this should be CreateInterfaceFn but that means including interface.h
															// which is a C++ file and some of the client files a C only...
															// so we return a void * which we then do a typecast on later.
} cl_clientfunc_t;

typedef struct
{
	double				dummy;

	double				m_clTime;			// Client clock
	double				m_clOldTime;		// Old Client clock
	qboolean			m_fDoInterp;		// Do interpolation?
	qboolean			m_fGaitEstimation;	// Do gait estimation?
	int					m_nFrameCount; // Current render frame #

	// Cvars that studio model code needs to reference
	cvar_t*				m_pCvarHiModels;				// Use high quality models?
	cvar_t*				m_pCvarDeveloper;				// Developer debug output desired?
	cvar_t*				m_pCvarDrawEntities; // Draw entities bone hit boxes, etc?

	cl_entity_t*		m_pCurrentEntity;		// The entity which we are currently rendering.
	model_t*			m_pRenderModel;			// The model for the entity being rendered
	player_info_t*		m_pPlayerInfo;			// Player info for current player, if drawing a player

	int					m_nPlayerIndex;			// The index of the player being drawn
	float				m_flGaitMovement;		// The player's gait movement

	studiohdr_t*		m_pStudioHeader;	// Pointer to header block for studio model data
	mstudiobodyparts_t* m_pBodyPart;		// Pointers to current body part and submodel
	mstudiomodel_t*		m_pSubModel;

	// Palette substition for top and bottom of model
	int					m_nTopColor;
	int					m_nBottomColor;

	model_t*			m_pChromeSprite; // Sprite model used for drawing studio model chrome

	// Caching
	int					m_nCachedBones;							// Number of bones in bone cache
	char				m_nCachedBoneNames[MAXSTUDIOBONES][32];	// Names of cached bones

	// Cached bone & light transformation matrices
	float				m_rgCachedBoneTransform[MAXSTUDIOBONES][3][4];
	float				m_rgCachedLightTransform[MAXSTUDIOBONES][3][4];

	// Software renderer scale factors
	float				m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	Vector				m_vUp;
	Vector				m_vRight;
	Vector				m_vNormal;

	Vector				m_vRenderOrigin;

	// Model render counters(from engine)
	int*				m_pStudioModelCount;
	int*				m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float				(*m_protationmatrix)[3][4]; // Model to world transformation
	float				(*m_paliastransform)[3][4]; // Model to view transformation

	// Concatenated bone and light transforms
	float				(*m_pbonetransform)[MAXSTUDIOBONES][3][4];
	float				(*m_plighttransform)[MAXSTUDIOBONES][3][4];
} StudioModelRenderer_td, *StudioModelRenderer_d;

typedef struct StudioModelRenderer_s 
{
	// Construction/Destruction
	void				(*CStudioModelRenderer)			(void);

	// Initialization
	void				(*Init)							(void);

	// Public Interfaces
	int					(*StudioDrawModel)				(int flags);
	int					(*StudioDrawPlayer)				(int flags, entity_state_s* pplayer);

	// Look up animation data for sequence
	mstudioanim_t*		(*StudioGetAnim)				(model_t* m_pSubModel, mstudioseqdesc_t* pseqdesc);

	// Interpolate model position and angles and set up matrices
	void				(*StudioSetUpTransform)			(int trivial_accept);

	// Set up model bone positions
	void				(*StudioSetupBones)				(void);

	// Find final attachment points
	void				(*StudioCalcAttachments)		(void);

	// Save bone matrices and names
	void				(*StudioSaveBones)				(void);

	// Merge cached bones with current bones for model
	void				(*StudioMergeBones)				(model_t* m_pSubModel);

	// Determine interpolation fraction
	float				(*StudioEstimateInterpolant)	(void);

	// Determine current frame for rendering
	float				(*StudioEstimateFrame)			(mstudioseqdesc_t* pseqdesc);

	// Apply special effects to transform matrix
	void				(*StudioFxTransform)			(cl_entity_t* ent, float transform[3][4]);

	// Spherical interpolation of bones
	void				(*StudioSlerpBones)				(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s);

	// Compute bone adjustments(bone controllers)
	void				(*StudioCalcBoneAdj)			(float dadt, float* adj, const byte* pcontroller1, const byte* pcontroller2, byte mouthopen);

	// Get bone quaternions
	void				(*StudioCalcBoneQuaterion)		(int frame, float s, mstudiobone_t* pbone, mstudioanim_t* panim, float* adj, float* q);

	// Get bone positions
	void				(*StudioCalcBonePosition)		(int frame, float s, mstudiobone_t* pbone, mstudioanim_t* panim, float* adj, float* pos);

	// Compute rotations
	void				(*StudioCalcRotations)			(float pos[][3], vec4_t* q, mstudioseqdesc_t* pseqdesc, mstudioanim_t* panim, float f);

	// Send bones and verts to renderer
	void				(*StudioRenderModel)			(void);

	// Finalize rendering
	void				(*StudioRenderFinal)			(void);

	// GL&D3D vs. Software renderer finishing functions
	void				(*StudioRenderFinal_Software)	(void);
	void				(*StudioRenderFinal_Hardware)	(void);

	// Player specific data
	// Determine pitch and blending amounts for players
	void				(*StudioPlayerBlend)			(mstudioseqdesc_t* pseqdesc, int* pBlend, float* pPitch);

	// Estimate gait frame for player
	void				(*StudioEstimateGait)			(entity_state_t* pplayer);

	// Process movement of player
	void				(*StudioProcessGait)			(entity_state_t* pplayer);

	int					(*_StudioDrawPlayer)			(int flags, entity_state_s* pplayer);

	int					(*CalculateYawBlend)			(entity_state_s* pplayer);

	void				(*CalculatePitchBlend)			(entity_state_t* pplayer);
} StudioModelRenderer_t;

extern char PM_FindTextureType(char* name);
extern char GetTextureType(pmtrace_t* ptr, Vector vecSrc, Vector vecEnd);
extern void PM_InitTextureTypes(struct playermove_s* ppmove);

struct keyname_t
{
	const char* name;
	int keynum;
};

static keyname_t keynames_ascii[] =
{
	{"TAB",			K_TAB},			{"ENTER",		K_ENTER},		{"ESCAPE",		K_ESCAPE},
	{"SPACE",		K_SPACE},		{"BACKSPACE",	K_BACKSPACE},	{"UPARROW",		K_UPARROW},
	{"DOWNARROW",	K_DOWNARROW},	{"LEFTARROW",	K_LEFTARROW},	{"RIGHTARROW",	K_RIGHTARROW},
	{"ALT",			K_ALT},			{"CTRL",		K_CTRL},		{"SHIFT",		K_SHIFT},
	{"F1",			K_F1},			{"F2",			K_F2},			{"F3",			K_F3},
	{"F4",			K_F4},			{"F5",			K_F5},			{"F6",			K_F6},
	{"F7",			K_F7},			{"F8",			K_F8},			{"F9",			K_F9},
	{"F10",			K_F10},			{"F11",			K_F11},			{"F12",			K_F12},
	{"INS",			K_INS},			{"DEL",			K_DEL},			{"PGDN",		K_PGDN},
	{"PGUP",		K_PGUP},		{"HOME",		K_HOME},		{"END",			K_END},
	{"MOUSE1",		K_MOUSE1},		{"MOUSE2",		K_MOUSE2},		{"MOUSE3",		K_MOUSE3},
	{"MOUSE4",		K_MOUSE4},		{"MOUSE5",		K_MOUSE5},		{"JOY1",		K_JOY1},
	{"JOY2",		K_JOY2},		{"JOY3",		K_JOY3},		{"JOY4",		K_JOY4},
	{"AUX1",		K_AUX1},		{"AUX2",		K_AUX2},		{"AUX3",		K_AUX3},
	{"AUX4",		K_AUX4},		{"AUX5",		K_AUX5},		{"AUX6",		K_AUX6},
	{"AUX7",		K_AUX7},		{"AUX8",		K_AUX8},		{"AUX9",		K_AUX9},
	{"AUX10",		K_AUX10},		{"AUX11",		K_AUX11},		{"AUX12",		K_AUX12},
	{"AUX13",		K_AUX13},		{"AUX14",		K_AUX14},		{"AUX15",		K_AUX15},
	{"AUX16",		K_AUX16},		{"AUX17",		K_AUX17},		{"AUX18",		K_AUX18},
	{"AUX19",		K_AUX19},		{"AUX20",		K_AUX20},		{"AUX21",		K_AUX21},
	{"AUX22",		K_AUX22},		{"AUX23",		K_AUX23},		{"AUX24",		K_AUX24},
	{"AUX25",		K_AUX25},		{"AUX26",		K_AUX26},		{"AUX27",		K_AUX27},
	{"AUX28",		K_AUX28},		{"AUX29",		K_AUX29},		{"AUX30",		K_AUX30},
	{"AUX31",		K_AUX31},		{"AUX32",		K_AUX32},		{"KP_HOME",		K_KP_HOME},
	{"KP_UP",		K_KP_UPARROW},	{"KP_PGUP",		K_KP_PGUP},		{"KP_LEFT",		K_KP_LEFTARROW},
	{"KP_5",		K_KP_5},		{"KP_PGDN",		K_KP_PGDN},		{"KP_RIGHT",	K_KP_RIGHTARROW},
	{"KP_END",		K_KP_END},		{"KP_ENTER",	K_KP_ENTER},	{"KP_DOWN",		K_KP_DOWNARROW},
	{"KP_INS",		K_KP_INS},		{"KP_DEL",		K_KP_DEL},		{"KP_SLASH",	K_KP_SLASH},
	{"KP_MINUS",	K_KP_MINUS},	{"KP_PLUS",		K_KP_PLUS},		{"CAPSLOCK",	K_CAPSLOCK},
	{"MWHEELUP",	K_MWHEELUP},	{"MWHEELDOWN",	K_MWHEELDOWN},	{"PAUSE",		K_PAUSE},
	{NULL,0}
};

enum SequencesList
{
	SEQUENCE_IDLE,
	SEQUENCE_SHOOT = 1 << 0,
	SEQUENCE_RELOAD = 1 << 1,
	SEQUENCE_DIE = 1 << 2,
	SEQUENCE_THROW = 1 << 3,
	SEQUENCE_ARM_C4 = 1 << 4,
	SEQUENCE_SHIELD = 1 << 5,
	SEQUENCE_SHIELD_SIDE = 1 << 6
};

static int Cstrike_SequenceInfo[] =
{
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 0..9   
	0,	1,	2,	0,	1,	2,	0,	1,	2,	0,	// 10..19 
	1,	2,	0,	1,	1,	2,	0,	1,	1,	2,	// 20..29 
	0,	1,	2,	0,	1,	2,	0,	1,	2,	0,	// 30..39 
	1,	2,	0,	1,	2,	0,	1,	2,	0,	1,	// 40..49 
	2,	0,	1,	2,	0,	0,	0,	8,	0,	8,	// 50..59 
	0, 16,  0, 16,  0,  0,  1,  1,  2,  0,	// 60..69 
	1,	1,	2,	0,	1,	0,	1,	0,	1,	2,	// 70..79 
	0,  1,  2, 32, 40, 32, 40, 32,  3, 32,	// 80..89
	33, 64, 33, 34, 64, 65, 34, 32, 32, 4,	// 90..99
	4,	4,	4,	4,	4,	4,	4,	4,	4,	4,	// 100..109
	4										// 110
};