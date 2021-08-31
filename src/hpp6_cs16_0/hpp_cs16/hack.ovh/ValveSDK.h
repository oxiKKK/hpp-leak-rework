#include "ValveSDK/common/const.h"
#include "ValveSDK/common/in_buttons.h"
#include "ValveSDK/common/cl_entity.h"
#include "ValveSDK/common/ref_params.h"
#include "ValveSDK/common/net_api.h"
#include "ValveSDK/common/event_api.h"
#include "ValveSDK/common/Sequence.h"
#include "ValveSDK/common/vmodes.h"
#include "ValveSDK/common/screenfade.h"
#include "ValveSDK/common/dlight.h"
#include "ValveSDK/common/r_studioint.h"
#include "ValveSDK/common/cvardef.h"
#include "ValveSDK/common/entity_types.h"
#include "ValveSDK/common/qlimits.h"
#include "ValveSDK/common/triangleapi.h"
#include "ValveSDK/common/r_efx.h"

#include "ValveSDK/engine/archtypes.h"
#include "ValveSDK/engine/studio.h"
#include "ValveSDK/engine/model.h"
#include "ValveSDK/engine/cdll_int.h"
#include "ValveSDK/engine/sound.h"
#include "ValveSDK/engine/event.h"
#include "ValveSDK/engine/delta_packet.h"
#include "ValveSDK/engine/consistency.h"
#include "ValveSDK/engine/client.h"

#include "ValveSDK/pm_shared/pm_shared.h"
#include "ValveSDK/pm_shared/pm_materials.h"
#include "ValveSDK/pm_shared/pm_defs.h"
#include "ValveSDK/pm_shared/pm_movevars.h"

#include "ValveSDK/public/keydefs.h"

#include "ValveSDK/dlls/activity.h"
#include "ValveSDK/dlls/player.h"
#include "ValveSDK/dlls/cdll_dll.h"

#include "ValveSDK/common/parsemsg.h"

#define CREATEINTERFACE_PROCNAME	"CreateInterface"
// All interfaces derive from this.
class IBaseInterface
{
public:
	virtual			~IBaseInterface() {}
};

typedef IBaseInterface* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
typedef IBaseInterface* (*InstantiateInterfaceFn)();

void *CaptureInterface(CreateInterfaceFn Interface, const char* InterfaceName);
CreateInterfaceFn CaptureFactory(const char* FactoryModule);

#include "ValveSDK/VGUI/VGUI_Frame.h"
#include "ValveSDK/VGUI/VGUI_Panel.h"
#include "ValveSDK/VGUI2/IHTML.h"
#include "ValveSDK/VGUI2/IPanel.h"
#include "ValveSDK/VGUI2/ISurface.h"

#include "ValveSDK/GameUI/IVGuiModuleLoader.h"
#include "ValveSDK/GameUI/IRunGameEngine.h"
#include "ValveSDK/GameUI/IGameUI.h"
#include "ValveSDK/GameUI/IGameConsole.h"

#pragma comment(lib, "vgui.lib")

typedef float TransformMatrix[MAXSTUDIOBONES][3][4];

#define MAX_TEAM_NAME 16

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
	char teamname[MAX_TEAM_NAME];
	bool dead;
	float showhealth;
	int health;
	char location[32];
};

typedef float vec4_t[4];
typedef struct
{
	double			dummy;

	double   m_clTime;			// Client clock
	double   m_clOldTime;		// Old Client clock
	qboolean m_fDoInterp;		// Do interpolation?
	qboolean m_fGaitEstimation;	// Do gait estimation?
	int m_nFrameCount; // Current render frame #

	// Cvars that studio model code needs to reference
	cvar_t *m_pCvarHiModels;				// Use high quality models?
	cvar_t *m_pCvarDeveloper;				// Developer debug output desired?
	cvar_t *m_pCvarDrawEntities; // Draw entities bone hit boxes, etc?

	cl_entity_t   *m_pCurrentEntity;		// The entity which we are currently rendering.
	model_t       *m_pRenderModel;			// The model for the entity being rendered
	player_info_t *m_pPlayerInfo;			// Player info for current player, if drawing a player

	int            m_nPlayerIndex;			// The index of the player being drawn
	float          m_flGaitMovement;		// The player's gait movement

	studiohdr_t        *m_pStudioHeader;	// Pointer to header block for studio model data
	mstudiobodyparts_t *m_pBodyPart;		// Pointers to current body part and submodel
	mstudiomodel_t *m_pSubModel;

	// Palette substition for top and bottom of model
	int m_nTopColor;
	int m_nBottomColor;

	model_t *m_pChromeSprite; // Sprite model used for drawing studio model chrome

	// Caching
	int  m_nCachedBones;							// Number of bones in bone cache
	char m_nCachedBoneNames[MAXSTUDIOBONES][32];	// Names of cached bones

	// Cached bone & light transformation matrices
	float m_rgCachedBoneTransform[MAXSTUDIOBONES][3][4];
	float m_rgCachedLightTransform[MAXSTUDIOBONES][3][4];

	// Software renderer scale factors
	float m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	float m_vUp[3];
	float m_vRight[3];
	float m_vNormal[3];

	float m_vRenderOrigin[3];

	// Model render counters(from engine)
	int *m_pStudioModelCount;
	int *m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float(*m_protationmatrix)[3][4]; // Model to world transformation
	float(*m_paliastransform)[3][4]; // Model to view transformation

	// Concatenated bone and light transforms
	float(*m_pbonetransform)[MAXSTUDIOBONES][3][4];
	float(*m_plighttransform)[MAXSTUDIOBONES][3][4];
} *StudioModelRenderer_d;

typedef struct StudioModelRenderer_s {
	// Construction/Destruction
	void(*CStudioModelRenderer)();

	// Initialization
	void(*Init)();

	// Public Interfaces
	int(*StudioDrawModel)(int flags);
	int(*StudioDrawPlayer)(int flags, entity_state_s * pplayer);

	// Look up animation data for sequence
	mstudioanim_t(*StudioGetAnim)(model_t * m_pSubModel, mstudioseqdesc_t * pseqdesc);

	// Interpolate model position and angles and set up matrices
	void(*StudioSetUpTransform)(int trivial_accept);

	// Set up model bone positions
	void(*StudioSetupBones)();

	// Find final attachment points
	void(*StudioCalcAttachments)();

	// Save bone matrices and names
	void(*StudioSaveBones)();

	// Merge cached bones with current bones for model
	void(*StudioMergeBones)(model_t * m_pSubModel);

	// Determine interpolation fraction
	float(*StudioEstimateInterpolant)();

	// Determine current frame for rendering
	float(*StudioEstimateFrame)(mstudioseqdesc_t * pseqdesc);

	// Apply special effects to transform matrix
	void(*StudioFxTransform)(cl_entity_t * ent, float transform[3][4]);

	// Spherical interpolation of bones
	void(*StudioSlerpBones)(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s);

	// Compute bone adjustments(bone controllers)
	void(*StudioCalcBoneAdj)(float dadt, float * adj, const byte * pcontroller1, const byte * pcontroller2, byte mouthopen);

	// Get bone quaternions
	void(*StudioCalcBoneQuaterion)(int frame, float s, mstudiobone_t * pbone, mstudioanim_t * panim, float * adj, float * q);

	// Get bone positions
	void(*StudioCalcBonePosition)(int frame, float s, mstudiobone_t * pbone, mstudioanim_t * panim, float * adj, float * pos);

	// Compute rotations
	void(*StudioCalcRotations)(float pos[][3], vec4_t * q, mstudioseqdesc_t * pseqdesc, mstudioanim_t * panim, float f);

	// Send bones and verts to renderer
	void(*StudioRenderModel)();

	// Finalize rendering
	void(*StudioRenderFinal)();

	// GL&D3D vs. Software renderer finishing functions
	void(*StudioRenderFinal_Software)();
	void(*StudioRenderFinal_Hardware)();

	// Player specific data
	// Determine pitch and blending amounts for players
	void(*StudioPlayerBlend)(mstudioseqdesc_t * pseqdesc, int * pBlend, float * pPitch);

	// Estimate gait frame for player
	void(*StudioEstimateGait)(entity_state_t * pplayer);

	// Process movement of player
	void(*StudioProcessGait)(entity_state_t * pplayer);
} StudioModelRenderer_t, *pStudioModelRenderer_t;

enum CursorCode
{
	dc_user,
	dc_none,
	dc_arrow,
	dc_ibeam,
	dc_hourglass,
	dc_waitarrow,
	dc_crosshair,
	dc_up,
	dc_sizenwse,
	dc_sizenesw,
	dc_sizewe,
	dc_sizens,
	dc_sizeall,
	dc_no,
	dc_hand,
	dc_blank, // don't show any custom vgui cursor, just let windows do it stuff (for HTML widget)
	dc_last,
};

typedef struct
{
	Vector origin;
	char *name;
	DWORD timestamp;
	int id;
} my_sound_t;

typedef void(*xcommand_t)(void);
typedef struct cmd_s
{
	struct cmd_s *next;
	const char *name;
	xcommand_t function;
	int flags;
} cmd_t, *pcmd_t;

typedef struct _UserMsg
{
	int iMsg;
	int iSize;
	char szName[16];
	struct _UserMsg* next;
	pfnUserMsgHook pfn;
} *UserMsg;

typedef struct cl_enginefuncs_s
{
	// sprite handlers
	HSPRITE(*pfnSPR_Load)			(const char *szPicName);
	int(*pfnSPR_Frames)			(HSPRITE hPic);
	int(*pfnSPR_Height)			(HSPRITE hPic, int frame);
	int(*pfnSPR_Width)			(HSPRITE hPic, int frame);
	void(*pfnSPR_Set)				(HSPRITE hPic, int r, int g, int b);
	void(*pfnSPR_Draw)			(int frame, int x, int y, const wrect_t *prc);
	void(*pfnSPR_DrawHoles)		(int frame, int x, int y, const wrect_t *prc);
	void(*pfnSPR_DrawAdditive)	(int frame, int x, int y, const wrect_t *prc);
	void(*pfnSPR_EnableScissor)	(int x, int y, int width, int height);
	void(*pfnSPR_DisableScissor)	(void);
	client_sprite_t				*(*pfnSPR_GetList)			(char *psz, int *piCount);

	// screen handlers
	void(*pfnFillRGBA)			(int x, int y, int width, int height, int r, int g, int b, int a);
	int(*pfnGetScreenInfo) 		(SCREENINFO *pscrinfo);
	void(*pfnSetCrosshair)		(HSPRITE hspr, wrect_t rc, int r, int g, int b);

	// cvar handlers
	struct cvar_s				*(*pfnRegisterVariable)	(char *szName, char *szValue, int flags);
	float(*pfnGetCvarFloat)		(const char *szName);
	char*						(*pfnGetCvarString)		(char *szName);

	// command handlers
	int(*pfnAddCommand)			(char *cmd_name, void(*function)(void));
	int(*pfnHookUserMsg)			(char *szMsgName, pfnUserMsgHook pfn);
	int(*pfnServerCmd)			(char *szCmdString);
	int(*pfnClientCmd)			(char *szCmdString);

	void(*pfnGetPlayerInfo)		(int ent_num, hud_player_info_t *pinfo);

	// sound handlers
	void(*pfnPlaySoundByName)		(char *szSound, float volume);
	void(*pfnPlaySoundByIndex)	(int iSound, float volume);

	// vector helpers
	void(*pfnAngleVectors)		(const float * vecAngles, float * forward, float * right, float * up);

	// text message system
	client_textmessage_t		*(*pfnTextMessageGet)		(const char *pName);
	int(*pfnDrawCharacter)		(int x, int y, int number, int r, int g, int b);
	int(*pfnDrawConsoleString)	(int x, int y, char *string);
	void(*pfnDrawSetTextColor)	(float r, float g, float b);
	void(*pfnDrawConsoleStringLen)(const char *string, int *length, int *height);

	void(*pfnConsolePrint)		(const char *string);
	void(*pfnCenterPrint)			(const char *string);


	// Added for user input processing
	int(*GetWindowCenterX)		(void);
	int(*GetWindowCenterY)		(void);
	void(*GetViewAngles)			(float *);
	void(*SetViewAngles)			(float *);
	int(*GetMaxClients)			(void);
	void(*Cvar_SetValue)			(const char *cvar, float value);

	int(*Cmd_Argc)					(void);
	char						*(*Cmd_Argv)				(int arg);
	void(*Con_Printf)				(const char *fmt, ...);
	void(*Con_DPrintf)			(const char *fmt, ...);
	void(*Con_NPrintf)			(int pos, const char *fmt, ...);
	void(*Con_NXPrintf)			(struct con_nprint_s *info, const char *fmt, ...);

	const char					*(*PhysInfo_ValueForKey)	(const char *key);
	const char					*(*ServerInfo_ValueForKey)(const char *key);
	float(*GetClientMaxspeed)		(void);
	int(*CheckParm)				(char *parm, char **ppnext);
	void(*Key_Event)				(int key, int down);
	void(*GetMousePosition)		(int *mx, int *my);
	int(*IsNoClipping)			(void);

	struct cl_entity_s			*(*GetLocalPlayer)		(void);
	struct cl_entity_s			*(*GetViewModel)			(void);
	struct cl_entity_s			*(*GetEntityByIndex)		(int idx);

	float(*GetClientTime)			(void);
	void(*V_CalcShake)			(void);
	void(*V_ApplyShake)			(float *origin, float *angles, float factor);

	int(*PM_PointContents)		(float *point, int *truecontents);
	int(*PM_WaterEntity)			(float *p);
	struct pmtrace_s			*(*PM_TraceLine)			(float *start, float *end, int flags, int usehull, int ignore_pe);

	struct model_s				*(*CL_LoadModel)			(const char *modelname, int *index);
	int(*CL_CreateVisibleEntity)	(int type, struct cl_entity_s *ent);

	const struct model_s *		(*GetSpritePointer)		(HSPRITE hSprite);
	void(*pfnPlaySoundByNameAtLocation)	(char *szSound, float volume, float *origin);

	unsigned short(*pfnPrecacheEvent)		(int type, const char* psz);
	void(*pfnPlaybackEvent)		(int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
	void(*pfnWeaponAnim)			(int iAnim, int body);
	float(*pfnRandomFloat)			(float flLow, float flHigh);
	long(*pfnRandomLong)			(long lLow, long lHigh);
	void(*pfnHookEvent)			(char *name, void(*pfnEvent)(struct event_args_s *args));
	int(*Con_IsVisible)			();
	const char					*(*pfnGetGameDirectory)	(void);
	struct cvar_s				*(*pfnGetCvarPointer)		(const char *szName);
	const char					*(*Key_LookupBinding)		(const char *pBinding);
	const char					*(*pfnGetLevelName)		(void);
	void(*pfnGetScreenFade)		(struct screenfade_s *fade);
	void(*pfnSetScreenFade)		(struct screenfade_s *fade);
	void                        *(*VGui_GetPanel)         ();
	void(*VGui_ViewportPaintBackground) (int extents[4]);

	byte*						(*COM_LoadFile)				(char *path, int usehunk, int *pLength);
	char*						(*COM_ParseFile)			(char *data, char *token);
	void(*COM_FreeFile)				(void *buffer);

	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int(*IsSpectateOnly) (void);
	struct model_s				*(*LoadMapSprite)			(const char *filename);

	// file search functions
	void(*COM_AddAppDirectoryToSearchPath) (const char *pszBaseDir, const char *appName);
	int(*COM_ExpandFilename)				 (const char *fileName, char *nameOutBuffer, int nameOutBufferSize);

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char					*(*PlayerInfo_ValueForKey)(int playerNum, const char *key);
	void(*PlayerInfo_SetValueForKey)(const char *key, const char *value);

	// Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);

	// TrackerID access
	int(*GetTrackerIDForPlayer)(int playerSlot);
	int(*GetPlayerForTrackerID)(int trackerID);

	// Same as pfnServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int(*pfnServerCmdUnreliable)(char *szCmdString);

	void(*pfnGetMousePos)(struct tagPOINT *ppt);
	void(*pfnSetMousePos)(int x, int y);
	void(*pfnSetMouseEnable)(qboolean fEnable);

	struct cvar_s*	(*pfnGetCvarList)(void);
	struct cmd_s* (*pfnGetCmdList)(void);

	char* (*pfnGetCvarName)(struct cvar_s* cvar);
	char* (*pfnGetCmdName)(struct cmd_s* cmd);

	float(*pfnGetServerTime)(void);
	float(*pfnGetGravity)(void);
	const struct model_s* (*pfnPrecacheSprite)(HSPRITE spr);
	void(*OverrideLightmap)(int override);
	void(*SetLightmapColor)(float r, float g, float b);
	void(*SetLightmapDarkness)(float dark);

	//this will always fail with the current engine
	int(*pfnGetSequenceByName)(int flags, const char* seq);

	void(*pfnSPR_DrawGeneric)(int frame, int x, int y, const wrect_t *prc, int blendsrc, int blenddst, int unknown1, int unknown2);

	//this will always fail with engine, don't call
	//it actually has paramenters but i dunno what they do
	void(*pfnLoadSentence)(void);

	//localizes hud string, uses Legacy font from skin def
	// also supports unicode strings
	int(*pfnDrawLocalizedHudString)(int x, int y, const char* str, int r, int g, int b);

	//i can't get this to work for some reason, don't use this
	int(*pfnDrawLocalizedConsoleString)(int x, int y, const char* str);

	//gets keyvalue for local player, useful for querying vgui menus or autohelp
	const char	*(*LocalPlayerInfo_ValueForKey)(const char* key);

	//another vgui2 text drawing function, i dunno how it works
	//it doesn't localize though
	void(*pfnDrawText_0)(int x, int y, const char* text, unsigned long font);

	int(*pfnDrawUnicodeCharacter)(int x, int y, short number, int r, int g, int b, unsigned long hfont);

	//checks sound header of a sound file, determines if its a supported type
	int(*pfnCheckSoundFile)(const char* path);

	//for condition zero, returns interface from GameUI
	void* (*GetCareerGameInterface)(void);

	void(*pfnCvar_Set)(const char* cvar, const char* value);

	//this actually checks for if the CareerGameInterface is found
	//and if a server is being run
	int(*IsSinglePlayer)(void);

	void(*pfnPlaySound)(const char* sound, float vol, float pitch);

	void(*pfnPlayMp3)(const char* mp3, int flags);

	//get the systems current time as a float
	float(*Sys_FloatTime)(void);

	void(*pfnSetArray)(int* array, int size);
	void(*pfnSetClearArray)(int* array, int size);
	void(*pfnClearArray)(void);

	void(*pfnPlaySound2)(const char* sound, float vol, float pitch);

	void(*pfnTintRGBA)			(int x, int y, int width, int height, int r, int g, int b, int a);
} cl_enginefunc_t;

typedef struct cl_clientfuncs_s
{
	int(*Initialize) (cl_enginefunc_t *pEnginefuncs, int iVersion);
	int(*HUD_Init) (void);
	int(*HUD_VidInit) (void);
	void(*HUD_Redraw) (float time, int intermission);
	int(*HUD_UpdateClientData) (client_data_t *pcldata, float flTime);
	int(*HUD_Reset) (void);
	void(*HUD_PlayerMove) (struct playermove_s *ppmove, int server);
	void(*HUD_PlayerMoveInit) (struct playermove_s *ppmove);
	char(*HUD_PlayerMoveTexture) (char *name);
	void(*IN_ActivateMouse) (void);
	void(*IN_DeactivateMouse) (void);
	void(*IN_MouseEvent) (int mstate);
	void(*IN_ClearStates) (void);
	void(*IN_Accumulate) (void);
	void(*CL_CreateMove) (float frametime, struct usercmd_s *cmd, int active);
	int(*CL_IsThirdPerson) (void);
	void(*CL_CameraOffset) (float *ofs);
	struct kbutton_s *(*KB_Find) (const char *name);
	void(*CAM_Think) (void);
	void(*V_CalcRefdef) (struct ref_params_s *pparams);
	int(*HUD_AddEntity) (int type, struct cl_entity_s *ent, const char *modelname);
	void(*HUD_CreateEntities) (void);
	void(*HUD_DrawNormalTriangles) (void);
	void(*HUD_DrawTransparentTriangles) (void);
	void(*HUD_StudioEvent) (const struct mstudioevent_s *event, const struct cl_entity_s *entity);
	void(*HUD_PostRunCmd) (struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);
	void(*HUD_Shutdown) (void);
	void(*HUD_TxferLocalOverrides) (struct entity_state_s *state, const struct clientdata_s *client);
	void(*HUD_ProcessPlayerState) (struct entity_state_s *dst, const struct entity_state_s *src);
	void(*HUD_TxferPredictionData) (struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd);
	void(*Demo_ReadBuffer) (int size, unsigned char *buffer);
	int(*HUD_ConnectionlessPacket) (struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
	int(*HUD_GetHullBounds) (int hullnumber, float *mins, float *maxs);
	void(*HUD_Frame) (double time);
	int(*HUD_Key_Event) (int down, int keynum, const char *pszCurrentBinding);
	void(*HUD_TempEntUpdate) (double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int(*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void(*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp));
	struct cl_entity_s *(*HUD_GetUserEntity) (int index);
	int(*HUD_VoiceStatus) (int entindex, qboolean bTalking);
	int(*HUD_DirectorMessage) (unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags);
	int(*HUD_GetStudioModelInterface) (int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
	void(*HUD_CHATINPUTPOSITION_FUNCTION) (int *x, int *y);
	void(*CLIENTFACTORY) (void);
	int(*HUD_GETPLAYERTEAM_FUNCTION) (int iplayer);
} cl_clientfunc_t;

void PM_InitTextureTypes(struct playermove_s *ppmove);
char GetTextureType(pmtrace_t *ptr, Vector vecSrc, Vector vecEnd);