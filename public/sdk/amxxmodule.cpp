// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
// Parts Copyright (C) 2001-2003 Will Day <willday@hpgx.net>
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// Module SDK
//

#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include "amxxmodule.h"

/************* METAMOD SUPPORT *************/
#ifdef USE_METAMOD

enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;

DLL_FUNCTIONS *g_pFunctionTable;
DLL_FUNCTIONS *g_pFunctionTable_Post;
enginefuncs_t *g_pengfuncsTable;
enginefuncs_t *g_pengfuncsTable_Post;
NEW_DLL_FUNCTIONS *g_pNewFunctionsTable;
NEW_DLL_FUNCTIONS *g_pNewFunctionsTable_Post;

// GetEntityAPI2 functions
static DLL_FUNCTIONS g_EntityAPI_Table = 
{
#ifdef FN_GameDLLInit
	FN_GameDLLInit,
#else
	nullptr,
#endif
#ifdef FN_DispatchSpawn
	FN_DispatchSpawn,
#else
	nullptr,
#endif
#ifdef FN_DispatchThink
	FN_DispatchThink,
#else
	nullptr,
#endif
#ifdef FN_DispatchUse
	FN_DispatchUse,
#else
	nullptr,
#endif
#ifdef FN_DispatchTouch
	FN_DispatchTouch,
#else
	nullptr,
#endif
#ifdef FN_DispatchBlocked
	FN_DispatchBlocked,
#else
	nullptr,
#endif
#ifdef FN_DispatchKeyValue
	FN_DispatchKeyValue,
#else
	nullptr,
#endif
#ifdef FN_DispatchSave
	FN_DispatchSave,
#else
	nullptr,
#endif
#ifdef FN_DispatchRestore
	FN_DispatchRestore,
#else
	nullptr,
#endif
#ifdef FN_DispatchObjectCollsionBox
	FN_DispatchObjectCollsionBox,
#else
	nullptr,
#endif
#ifdef FN_SaveWriteFields
	FN_SaveWriteFields,
#else
	nullptr,
#endif
#ifdef FN_SaveReadFields
	FN_SaveReadFields,
#else
	nullptr,
#endif
#ifdef FN_SaveGlobalState
	FN_SaveGlobalState,
#else
	nullptr,
#endif
#ifdef FN_RestoreGlobalState
	FN_RestoreGlobalState,
#else
	nullptr,
#endif
#ifdef FN_ResetGlobalState
	FN_ResetGlobalState,
#else
	nullptr,
#endif
#ifdef FN_ClientConnect
	FN_ClientConnect,
#else
	nullptr,
#endif
#ifdef FN_ClientDisconnect
	FN_ClientDisconnect,
#else
	nullptr,
#endif
#ifdef FN_ClientKill
	FN_ClientKill,
#else
	nullptr,
#endif
#ifdef FN_ClientPutInServer
	FN_ClientPutInServer,
#else
	nullptr,
#endif
#ifdef FN_ClientCommand
	FN_ClientCommand,
#else
	nullptr,
#endif
#ifdef FN_ClientUserInfoChanged
	FN_ClientUserInfoChanged,
#else
	nullptr,
#endif
#ifdef FN_ServerActivate
	FN_ServerActivate,
#else
	nullptr,
#endif
#ifdef FN_ServerDeactivate
	FN_ServerDeactivate,
#else
	nullptr,
#endif
#ifdef FN_PlayerPreThink
	FN_PlayerPreThink,
#else
	nullptr,
#endif
#ifdef FN_PlayerPostThink
	FN_PlayerPostThink,
#else
	nullptr,
#endif
#ifdef FN_StartFrame
	FN_StartFrame,
#else
	nullptr,
#endif
#ifdef FN_ParmsNewLevel
	FN_ParmsNewLevel,
#else
	nullptr,
#endif
#ifdef FN_ParmsChangeLevel
	FN_ParmsChangeLevel,
#else
	nullptr,
#endif
#ifdef FN_GetGameDescription
	FN_GetGameDescription,
#else
	nullptr,
#endif
#ifdef FN_PlayerCustomization
	FN_PlayerCustomization,
#else
	nullptr,
#endif
#ifdef FN_SpectatorConnect
	FN_SpectatorConnect,
#else
	nullptr,
#endif
#ifdef FN_SpectatorDisconnect
	FN_SpectatorDisconnect,
#else
	nullptr,
#endif
#ifdef FN_SpectatorThink
	FN_SpectatorThink,
#else
	nullptr,
#endif
#ifdef FN_Sys_Error
	FN_Sys_Error,
#else
	nullptr,
#endif
#ifdef FN_PM_Move
	FN_PM_Move,
#else
	nullptr,
#endif
#ifdef FN_PM_Init
	FN_PM_Init,
#else
	nullptr,
#endif
#ifdef FN_PM_FindTextureType
	FN_PM_FindTextureType,
#else
	nullptr,
#endif
#ifdef FN_SetupVisibility
	FN_SetupVisibility,
#else
	nullptr,
#endif
#ifdef FN_UpdateClientData
	FN_UpdateClientData,
#else
	nullptr,
#endif
#ifdef FN_AddToFullPack
	FN_AddToFullPack,
#else
	nullptr,
#endif
#ifdef FN_CreateBaseline
	FN_CreateBaseline,
#else
	nullptr,
#endif
#ifdef FN_RegisterEncoders
	FN_RegisterEncoders,
#else
	nullptr,
#endif
#ifdef FN_GetWeaponData
	FN_GetWeaponData,
#else
	nullptr,
#endif
#ifdef FN_CmdStart
	FN_CmdStart,
#else
	nullptr,
#endif
#ifdef FN_CmdEnd
	FN_CmdEnd,
#else
	nullptr,
#endif
#ifdef FN_ConnectionlessPacket
	FN_ConnectionlessPacket,
#else
	nullptr,
#endif
#ifdef FN_GetHullBounds
	FN_GetHullBounds,
#else
	nullptr,
#endif
#ifdef FN_CreateInstancedBaselines
	FN_CreateInstancedBaselines,
#else
	nullptr,
#endif
#ifdef FN_InconsistentFile
	FN_InconsistentFile,
#else
	nullptr,
#endif
#ifdef FN_AllowLagCompensation
	FN_AllowLagCompensation
#else
	nullptr
#endif
}; // g_EntityAPI2_Table

// GetEntityAPI2_Post functions
static DLL_FUNCTIONS g_EntityAPI_Post_Table = 
{
#ifdef FN_GameDLLInit_Post
	FN_GameDLLInit_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchSpawn_Post
	FN_DispatchSpawn_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchThink_Post
	FN_DispatchThink_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchUse_Post
	FN_DispatchUse_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchTouch_Post
	FN_DispatchTouch_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchBlocked_Post
	FN_DispatchBlocked_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchKeyValue_Post
	FN_DispatchKeyValue_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchSave_Post
	FN_DispatchSave_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchRestore_Post
	FN_DispatchRestore_Post,
#else
	nullptr,
#endif
#ifdef FN_DispatchObjectCollsionBox_Post
	FN_DispatchObjectCollsionBox_Post,
#else
	nullptr,
#endif
#ifdef FN_SaveWriteFields_Post
	FN_SaveWriteFields_Post,
#else
	nullptr,
#endif
#ifdef FN_SaveReadFields_Post
	FN_SaveReadFields_Post,
#else
	nullptr,
#endif
#ifdef FN_SaveGlobalState_Post
	FN_SaveGlobalState_Post,
#else
	nullptr,
#endif
#ifdef FN_RestoreGlobalState_Post
	FN_RestoreGlobalState_Post,
#else
	nullptr,
#endif
#ifdef FN_ResetGlobalState_Post
	FN_ResetGlobalState_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientConnect_Post
	FN_ClientConnect_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientDisconnect_Post
	FN_ClientDisconnect_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientKill_Post
	FN_ClientKill_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientPutInServer_Post
	FN_ClientPutInServer_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientCommand_Post
	FN_ClientCommand_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientUserInfoChanged_Post
	FN_ClientUserInfoChanged_Post,
#else
	nullptr,
#endif
#ifdef FN_ServerActivate_Post
	FN_ServerActivate_Post,
#else
	nullptr,
#endif
#ifdef FN_ServerDeactivate_Post
	FN_ServerDeactivate_Post,
#else
	nullptr,
#endif
#ifdef FN_PlayerPreThink_Post
	FN_PlayerPreThink_Post,
#else
	nullptr,
#endif
#ifdef FN_PlayerPostThink_Post
	FN_PlayerPostThink_Post,
#else
	nullptr,
#endif
#ifdef FN_StartFrame_Post
	FN_StartFrame_Post,
#else
	nullptr,
#endif
#ifdef FN_ParmsNewLevel_Post
	FN_ParmsNewLevel_Post,
#else
	nullptr,
#endif
#ifdef FN_ParmsChangeLevel_Post
	FN_ParmsChangeLevel_Post,
#else
	nullptr,
#endif
#ifdef FN_GetGameDescription_Post
	FN_GetGameDescription_Post,
#else
	nullptr,
#endif
#ifdef FN_PlayerCustomization_Post
	FN_PlayerCustomization_Post,
#else
	nullptr,
#endif
#ifdef FN_SpectatorConnect_Post
	FN_SpectatorConnect_Post,
#else
	nullptr,
#endif
#ifdef FN_SpectatorDisconnect_Post
	FN_SpectatorDisconnect_Post,
#else
	nullptr,
#endif
#ifdef FN_SpectatorThink_Post
	FN_SpectatorThink_Post,
#else
	nullptr,
#endif
#ifdef FN_Sys_Error_Post
	FN_Sys_Error_Post,
#else
	nullptr,
#endif
#ifdef FN_PM_Move_Post
	FN_PM_Move_Post,
#else
	nullptr,
#endif
#ifdef FN_PM_Init_Post
	FN_PM_Init_Post,
#else
	nullptr,
#endif
#ifdef FN_PM_FindTextureType_Post
	FN_PM_FindTextureType_Post,
#else
	nullptr,
#endif
#ifdef FN_SetupVisibility_Post
	FN_SetupVisibility_Post,
#else
	nullptr,
#endif
#ifdef FN_UpdateClientData_Post
	FN_UpdateClientData_Post,
#else
	nullptr,
#endif
#ifdef FN_AddToFullPack_Post
	FN_AddToFullPack_Post,
#else
	nullptr,
#endif
#ifdef FN_CreateBaseline_Post
	FN_CreateBaseline_Post,
#else
	nullptr,
#endif
#ifdef FN_RegisterEncoders_Post
	FN_RegisterEncoders_Post,
#else
	nullptr,
#endif
#ifdef FN_GetWeaponData_Post
	FN_GetWeaponData_Post,
#else
	nullptr,
#endif
#ifdef FN_CmdStart_Post
	FN_CmdStart_Post,
#else
	nullptr,
#endif
#ifdef FN_CmdEnd_Post
	FN_CmdEnd_Post,
#else
	nullptr,
#endif
#ifdef FN_ConnectionlessPacket_Post
	FN_ConnectionlessPacket_Post,
#else
	nullptr,
#endif
#ifdef FN_GetHullBounds_Post
	FN_GetHullBounds_Post,
#else
	nullptr,
#endif
#ifdef FN_CreateInstancedBaselines_Post
	FN_CreateInstancedBaselines_Post,
#else
	nullptr,
#endif
#ifdef FN_InconsistentFile_Post
	FN_InconsistentFile_Post,
#else
	nullptr,
#endif
#ifdef FN_AllowLagCompensation
	FN_AllowLagCompensation,
#else
	nullptr,
#endif
}; // g_EntityAPI2_Table

static enginefuncs_t g_EngineFuncs_Table = 
{
#ifdef FN_PrecacheModel
	FN_PrecacheModel,
#else
	nullptr,
#endif
#ifdef FN_PrecacheSound
	FN_PrecacheSound,
#else
	nullptr,
#endif
#ifdef FN_SetModel
	FN_SetModel,
#else
	nullptr,
#endif
#ifdef FN_ModelIndex
	FN_ModelIndex,
#else
	nullptr,
#endif
#ifdef FN_ModelFrames
	FN_ModelFrames,
#else
	nullptr,
#endif
#ifdef FN_SetSize
	FN_SetSize,
#else
	nullptr,
#endif
#ifdef FN_ChangeLevel
	FN_ChangeLevel,
#else
	nullptr,
#endif
#ifdef FN_GetSpawnParms
	FN_GetSpawnParms,
#else
	nullptr,
#endif
#ifdef FN_SaveSpawnParms
	FN_SaveSpawnParms,
#else
	nullptr,
#endif
#ifdef FN_VecToYaw
	FN_VecToYaw,
#else
	nullptr,
#endif
#ifdef FN_VecToAngles
	FN_VecToAngles,
#else
	nullptr,
#endif
#ifdef FN_MoveToOrigin
	FN_MoveToOrigin,
#else
	nullptr,
#endif
#ifdef FN_ChangeYaw
	FN_ChangeYaw,
#else
	nullptr,
#endif
#ifdef FN_ChangePitch
	FN_ChangePitch,
#else
	nullptr,
#endif
#ifdef FN_FindEntityByString
	FN_FindEntityByString,
#else
	nullptr,
#endif
#ifdef FN_GetEntityIllum
	FN_GetEntityIllum,
#else
	nullptr,
#endif
#ifdef FN_FindEntityInSphere
	FN_FindEntityInSphere,
#else
	nullptr,
#endif
#ifdef FN_FindClientInPVS
	FN_FindClientInPVS,
#else
	nullptr,
#endif
#ifdef FN_EntitiesInPVS
	FN_EntitiesInPVS,
#else
	nullptr,
#endif
#ifdef FN_MakeVectors
	FN_MakeVectors,
#else
	nullptr,
#endif
#ifdef FN_AngleVectors
	FN_AngleVectors,
#else
	nullptr,
#endif
#ifdef FN_CreateEntity
	FN_CreateEntity,
#else
	nullptr,
#endif
#ifdef FN_RemoveEntity
	FN_RemoveEntity,
#else
	nullptr,
#endif
#ifdef FN_CreateNamedEntity
	FN_CreateNamedEntity,
#else
	nullptr,
#endif
#ifdef FN_MakeStatic
	FN_MakeStatic,
#else
	nullptr,
#endif
#ifdef FN_EntIsOnFloor
	FN_EntIsOnFloor,
#else
	nullptr,
#endif
#ifdef FN_DropToFloor
	FN_DropToFloor,
#else
	nullptr,
#endif
#ifdef FN_WalkMove
	FN_WalkMove,
#else
	nullptr,
#endif
#ifdef FN_SetOrigin
	FN_SetOrigin,
#else
	nullptr,
#endif
#ifdef FN_EmitSound
	FN_EmitSound,
#else
	nullptr,
#endif
#ifdef FN_EmitAmbientSound
	FN_EmitAmbientSound,
#else
	nullptr,
#endif
#ifdef FN_TraceLine
	FN_TraceLine,
#else
	nullptr,
#endif
#ifdef FN_TraceToss
	FN_TraceToss,
#else
	nullptr,
#endif
#ifdef FN_TraceMonsterHull
	FN_TraceMonsterHull,
#else
	nullptr,
#endif
#ifdef FN_TraceHull
	FN_TraceHull,
#else
	nullptr,
#endif
#ifdef FN_TraceModel
	FN_TraceModel,
#else
	nullptr,
#endif
#ifdef FN_TraceTexture
	FN_TraceTexture,
#else
	nullptr,
#endif
#ifdef FN_TraceSphere
	FN_TraceSphere,
#else
	nullptr,
#endif
#ifdef FN_GetAimVector
	FN_GetAimVector,
#else
	nullptr,
#endif
#ifdef FN_ServerCommand
	FN_ServerCommand,
#else
	nullptr,
#endif
#ifdef FN_ServerExecute
	FN_ServerExecute,
#else
	nullptr,
#endif
#ifdef FN_engClientCommand
	FN_engClientCommand,
#else
	nullptr,
#endif
#ifdef FN_ParticleEffect
	FN_ParticleEffect,
#else
	nullptr,
#endif
#ifdef FN_LightStyle
	FN_LightStyle,
#else
	nullptr,
#endif
#ifdef FN_DecalIndex
	FN_DecalIndex,
#else
	nullptr,
#endif
#ifdef FN_PointContents
	FN_PointContents,
#else
	nullptr,
#endif
#ifdef FN_MessageBegin
	FN_MessageBegin,
#else
	nullptr,
#endif
#ifdef FN_MessageEnd
	FN_MessageEnd,
#else
	nullptr,
#endif
#ifdef FN_WriteByte
	FN_WriteByte,
#else
	nullptr,
#endif
#ifdef FN_WriteChar
	FN_WriteChar,
#else
	nullptr,
#endif
#ifdef FN_WriteShort
	FN_WriteShort,
#else
	nullptr,
#endif
#ifdef FN_WriteLong
	FN_WriteLong,
#else
	nullptr,
#endif
#ifdef FN_WriteAngle
	FN_WriteAngle,
#else
	nullptr,
#endif
#ifdef FN_WriteCoord
	FN_WriteCoord,
#else
	nullptr,
#endif
#ifdef FN_WriteString
	FN_WriteString,
#else
	nullptr,
#endif
#ifdef FN_WriteEntity
	FN_WriteEntity,
#else
	nullptr,
#endif
#ifdef FN_CVarRegister
	FN_CVarRegister,
#else
	nullptr,
#endif
#ifdef FN_CVarGetFloat
	FN_CVarGetFloat,
#else
	nullptr,
#endif
#ifdef FN_CVarGetString
	FN_CVarGetString,
#else
	nullptr,
#endif
#ifdef FN_CVarSetFloat
	FN_CVarSetFloat,
#else
	nullptr,
#endif
#ifdef FN_CVarSetString
	FN_CVarSetString,
#else
	nullptr,
#endif
#ifdef FN_AlertMessage
	FN_AlertMessage,
#else
	nullptr,
#endif
#ifdef FN_EngineFprintf
	FN_EngineFprintf,
#else
	nullptr,
#endif
#ifdef FN_PvAllocEntPrivateData
	FN_PvAllocEntPrivateData,
#else
	nullptr,
#endif
#ifdef FN_PvEntPrivateData
	FN_PvEntPrivateData,
#else
	nullptr,
#endif
#ifdef FN_FreeEntPrivateData
	FN_FreeEntPrivateData,
#else
	nullptr,
#endif
#ifdef FN_SzFromIndex
	FN_SzFromIndex,
#else
	nullptr,
#endif
#ifdef FN_AllocString
	FN_AllocString,
#else
	nullptr,
#endif
#ifdef FN_GetVarsOfEnt
	FN_GetVarsOfEnt,
#else
	nullptr,
#endif
#ifdef FN_PEntityOfEntOffset
	FN_PEntityOfEntOffset,
#else
	nullptr,
#endif
#ifdef FN_EntOffsetOfPEntity
	FN_EntOffsetOfPEntity,
#else
	nullptr,
#endif
#ifdef FN_IndexOfEdict
	FN_IndexOfEdict,
#else
	nullptr,
#endif
#ifdef FN_PEntityOfEntIndex
	FN_PEntityOfEntIndex,
#else
	nullptr,
#endif
#ifdef FN_FindEntityByVars
	FN_FindEntityByVars,
#else
	nullptr,
#endif
#ifdef FN_GetModelPtr
	FN_GetModelPtr,
#else
	nullptr,
#endif
#ifdef FN_RegUserMsg
	FN_RegUserMsg,
#else
	nullptr,
#endif
#ifdef FN_AnimationAutomove
	FN_AnimationAutomove,
#else
	nullptr,
#endif
#ifdef FN_GetBonePosition
	FN_GetBonePosition,
#else
	nullptr,
#endif
#ifdef FN_FunctionFromName
	FN_FunctionFromName,
#else
	nullptr,
#endif
#ifdef FN_NameForFunction
	FN_NameForFunction,
#else
	nullptr,
#endif
#ifdef FN_ClientPrintf
	FN_ClientPrintf,
#else
	nullptr,
#endif
#ifdef FN_ServerPrint
	FN_ServerPrint,
#else
	nullptr,
#endif
#ifdef FN_Cmd_Args
	FN_Cmd_Args,
#else
	nullptr,
#endif
#ifdef FN_Cmd_Argv
	FN_Cmd_Argv,
#else
	nullptr,
#endif
#ifdef FN_Cmd_Argc
	FN_Cmd_Argc,
#else
	nullptr,
#endif
#ifdef FN_GetAttachment
	FN_GetAttachment,
#else
	nullptr,
#endif
#ifdef FN_CRC32_Init
	FN_CRC32_Init,
#else
	nullptr,
#endif
#ifdef FN_CRC32_ProcessBuffer
	FN_CRC32_ProcessBuffer,
#else
	nullptr,
#endif
#ifdef FN_CRC32_ProcessByte
	FN_CRC32_ProcessByte,
#else
	nullptr,
#endif
#ifdef FN_CRC32_Final
	FN_CRC32_Final,
#else
	nullptr,
#endif
#ifdef FN_RandomLong
	FN_RandomLong,
#else
	nullptr,
#endif
#ifdef FN_RandomFloat
	FN_RandomFloat,
#else
	nullptr,
#endif
#ifdef FN_SetView
	FN_SetView,
#else
	nullptr,
#endif
#ifdef FN_Time
	FN_Time,
#else
	nullptr,
#endif
#ifdef FN_CrosshairAngle
	FN_CrosshairAngle,
#else
	nullptr,
#endif
#ifdef FN_LoadFileForMe
	FN_LoadFileForMe,
#else
	nullptr,
#endif
#ifdef FN_FreeFile
	FN_FreeFile,
#else
	nullptr,
#endif
#ifdef FN_EndSection
	FN_EndSection,
#else
	nullptr,
#endif
#ifdef FN_CompareFileTime
	FN_CompareFileTime,
#else
	nullptr,
#endif
#ifdef FN_GetGameDir
	FN_GetGameDir,
#else
	nullptr,
#endif
#ifdef FN_Cvar_RegisterVariable
	FN_Cvar_RegisterVariable,
#else
	nullptr,
#endif
#ifdef FN_FadeClientVolume
	FN_FadeClientVolume,
#else
	nullptr,
#endif
#ifdef FN_SetClientMaxspeed
	FN_SetClientMaxspeed,
#else
	nullptr,
#endif
#ifdef FN_CreateFakeClient
	FN_CreateFakeClient,
#else
	nullptr,
#endif
#ifdef FN_RunPlayerMove
	FN_RunPlayerMove,
#else
	nullptr,
#endif
#ifdef FN_NumberOfEntities
	FN_NumberOfEntities,
#else
	nullptr,
#endif
#ifdef FN_GetInfoKeyBuffer
	FN_GetInfoKeyBuffer,
#else
	nullptr,
#endif
#ifdef FN_InfoKeyValue
	FN_InfoKeyValue,
#else
	nullptr,
#endif
#ifdef FN_SetKeyValue
	FN_SetKeyValue,
#else
	nullptr,
#endif
#ifdef FN_SetClientKeyValue
	FN_SetClientKeyValue,
#else
	nullptr,
#endif
#ifdef FN_IsMapValid
	FN_IsMapValid,
#else
	nullptr,
#endif
#ifdef FN_StaticDecal
	FN_StaticDecal,
#else
	nullptr,
#endif
#ifdef FN_PrecacheGeneric
	FN_PrecacheGeneric,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerUserId
	FN_GetPlayerUserId,
#else
	nullptr,
#endif
#ifdef FN_BuildSoundMsg
	FN_BuildSoundMsg,
#else
	nullptr,
#endif
#ifdef FN_IsDedicatedServer
	FN_IsDedicatedServer,
#else
	nullptr,
#endif
#ifdef FN_CVarGetPointer
	FN_CVarGetPointer,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerWONId
	FN_GetPlayerWONId,
#else
	nullptr,
#endif
#ifdef FN_Info_RemoveKey
	FN_Info_RemoveKey,
#else
	nullptr,
#endif
#ifdef FN_GetPhysicsKeyValue
	FN_GetPhysicsKeyValue,
#else
	nullptr,
#endif
#ifdef FN_SetPhysicsKeyValue
	FN_SetPhysicsKeyValue,
#else
	nullptr,
#endif
#ifdef FN_GetPhysicsInfoString
	FN_GetPhysicsInfoString,
#else
	nullptr,
#endif
#ifdef FN_PrecacheEvent
	FN_PrecacheEvent,
#else
	nullptr,
#endif
#ifdef FN_PlaybackEvent
	FN_PlaybackEvent,
#else
	nullptr,
#endif
#ifdef FN_SetFatPVS
	FN_SetFatPVS,
#else
	nullptr,
#endif
#ifdef FN_SetFatPAS
	FN_SetFatPAS,
#else
	nullptr,
#endif
#ifdef FN_CheckVisibility
	FN_CheckVisibility,
#else
	nullptr,
#endif
#ifdef FN_DeltaSetField
	FN_DeltaSetField,
#else
	nullptr,
#endif
#ifdef FN_DeltaUnsetField
	FN_DeltaUnsetField,
#else
	nullptr,
#endif
#ifdef FN_DeltaAddEncoder
	FN_DeltaAddEncoder,
#else
	nullptr,
#endif
#ifdef FN_GetCurrentPlayer
	FN_GetCurrentPlayer,
#else
	nullptr,
#endif
#ifdef FN_CanSkipPlayer
	FN_CanSkipPlayer,
#else
	nullptr,
#endif
#ifdef FN_DeltaFindField
	FN_DeltaFindField,
#else
	nullptr,
#endif
#ifdef FN_DeltaSetFieldByIndex
	FN_DeltaSetFieldByIndex,
#else
	nullptr,
#endif
#ifdef FN_DeltaUnsetFieldByIndex
	FN_DeltaUnsetFieldByIndex,
#else
	nullptr,
#endif
#ifdef FN_SetGroupMask
	FN_SetGroupMask,
#else
	nullptr,
#endif
#ifdef FN_engCreateInstancedBaseline
	FN_engCreateInstancedBaseline,
#else
	nullptr,
#endif
#ifdef FN_Cvar_DirectSet
	FN_Cvar_DirectSet,
#else
	nullptr,
#endif
#ifdef FN_ForceUnmodified
	FN_ForceUnmodified,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerStats
	FN_GetPlayerStats,
#else
	nullptr,
#endif
#ifdef FN_AddServerCommand
	FN_AddServerCommand,
#else
	nullptr,
#endif
#ifdef FN_Voice_GetClientListening
	FN_Voice_GetClientListening,
#else
	nullptr,
#endif
#ifdef FN_Voice_SetClientListening
	FN_Voice_SetClientListening,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerAuthId
	FN_GetPlayerAuthId
#else
	nullptr
#endif
}; // g_EngineFuncs_Table


static enginefuncs_t g_EngineFuncs_Post_Table =
{
#ifdef FN_PrecacheModel_Post
	FN_PrecacheModel_Post,
#else
	nullptr,
#endif
#ifdef FN_PrecacheSound_Post
	FN_PrecacheSound_Post,
#else
	nullptr,
#endif
#ifdef FN_SetModel_Post
	FN_SetModel_Post,
#else
	nullptr,
#endif
#ifdef FN_ModelIndex_Post
	FN_ModelIndex_Post,
#else
	nullptr,
#endif
#ifdef FN_ModelFrames_Post
	FN_ModelFrames_Post,
#else
	nullptr,
#endif
#ifdef FN_SetSize_Post
	FN_SetSize_Post,
#else
	nullptr,
#endif
#ifdef FN_ChangeLevel_Post
	FN_ChangeLevel_Post,
#else
	nullptr,
#endif
#ifdef FN_GetSpawnParms_Post
	FN_GetSpawnParms_Post,
#else
	nullptr,
#endif
#ifdef FN_SaveSpawnParms_Post
	FN_SaveSpawnParms_Post,
#else
	nullptr,
#endif
#ifdef FN_VecToYaw_Post
	FN_VecToYaw_Post,
#else
	nullptr,
#endif
#ifdef FN_VecToAngles_Post
	FN_VecToAngles_Post,
#else
	nullptr,
#endif
#ifdef FN_MoveToOrigin_Post
	FN_MoveToOrigin_Post,
#else
	nullptr,
#endif
#ifdef FN_ChangeYaw_Post
	FN_ChangeYaw_Post,
#else
	nullptr,
#endif
#ifdef FN_ChangePitch_Post
	FN_ChangePitch_Post,
#else
	nullptr,
#endif
#ifdef FN_FindEntityByString_Post
	FN_FindEntityByString_Post,
#else
	nullptr,
#endif
#ifdef FN_GetEntityIllum_Post
	FN_GetEntityIllum_Post,
#else
	nullptr,
#endif
#ifdef FN_FindEntityInSphere_Post
	FN_FindEntityInSphere_Post,
#else
	nullptr,
#endif
#ifdef FN_FindClientInPVS_Post
	FN_FindClientInPVS_Post,
#else
	nullptr,
#endif
#ifdef FN_EntitiesInPVS_Post
	FN_EntitiesInPVS_Post,
#else
	nullptr,
#endif
#ifdef FN_MakeVectors_Post
	FN_MakeVectors_Post,
#else
	nullptr,
#endif
#ifdef FN_AngleVectors_Post
	FN_AngleVectors_Post,
#else
	nullptr,
#endif
#ifdef FN_CreateEntity_Post
	FN_CreateEntity_Post,
#else
	nullptr,
#endif
#ifdef FN_RemoveEntity_Post
	FN_RemoveEntity_Post,
#else
	nullptr,
#endif
#ifdef FN_CreateNamedEntity_Post
	FN_CreateNamedEntity_Post,
#else
	nullptr,
#endif
#ifdef FN_MakeStatic_Post
	FN_MakeStatic_Post,
#else
	nullptr,
#endif
#ifdef FN_EntIsOnFloor_Post
	FN_EntIsOnFloor_Post,
#else
	nullptr,
#endif
#ifdef FN_DropToFloor_Post
	FN_DropToFloor_Post,
#else
	nullptr,
#endif
#ifdef FN_WalkMove_Post
	FN_WalkMove_Post,
#else
	nullptr,
#endif
#ifdef FN_SetOrigin_Post
	FN_SetOrigin_Post,
#else
	nullptr,
#endif
#ifdef FN_EmitSound_Post
	FN_EmitSound_Post,
#else
	nullptr,
#endif
#ifdef FN_EmitAmbientSound_Post
	FN_EmitAmbientSound_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceLine_Post
	FN_TraceLine_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceToss_Post
	FN_TraceToss_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceMonsterHull_Post
	FN_TraceMonsterHull_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceHull_Post
	FN_TraceHull_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceModel_Post
	FN_TraceModel_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceTexture_Post
	FN_TraceTexture_Post,
#else
	nullptr,
#endif
#ifdef FN_TraceSphere_Post
	FN_TraceSphere_Post,
#else
	nullptr,
#endif
#ifdef FN_GetAimVector_Post
	FN_GetAimVector_Post,
#else
	nullptr,
#endif
#ifdef FN_ServerCommand_Post
	FN_ServerCommand_Post,
#else
	nullptr,
#endif
#ifdef FN_ServerExecute_Post
	FN_ServerExecute_Post,
#else
	nullptr,
#endif
#ifdef FN_engClientCommand_Post
	FN_engClientCommand_Post,
#else
	nullptr,
#endif
#ifdef FN_ParticleEffect_Post
	FN_ParticleEffect_Post,
#else
	nullptr,
#endif
#ifdef FN_LightStyle_Post
	FN_LightStyle_Post,
#else
	nullptr,
#endif
#ifdef FN_DecalIndex_Post
	FN_DecalIndex_Post,
#else
	nullptr,
#endif
#ifdef FN_PointContents_Post
	FN_PointContents_Post,
#else
	nullptr,
#endif
#ifdef FN_MessageBegin_Post
	FN_MessageBegin_Post,
#else
	nullptr,
#endif
#ifdef FN_MessageEnd_Post
	FN_MessageEnd_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteByte_Post
	FN_WriteByte_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteChar_Post
	FN_WriteChar_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteShort_Post
	FN_WriteShort_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteLong_Post
	FN_WriteLong_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteAngle_Post
	FN_WriteAngle_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteCoord_Post
	FN_WriteCoord_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteString_Post
	FN_WriteString_Post,
#else
	nullptr,
#endif
#ifdef FN_WriteEntity_Post
	FN_WriteEntity_Post,
#else
	nullptr,
#endif
#ifdef FN_CVarRegister_Post
	FN_CVarRegister_Post,
#else
	nullptr,
#endif
#ifdef FN_CVarGetFloat_Post
	FN_CVarGetFloat_Post,
#else
	nullptr,
#endif
#ifdef FN_CVarGetString_Post
	FN_CVarGetString_Post,
#else
	nullptr,
#endif
#ifdef FN_CVarSetFloat_Post
	FN_CVarSetFloat_Post,
#else
	nullptr,
#endif
#ifdef FN_CVarSetString_Post
	FN_CVarSetString_Post,
#else
	nullptr,
#endif
#ifdef FN_AlertMessage_Post
	FN_AlertMessage_Post,
#else
	nullptr,
#endif
#ifdef FN_EngineFprintf_Post
	FN_EngineFprintf_Post,
#else
	nullptr,
#endif
#ifdef FN_PvAllocEntPrivateData_Post
	FN_PvAllocEntPrivateData_Post,
#else
	nullptr,
#endif
#ifdef FN_PvEntPrivateData_Post
	FN_PvEntPrivateData_Post,
#else
	nullptr,
#endif
#ifdef FN_FreeEntPrivateData_Post
	FN_FreeEntPrivateData_Post,
#else
	nullptr,
#endif
#ifdef FN_SzFromIndex_Post
	FN_SzFromIndex_Post,
#else
	nullptr,
#endif
#ifdef FN_AllocString_Post
	FN_AllocString_Post,
#else
	nullptr,
#endif
#ifdef FN_GetVarsOfEnt_Post
	FN_GetVarsOfEnt_Post,
#else
	nullptr,
#endif
#ifdef FN_PEntityOfEntOffset_Post
	FN_PEntityOfEntOffset_Post,
#else
	nullptr,
#endif
#ifdef FN_EntOffsetOfPEntity_Post
	FN_EntOffsetOfPEntity_Post,
#else
	nullptr,
#endif
#ifdef FN_IndexOfEdict_Post
	FN_IndexOfEdict_Post,
#else
	nullptr,
#endif
#ifdef FN_PEntityOfEntIndex_Post
	FN_PEntityOfEntIndex_Post,
#else
	nullptr,
#endif
#ifdef FN_FindEntityByVars_Post
	FN_FindEntityByVars_Post,
#else
	nullptr,
#endif
#ifdef FN_GetModelPtr_Post
	FN_GetModelPtr_Post,
#else
	nullptr,
#endif
#ifdef FN_RegUserMsg_Post
	FN_RegUserMsg_Post,
#else
	nullptr,
#endif
#ifdef FN_AnimationAutomove_Post
	FN_AnimationAutomove_Post,
#else
	nullptr,
#endif
#ifdef FN_GetBonePosition_Post
	FN_GetBonePosition_Post,
#else
	nullptr,
#endif
#ifdef FN_FunctionFromName_Post
	FN_FunctionFromName_Post,
#else
	nullptr,
#endif
#ifdef FN_NameForFunction_Post
	FN_NameForFunction_Post,
#else
	nullptr,
#endif
#ifdef FN_ClientPrintf_Post
	FN_ClientPrintf_Post,
#else
	nullptr,
#endif
#ifdef FN_ServerPrint_Post
	FN_ServerPrint_Post,
#else
	nullptr,
#endif
#ifdef FN_Cmd_Args_Post
	FN_Cmd_Args_Post,
#else
	nullptr,
#endif
#ifdef FN_Cmd_Argv_Post
	FN_Cmd_Argv_Post,
#else
	nullptr,
#endif
#ifdef FN_Cmd_Argc_Post
	FN_Cmd_Argc_Post,
#else
	nullptr,
#endif
#ifdef FN_GetAttachment_Post
	FN_GetAttachment_Post,
#else
	nullptr,
#endif
#ifdef FN_CRC32_Init_Post
	FN_CRC32_Init_Post,
#else
	nullptr,
#endif
#ifdef FN_CRC32_ProcessBuffer_Post
	FN_CRC32_ProcessBuffer_Post,
#else
	nullptr,
#endif
#ifdef FN_CRC32_ProcessByte_Post
	FN_CRC32_ProcessByte_Post,
#else
	nullptr,
#endif
#ifdef FN_CRC32_Final_Post
	FN_CRC32_Final_Post,
#else
	nullptr,
#endif
#ifdef FN_RandomLong_Post
	FN_RandomLong_Post,
#else
	nullptr,
#endif
#ifdef FN_RandomFloat_Post
	FN_RandomFloat_Post,
#else
	nullptr,
#endif
#ifdef FN_SetView_Post
	FN_SetView_Post,
#else
	nullptr,
#endif
#ifdef FN_Time_Post
	FN_Time_Post,
#else
	nullptr,
#endif
#ifdef FN_CrosshairAngle_Post
	FN_CrosshairAngle_Post,
#else
	nullptr,
#endif
#ifdef FN_LoadFileForMe_Post
	FN_LoadFileForMe_Post,
#else
	nullptr,
#endif
#ifdef FN_FreeFile_Post
	FN_FreeFile_Post,
#else
	nullptr,
#endif
#ifdef FN_EndSection_Post
	FN_EndSection_Post,
#else
	nullptr,
#endif
#ifdef FN_CompareFileTime_Post
	FN_CompareFileTime_Post,
#else
	nullptr,
#endif
#ifdef FN_GetGameDir_Post
	FN_GetGameDir_Post,
#else
	nullptr,
#endif
#ifdef FN_Cvar_RegisterVariable_Post
	FN_Cvar_RegisterVariable_Post,
#else
	nullptr,
#endif
#ifdef FN_FadeClientVolume_Post
	FN_FadeClientVolume_Post,
#else
	nullptr,
#endif
#ifdef FN_SetClientMaxspeed_Post
	FN_SetClientMaxspeed_Post,
#else
	nullptr,
#endif
#ifdef FN_CreateFakeClient_Post
	FN_CreateFakeClient_Post,
#else
	nullptr,
#endif
#ifdef FN_RunPlayerMove_Post
	FN_RunPlayerMove_Post,
#else
	nullptr,
#endif
#ifdef FN_NumberOfEntities_Post
	FN_NumberOfEntities_Post,
#else
	nullptr,
#endif
#ifdef FN_GetInfoKeyBuffer_Post
	FN_GetInfoKeyBuffer_Post,
#else
	nullptr,
#endif
#ifdef FN_InfoKeyValue_Post
	FN_InfoKeyValue_Post,
#else
	nullptr,
#endif
#ifdef FN_SetKeyValue_Post
	FN_SetKeyValue_Post,
#else
	nullptr,
#endif
#ifdef FN_SetClientKeyValue_Post
	FN_SetClientKeyValue_Post,
#else
	nullptr,
#endif
#ifdef FN_IsMapValid_Post
	FN_IsMapValid_Post,
#else
	nullptr,
#endif
#ifdef FN_StaticDecal_Post
	FN_StaticDecal_Post,
#else
	nullptr,
#endif
#ifdef FN_PrecacheGeneric_Post
	FN_PrecacheGeneric_Post,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerUserId_Post
	FN_GetPlayerUserId_Post,
#else
	nullptr,
#endif
#ifdef FN_BuildSoundMsg_Post
	FN_BuildSoundMsg_Post,
#else
	nullptr,
#endif
#ifdef FN_IsDedicatedServer_Post
	FN_IsDedicatedServer_Post,
#else
	nullptr,
#endif
#ifdef FN_CVarGetPointer_Post
	FN_CVarGetPointer_Post,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerWONId_Post
	FN_GetPlayerWONId_Post,
#else
	nullptr,
#endif
#ifdef FN_Info_RemoveKey_Post
	FN_Info_RemoveKey_Post,
#else
	nullptr,
#endif
#ifdef FN_GetPhysicsKeyValue_Post
	FN_GetPhysicsKeyValue_Post,
#else
	nullptr,
#endif
#ifdef FN_SetPhysicsKeyValue_Post
	FN_SetPhysicsKeyValue_Post,
#else
	nullptr,
#endif
#ifdef FN_GetPhysicsInfoString_Post
	FN_GetPhysicsInfoString_Post,
#else
	nullptr,
#endif
#ifdef FN_PrecacheEvent_Post
	FN_PrecacheEvent_Post,
#else
	nullptr,
#endif
#ifdef FN_PlaybackEvent_Post
	FN_PlaybackEvent_Post,
#else
	nullptr,
#endif
#ifdef FN_SetFatPVS_Post
	FN_SetFatPVS_Post,
#else
	nullptr,
#endif
#ifdef FN_SetFatPAS_Post
	FN_SetFatPAS_Post,
#else
	nullptr,
#endif
#ifdef FN_CheckVisibility_Post
	FN_CheckVisibility_Post,
#else
	nullptr,
#endif
#ifdef FN_DeltaSetField_Post
	FN_DeltaSetField_Post,
#else
	nullptr,
#endif
#ifdef FN_DeltaUnsetField_Post
	FN_DeltaUnsetField_Post,
#else
	nullptr,
#endif
#ifdef FN_DeltaAddEncoder_Post
	FN_DeltaAddEncoder_Post,
#else
	nullptr,
#endif
#ifdef FN_GetCurrentPlayer_Post
	FN_GetCurrentPlayer_Post,
#else
	nullptr,
#endif
#ifdef FN_CanSkipPlayer_Post
	FN_CanSkipPlayer_Post,
#else
	nullptr,
#endif
#ifdef FN_DeltaFindField_Post
	FN_DeltaFindField_Post,
#else
	nullptr,
#endif
#ifdef FN_DeltaSetFieldByIndex_Post
	FN_DeltaSetFieldByIndex_Post,
#else
	nullptr,
#endif
#ifdef FN_DeltaUnsetFieldByIndex_Post
	FN_DeltaUnsetFieldByIndex_Post,
#else
	nullptr,
#endif
#ifdef FN_SetGroupMask_Post
	FN_SetGroupMask_Post,
#else
	nullptr,
#endif
#ifdef FN_engCreateInstancedBaseline_Post
	FN_engCreateInstancedBaseline_Post,
#else
	nullptr,
#endif
#ifdef FN_Cvar_DirectSet_Post
	FN_Cvar_DirectSet_Post,
#else
	nullptr,
#endif
#ifdef FN_ForceUnmodified_Post
	FN_ForceUnmodified_Post,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerStats_Post
	FN_GetPlayerStats_Post,
#else
	nullptr,
#endif
#ifdef FN_AddServerCommand_Post
	FN_AddServerCommand_Post,
#else
	nullptr,
#endif
#ifdef FN_Voice_GetClientListening_Post
	FN_Voice_GetClientListening_Post,
#else
	nullptr,
#endif
#ifdef FN_Voice_SetClientListening_Post
	FN_Voice_SetClientListening_Post,
#else
	nullptr,
#endif
#ifdef FN_GetPlayerAuthId_Post
	FN_GetPlayerAuthId_Post
#else
	nullptr
#endif
}; // g_EngineFuncs_Post_Table


static NEW_DLL_FUNCTIONS g_NewFuncs_Table = 
{
#ifdef FN_OnFreeEntPrivateData
	FN_OnFreeEntPrivateData,
#else
	nullptr,
#endif
#ifdef FN_GameShutdown
	FN_GameShutdown,
#else
	nullptr,
#endif
#ifdef FN_ShouldCollide
	ShouldCollide,
#else
	nullptr,
#endif
};


static NEW_DLL_FUNCTIONS g_NewFuncs_Post_Table = 
{
#ifdef FN_OnFreeEntPrivateData_Post
	FN_OnFreeEntPrivateData_Post,
#else
	nullptr,
#endif
#ifdef FN_GameShutdown_Post
	FN_GameShutdown_Post,
#else
	nullptr,
#endif
#ifdef FN_ShouldCollide_Post
	ShouldCollide_Post,
#else
	nullptr,
#endif
};

// Global variables from metamod.  These variable names are referenced by
// various macros.
meta_globals_t *gpMetaGlobals;		// metamod globals
gamedll_funcs_t *gpGamedllFuncs;	// gameDLL function tables
mutil_funcs_t *gpMetaUtilFuncs;		// metamod utility functions


plugin_info_t Plugin_info = {
		META_INTERFACE_VERSION,
		MODULE_NAME,
		MODULE_VERSION,
		MODULE_DATE,
		MODULE_AUTHOR,
		MODULE_URL,
		MODULE_LOGTAG,
		PT_ANYTIME,
		PT_ANYTIME
};

/*
C_DLLEXPORT int GetEntityAPI(DLL_FUNCTIONS *pFunctionTable, int interfaceVersion)
{
	LOG_DEVELOPER(PLID, "called: GetEntityAPI; version=%d", interfaceVersion);
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "GetEntityAPI called with null pFunctionTable");
		return(FALSE);
	}
	else if(interfaceVersion != INTERFACE_VERSION) {
		LOG_ERROR(PLID, "GetEntityAPI version mismatch; requested=%d ours=%d", interfaceVersion, INTERFACE_VERSION);
		return(FALSE);
	}
	memcpy(pFunctionTable, &g_EntityAPI_Table, sizeof( DLL_FUNCTIONS ) );

	return (TRUE);
}

C_DLLEXPORT int GetEntityAPI_Post(DLL_FUNCTIONS *pFunctionTable, int interfaceVersion)
{
	LOG_DEVELOPER(PLID, "called: GetEntityAPI_Post; version=%d", interfaceVersion);
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "GetEntityAPI_Post called with null pFunctionTable");
		return(FALSE);
	}
	else if(interfaceVersion != INTERFACE_VERSION) {
		LOG_ERROR(PLID, "GetEntityAPI_Post version mismatch; requested=%d ours=%d", interfaceVersion, INTERFACE_VERSION);
		return(FALSE);
	}
	memcpy(pFunctionTable, &g_EntityAPI_Post_Table, sizeof( DLL_FUNCTIONS ) );

	return(TRUE);
}
*/

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	LOG_DEVELOPER(PLID, "called: GetEntityAPI2; version=%d", *interfaceVersion);
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "GetEntityAPI2 called with null pFunctionTable");
		return(FALSE);
	}
	else if(*interfaceVersion != INTERFACE_VERSION) {
		LOG_ERROR(PLID, 
				"GetEntityAPI2 version mismatch; requested=%d ours=%d", 
				*interfaceVersion, INTERFACE_VERSION);
		//! Tell engine what version we had, so it can figure out who is 
		//! out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pFunctionTable, &g_EntityAPI_Table, sizeof(DLL_FUNCTIONS));
	g_pFunctionTable=pFunctionTable;
	return(TRUE);
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	LOG_DEVELOPER(PLID, "called: GetEntityAPI2_Post; version=%d", *interfaceVersion);
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "GetEntityAPI2_Post called with null pFunctionTable");
		return(FALSE);
	}
	else if(*interfaceVersion != INTERFACE_VERSION) {
		LOG_ERROR(PLID, "GetEntityAPI2_Post version mismatch; requested=%d ours=%d", *interfaceVersion, INTERFACE_VERSION);
		//! Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy( pFunctionTable, &g_EntityAPI_Post_Table, sizeof( DLL_FUNCTIONS ) );
	g_pFunctionTable_Post=pFunctionTable;
	return(TRUE);
}

C_DLLEXPORT int GetEngineFunctions(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
	LOG_DEVELOPER(PLID, "called: GetEngineFunctions; version=%d", 
			*interfaceVersion);
	if(!pengfuncsFromEngine) {
		LOG_ERROR(PLID, 
				"GetEngineFunctions called with null pengfuncsFromEngine");
		return(FALSE);
	}
	else if(*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		LOG_ERROR(PLID, 
				"GetEngineFunctions version mismatch; requested=%d ours=%d", 
				*interfaceVersion, ENGINE_INTERFACE_VERSION);
		// Tell metamod what version we had, so it can figure out who is 
		// out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pengfuncsFromEngine, &g_EngineFuncs_Table, sizeof(enginefuncs_t));
	g_pengfuncsTable=pengfuncsFromEngine;
	return TRUE;
}

C_DLLEXPORT int GetEngineFunctions_Post(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
	LOG_DEVELOPER(PLID, "called: GetEngineFunctions_Post; version=%d", *interfaceVersion);
	if(!pengfuncsFromEngine) {
		LOG_ERROR(PLID, "GetEngineFunctions_Post called with null pengfuncsFromEngine");
		return(FALSE);
	}
	else if(*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		LOG_ERROR(PLID, "GetEngineFunctions_Post version mismatch; requested=%d ours=%d", *interfaceVersion, ENGINE_INTERFACE_VERSION);
		// Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return(FALSE);
	}
	memcpy(pengfuncsFromEngine, &g_EngineFuncs_Post_Table, sizeof(enginefuncs_t));
	g_pengfuncsTable_Post=pengfuncsFromEngine;
	return TRUE;

}

C_DLLEXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS *pNewFunctionTable, 
		int *interfaceVersion) 
{
	LOG_DEVELOPER(PLID, "called: GetNewDLLFunctions; version=%d", 
			*interfaceVersion);
	if(!pNewFunctionTable) {
		LOG_ERROR(PLID, 
				"GetNewDLLFunctions called with null pNewFunctionTable");
		return(FALSE);
	}
	else if(*interfaceVersion != NEW_DLL_FUNCTIONS_VERSION) {
		LOG_ERROR(PLID, 
				"GetNewDLLFunctions version mismatch; requested=%d ours=%d", 
				*interfaceVersion, NEW_DLL_FUNCTIONS_VERSION);
		//! Tell engine what version we had, so it can figure out who is 
		//! out of date.
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return(FALSE);
	}
	memcpy(pNewFunctionTable, &g_NewFuncs_Table, sizeof(NEW_DLL_FUNCTIONS));
	g_pNewFunctionsTable=pNewFunctionTable;
	return TRUE;
}

C_DLLEXPORT int GetNewDLLFunctions_Post( NEW_DLL_FUNCTIONS *pNewFunctionTable, int *interfaceVersion ) 
{
	LOG_DEVELOPER(PLID, "called: GetNewDLLFunctions_Post; version=%d", *interfaceVersion);
	if(!pNewFunctionTable) {
		LOG_ERROR(PLID, "GetNewDLLFunctions_Post called with null pNewFunctionTable");
		return(FALSE);
	}
	else if(*interfaceVersion != NEW_DLL_FUNCTIONS_VERSION) {
		LOG_ERROR(PLID, "GetNewDLLFunctions_Post version mismatch; requested=%d ours=%d", *interfaceVersion, NEW_DLL_FUNCTIONS_VERSION);
		//! Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return(FALSE);
	}
	memcpy(pNewFunctionTable, &g_NewFuncs_Post_Table, sizeof(NEW_DLL_FUNCTIONS));
	g_pNewFunctionsTable_Post=pNewFunctionTable;
	return TRUE;
}


static META_FUNCTIONS g_MetaFunctions_Table =
{
	nullptr,
	nullptr,
	GetEntityAPI2,
	GetEntityAPI2_Post,
	GetNewDLLFunctions,
	GetNewDLLFunctions_Post,
	GetEngineFunctions,
	GetEngineFunctions_Post
};

C_DLLEXPORT int Meta_Query(const char *ifvers, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs)
{
	if (static_cast<int>(CVAR_GET_FLOAT("developer")) != 0)
		UTIL_LogPrintf("[%s] dev: called: Meta_Query; version=%s, ours=%s\n", 
				Plugin_info.logtag, ifvers, Plugin_info.ifvers);

	// Check for valid pMetaUtilFuncs before we continue.
	if(!pMetaUtilFuncs) {
		UTIL_LogPrintf("[%s] ERROR: Meta_Query called with null pMetaUtilFuncs\n", Plugin_info.logtag);
		return(FALSE);
	}

	gpMetaUtilFuncs = pMetaUtilFuncs;

	*pPlugInfo = &Plugin_info;

	// Check for interface version compatibility.
	if(!FStrEq(ifvers, Plugin_info.ifvers)) {
		int mmajor=0, mminor=0, pmajor=0, pminor=0;
		LOG_MESSAGE(PLID, "WARNING: meta-interface version mismatch; requested=%s ours=%s",
				Plugin_info.logtag, ifvers);
		// If plugin has later interface version, it's incompatible (update
		// metamod).
		sscanf(ifvers, "%d:%d", &mmajor, &mminor);
		sscanf(META_INTERFACE_VERSION, "%d:%d", &pmajor, &pminor);
		if(pmajor > mmajor || (pmajor==mmajor && pminor > mminor)) {
			LOG_ERROR(PLID, "metamod version is too old for this module; update metamod");
			return(FALSE);
		}
		// If plugin has older major interface version, it's incompatible
		// (update plugin).
		else if(pmajor < mmajor) {
			LOG_ERROR(PLID, "metamod version is incompatible with this module; please find a newer version of this module");
			return(FALSE);
		}
		// Minor interface is older, but this is guaranteed to be backwards
		// compatible, so we warn, but we still accept it.
		else if(pmajor==mmajor && pminor < mminor)
			LOG_MESSAGE(PLID, "WARNING: metamod version is newer than expected; consider finding a newer version of this module");
		else
			LOG_ERROR(PLID, "unexpected version comparison; metavers=%s, mmajor=%d, mminor=%d; plugvers=%s, pmajor=%d, pminor=%d", ifvers, mmajor, mminor, META_INTERFACE_VERSION, pmajor, pminor);
	}

#ifdef FN_META_QUERY
	FN_META_QUERY();
#endif	// FN_META_QUERY

	return 1;
}


C_DLLEXPORT int Meta_Attach(PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs)
{
	if(now > Plugin_info.loadable) {
		LOG_ERROR(PLID, "Can't load module right now");
		return(FALSE);
	}
	if(!pMGlobals) {
		LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
		return(FALSE);
	}
	gpMetaGlobals=pMGlobals;
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "Meta_Attach called with null pFunctionTable");
		return(FALSE);
	}

	memcpy(pFunctionTable, &g_MetaFunctions_Table, sizeof(META_FUNCTIONS));
	gpGamedllFuncs=pGamedllFuncs;

	// Let's go.

#ifdef FN_META_ATTACH
	FN_META_ATTACH();
#endif	// FN_META_ATTACH

	return TRUE;
}

C_DLLEXPORT int Meta_Detach(PLUG_LOADTIME now, PL_UNLOAD_REASON reason)
{
	if(now > Plugin_info.unloadable && reason != PNL_CMD_FORCED) {
		LOG_ERROR(PLID, "Can't unload plugin right now");
		return(FALSE);
	}

#ifdef FN_META_DETACH
	FN_META_DETACH();
#endif	// FN_META_DETACH
	return TRUE;
}

C_DLLEXPORT void WINAPI GiveFnptrsToDll(enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals)
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	gpGlobals = pGlobals;
}

#endif	// #ifdef USE_METAMOD

/************* AMXX Stuff *************/

// *** Globals ***
// Module info
static amxx_module_info_s g_ModuleInfo = 
{
	MODULE_NAME,
	MODULE_AUTHOR,
	MODULE_VERSION,
#ifdef MODULE_RELOAD_ON_MAPCHANGE
	1,
#else // MODULE_RELOAD_ON_MAPCHANGE
	0,
#endif // MODULE_RELOAD_ON_MAPCHANGE
	MODULE_LOGTAG,
	MODULE_LIBRARY,
	MODULE_LIBCLASS
};

// Storage for the requested functions
PFN_ADD_NATIVES				g_fn_AddNatives;
PFN_ADD_NEW_NATIVES			g_fn_AddNewNatives;
PFN_BUILD_PATHNAME			g_fn_BuildPathname;
PFN_BUILD_PATHNAME_R		g_fn_BuildPathnameR;
PFN_GET_AMXADDR				g_fn_GetAmxAddr;
PFN_GET_AMXVECTOR_NULL		g_fn_GetAmxVectorNull;
PFN_PRINT_SRVCONSOLE		g_fn_PrintSrvConsole;
PFN_GET_MODNAME				g_fn_GetModname;
PFN_GET_AMXSCRIPTNAME		g_fn_GetAmxScriptName;
PFN_GET_AMXSCRIPT			g_fn_GetAmxScript;
PFN_FIND_AMXSCRIPT_BYAMX	g_fn_FindAmxScriptByAmx;
PFN_FIND_AMXSCRIPT_BYNAME	g_fn_FindAmxScriptByName;
PFN_SET_AMXSTRING			g_fn_SetAmxString;
PFN_SET_AMXSTRING_UTF8_CHAR	g_fn_SetAmxStringUTF8Char;
PFN_SET_AMXSTRING_UTF8_CELL	g_fn_SetAmxStringUTF8Cell;
PFN_GET_AMXSTRING			g_fn_GetAmxString;
PFN_GET_AMXSTRING_NULL		g_fn_GetAmxStringNull;
PFN_GET_AMXSTRINGLEN		g_fn_GetAmxStringLen;
PFN_FORMAT_AMXSTRING		g_fn_FormatAmxString;
PFN_COPY_AMXMEMORY			g_fn_CopyAmxMemory;
PFN_LOG						g_fn_Log;
PFN_LOG_ERROR				g_fn_LogErrorFunc;
PFN_RAISE_AMXERROR			g_fn_RaiseAmxError;
PFN_REGISTER_FORWARD		g_fn_RegisterForward;
PFN_EXECUTE_FORWARD			g_fn_ExecuteForward;
PFN_PREPARE_CELLARRAY		g_fn_PrepareCellArray;
PFN_PREPARE_CHARARRAY		g_fn_PrepareCharArray;
PFN_PREPARE_CELLARRAY_A		g_fn_PrepareCellArrayA;
PFN_PREPARE_CHARARRAY_A		g_fn_PrepareCharArrayA;
PFN_IS_PLAYER_VALID			g_fn_IsPlayerValid;
PFN_GET_PLAYER_NAME			g_fn_GetPlayerName;
PFN_GET_PLAYER_IP			g_fn_GetPlayerIP;
PFN_IS_PLAYER_INGAME		g_fn_IsPlayerIngame;
PFN_IS_PLAYER_BOT			g_fn_IsPlayerBot;
PFN_IS_PLAYER_AUTHORIZED	g_fn_IsPlayerAuthorized;
PFN_GET_PLAYER_TIME			g_fn_GetPlayerTime;
PFN_GET_PLAYER_PLAYTIME		g_fn_GetPlayerPlayTime;
PFN_GET_PLAYER_CURWEAPON	g_fn_GetPlayerCurweapon;
PFN_GET_PLAYER_TEAM			g_fn_GetPlayerTeam;
PFN_GET_PLAYER_TEAMID		g_fn_GetPlayerTeamID;
PFN_GET_PLAYER_DEATHS		g_fn_GetPlayerDeaths;
PFN_GET_PLAYER_MENU			g_fn_GetPlayerMenu;
PFN_GET_PLAYER_KEYS			g_fn_GetPlayerKeys;
PFN_IS_PLAYER_ALIVE			g_fn_IsPlayerAlive;
PFN_GET_PLAYER_FRAGS		g_fn_GetPlayerFrags;
PFN_IS_PLAYER_CONNECTING	g_fn_IsPlayerConnecting;
PFN_IS_PLAYER_HLTV			g_fn_IsPlayerHLTV;
PFN_GET_PLAYER_ARMOR		g_fn_GetPlayerArmor;
PFN_GET_PLAYER_HEALTH		g_fn_GetPlayerHealth;
#ifdef MEMORY_TEST
PFN_ALLOCATOR				g_fn_Allocator;
PFN_REALLOCATOR				g_fn_Reallocator;
PFN_DEALLOCATOR				g_fn_Deallocator;
#endif
PFN_AMX_EXEC				g_fn_AmxExec;
PFN_AMX_EXECV				g_fn_AmxExecv;
PFN_AMX_ALLOT				g_fn_AmxAllot;
PFN_AMX_FINDPUBLIC			g_fn_AmxFindPublic;
PFN_LOAD_AMXSCRIPT			g_fn_LoadAmxScript;
PFN_LOAD_AMXSCRIPT_EX		g_fn_LoadAmxScriptEx;
PFN_UNLOAD_AMXSCRIPT		g_fn_UnloadAmxScript;
PFN_REAL_TO_CELL			g_fn_RealToCell;
PFN_CELL_TO_REAL			g_fn_CellToReal;
PFN_REGISTER_SPFORWARD		g_fn_RegisterSPForward;
PFN_REGISTER_SPFORWARD_BYNAME	g_fn_RegisterSPForwardByName;
PFN_UNREGISTER_SPFORWARD	g_fn_UnregisterSPForward;
PFN_MERGEDEFINITION_FILE	g_fn_MergeDefinition_File;
PFN_AMX_FINDNATIVE			g_fn_AmxFindNative;
PFN_GETPLAYERFLAGS			g_fn_GetPlayerFlags;
PFN_GET_PLAYER_EDICT		g_fn_GetPlayerEdict;
PFN_FORMAT					g_fn_Format;
PFN_REGISTERFUNCTION		g_fn_RegisterFunction;
PFN_REQ_FNPTR				g_fn_RequestFunction;
PFN_AMX_PUSH				g_fn_AmxPush;
PFN_SET_TEAM_INFO			g_fn_SetTeamInfo;
PFN_PLAYER_PROP_ADDR		g_fn_PlayerPropAddr;
PFN_REG_AUTH_FUNC			g_fn_RegAuthFunc;
PFN_UNREG_AUTH_FUNC			g_fn_UnregAuthFunc;
PFN_FINDLIBRARY				g_fn_FindLibrary;
PFN_ADDLIBRARIES			g_fn_AddLibraries;
PFN_REMOVELIBRARIES			g_fn_RemoveLibraries;
PFN_OVERRIDENATIVES			g_fn_OverrideNatives;
PFN_GETLOCALINFO			g_fn_GetLocalInfo;
PFN_AMX_REREGISTER			g_fn_AmxReRegister;
PFN_REGISTERFUNCTIONEX		g_fn_RegisterFunctionEx;
PFN_MESSAGE_BLOCK			g_fn_MessageBlock;
PFN_GET_CONFIG_MANAGER		g_fn_GetConfigManager;

// *** Exports ***
C_DLLEXPORT int AMXX_Query(int *interfaceVersion, amxx_module_info_s *moduleInfo)
{
	// check parameters
	if (!interfaceVersion || !moduleInfo)
		return AMXX_PARAM;

	// check interface version
	if (*interfaceVersion != AMXX_INTERFACE_VERSION)
	{
		// Tell amxx core our interface version
		*interfaceVersion = AMXX_INTERFACE_VERSION;
		return AMXX_IFVERS;
	}

	// copy module info
	memcpy(moduleInfo, &g_ModuleInfo, sizeof(amxx_module_info_s));

#ifdef FN_AMXX_QUERY
	FN_AMXX_QUERY();
#endif // FN_AMXX_QUERY
	// Everything ok :)
	return AMXX_OK;
}

// request function
#define REQFUNC(name, fptr, type) if (((fptr) = (type)reqFnptrFunc(name)) == 0) return AMXX_FUNC_NOT_PRESENT
// request optional function
#define REQFUNC_OPT(name, fptr, type) fptr = (type)reqFnptrFunc(name)

C_DLLEXPORT int AMXX_CheckGame(const char *game)
{
#ifdef FN_AMXX_CHECKGAME
	return FN_AMXX_CHECKGAME(game);
#else
	return AMXX_GAME_OK;
#endif
}
C_DLLEXPORT int AMXX_Attach(PFN_REQ_FNPTR reqFnptrFunc)
{
	// Check pointer
	if (!reqFnptrFunc)
		return AMXX_PARAM;

	g_fn_RequestFunction = reqFnptrFunc;

	// Req all known functions
	// Misc
	REQFUNC("BuildPathname", g_fn_BuildPathname, PFN_BUILD_PATHNAME);
	REQFUNC("BuildPathnameR", g_fn_BuildPathnameR, PFN_BUILD_PATHNAME_R);
	REQFUNC("PrintSrvConsole", g_fn_PrintSrvConsole, PFN_PRINT_SRVCONSOLE);
	REQFUNC("GetModname", g_fn_GetModname, PFN_GET_MODNAME);
	REQFUNC("Log", g_fn_Log, PFN_LOG);
	REQFUNC("LogError", g_fn_LogErrorFunc, PFN_LOG_ERROR);
	REQFUNC("MergeDefinitionFile", g_fn_MergeDefinition_File, PFN_MERGEDEFINITION_FILE);
	REQFUNC("Format", g_fn_Format, PFN_FORMAT);
	REQFUNC("RegisterFunction", g_fn_RegisterFunction, PFN_REGISTERFUNCTION);
	REQFUNC("RegisterFunctionEx", g_fn_RegisterFunctionEx, PFN_REGISTERFUNCTIONEX);
	REQFUNC("GetConfigManager", g_fn_GetConfigManager, PFN_GET_CONFIG_MANAGER);

	// Amx scripts
	REQFUNC("GetAmxScript", g_fn_GetAmxScript, PFN_GET_AMXSCRIPT);
	REQFUNC("FindAmxScriptByAmx", g_fn_FindAmxScriptByAmx, PFN_FIND_AMXSCRIPT_BYAMX);
	REQFUNC("FindAmxScriptByName", g_fn_FindAmxScriptByName, PFN_FIND_AMXSCRIPT_BYNAME);
	REQFUNC("LoadAmxScript", g_fn_LoadAmxScript, PFN_LOAD_AMXSCRIPT); // Deprecated. Please use LoadAmxScriptEx instead.
	REQFUNC("LoadAmxScriptEx", g_fn_LoadAmxScriptEx, PFN_LOAD_AMXSCRIPT_EX);
	REQFUNC("UnloadAmxScript", g_fn_UnloadAmxScript, PFN_UNLOAD_AMXSCRIPT);
    REQFUNC("GetAmxScriptName", g_fn_GetAmxScriptName, PFN_GET_AMXSCRIPTNAME);

	// String / mem in amx scripts support
	REQFUNC("SetAmxString", g_fn_SetAmxString, PFN_SET_AMXSTRING);
	REQFUNC("SetAmxStringUTF8Char", g_fn_SetAmxStringUTF8Char, PFN_SET_AMXSTRING_UTF8_CHAR);
	REQFUNC("SetAmxStringUTF8Cell", g_fn_SetAmxStringUTF8Cell, PFN_SET_AMXSTRING_UTF8_CELL);
	REQFUNC("GetAmxString", g_fn_GetAmxString, PFN_GET_AMXSTRING);
	REQFUNC("GetAmxStringNull", g_fn_GetAmxStringNull, PFN_GET_AMXSTRING_NULL);
	REQFUNC("GetAmxStringLen", g_fn_GetAmxStringLen, PFN_GET_AMXSTRINGLEN);
	REQFUNC("FormatAmxString", g_fn_FormatAmxString, PFN_FORMAT_AMXSTRING);
	REQFUNC("CopyAmxMemory", g_fn_CopyAmxMemory, PFN_COPY_AMXMEMORY);
	REQFUNC("GetAmxAddr", g_fn_GetAmxAddr, PFN_GET_AMXADDR);
	REQFUNC("GetAmxVectorNull", g_fn_GetAmxVectorNull, PFN_GET_AMXVECTOR_NULL);

	REQFUNC("amx_Exec", g_fn_AmxExec, PFN_AMX_EXEC);
	REQFUNC("amx_Execv", g_fn_AmxExecv, PFN_AMX_EXECV);
	REQFUNC("amx_FindPublic", g_fn_AmxFindPublic, PFN_AMX_FINDPUBLIC);
	REQFUNC("amx_Allot", g_fn_AmxAllot, PFN_AMX_ALLOT);
	REQFUNC("amx_FindNative", g_fn_AmxFindNative, PFN_AMX_FINDNATIVE);

	// Natives / Forwards
	REQFUNC("AddNatives", g_fn_AddNatives, PFN_ADD_NATIVES);
	REQFUNC("AddNewNatives", g_fn_AddNewNatives, PFN_ADD_NEW_NATIVES);
	REQFUNC("RaiseAmxError", g_fn_RaiseAmxError, PFN_RAISE_AMXERROR);
	REQFUNC("RegisterForward", g_fn_RegisterForward, PFN_REGISTER_FORWARD);
	REQFUNC("RegisterSPForward", g_fn_RegisterSPForward, PFN_REGISTER_SPFORWARD);
	REQFUNC("RegisterSPForwardByName", g_fn_RegisterSPForwardByName, PFN_REGISTER_SPFORWARD_BYNAME);
	REQFUNC("UnregisterSPForward", g_fn_UnregisterSPForward, PFN_UNREGISTER_SPFORWARD);
	REQFUNC("ExecuteForward", g_fn_ExecuteForward, PFN_EXECUTE_FORWARD);
	REQFUNC("PrepareCellArray", g_fn_PrepareCellArray, PFN_PREPARE_CELLARRAY);
	REQFUNC("PrepareCharArray", g_fn_PrepareCharArray, PFN_PREPARE_CHARARRAY);
	REQFUNC("PrepareCellArrayA", g_fn_PrepareCellArrayA, PFN_PREPARE_CELLARRAY_A);
	REQFUNC("PrepareCharArrayA", g_fn_PrepareCharArrayA, PFN_PREPARE_CHARARRAY_A);
	// Player
	REQFUNC("IsPlayerValid", g_fn_IsPlayerValid, PFN_IS_PLAYER_VALID);
	REQFUNC("GetPlayerName", g_fn_GetPlayerName, PFN_GET_PLAYER_NAME);
	REQFUNC("GetPlayerIP", g_fn_GetPlayerIP, PFN_GET_PLAYER_IP);
	REQFUNC("IsPlayerInGame", g_fn_IsPlayerIngame, PFN_IS_PLAYER_INGAME);		
	REQFUNC("IsPlayerBot", g_fn_IsPlayerBot, PFN_IS_PLAYER_BOT);	
	REQFUNC("IsPlayerAuthorized", g_fn_IsPlayerAuthorized, PFN_IS_PLAYER_AUTHORIZED);
	REQFUNC("GetPlayerTime", g_fn_GetPlayerTime, PFN_GET_PLAYER_TIME);
	REQFUNC("GetPlayerPlayTime", g_fn_GetPlayerPlayTime, PFN_GET_PLAYER_PLAYTIME);
	REQFUNC("GetPlayerCurweapon", g_fn_GetPlayerCurweapon, PFN_GET_PLAYER_CURWEAPON);
	REQFUNC("GetPlayerTeamID", g_fn_GetPlayerTeamID, PFN_GET_PLAYER_TEAMID);
	REQFUNC("GetPlayerTeam",g_fn_GetPlayerTeam, PFN_GET_PLAYER_TEAM);
	REQFUNC("GetPlayerDeaths", g_fn_GetPlayerDeaths, PFN_GET_PLAYER_DEATHS);
	REQFUNC("GetPlayerMenu", g_fn_GetPlayerMenu, PFN_GET_PLAYER_MENU);
	REQFUNC("GetPlayerKeys", g_fn_GetPlayerKeys, PFN_GET_PLAYER_KEYS);
	REQFUNC("IsPlayerAlive", g_fn_IsPlayerAlive, PFN_IS_PLAYER_ALIVE);
	REQFUNC("GetPlayerFrags", g_fn_GetPlayerFrags, PFN_GET_PLAYER_FRAGS);
	REQFUNC("IsPlayerConnecting", g_fn_IsPlayerConnecting, PFN_IS_PLAYER_CONNECTING);
	REQFUNC("IsPlayerHLTV", g_fn_IsPlayerHLTV, PFN_IS_PLAYER_HLTV);
	REQFUNC("GetPlayerArmor", g_fn_GetPlayerArmor, PFN_GET_PLAYER_ARMOR);
	REQFUNC("GetPlayerHealth", g_fn_GetPlayerHealth, PFN_GET_PLAYER_HEALTH);
	REQFUNC("GetPlayerFlags", g_fn_GetPlayerFlags, PFN_GETPLAYERFLAGS);
	REQFUNC("GetPlayerEdict", g_fn_GetPlayerEdict, PFN_GET_PLAYER_EDICT);
	REQFUNC("amx_Push", g_fn_AmxPush, PFN_AMX_PUSH);
	REQFUNC("SetPlayerTeamInfo", g_fn_SetTeamInfo, PFN_SET_TEAM_INFO);
	REQFUNC("PlayerPropAddr", g_fn_PlayerPropAddr, PFN_PLAYER_PROP_ADDR);
	REQFUNC("RegAuthFunc", g_fn_RegAuthFunc, PFN_REG_AUTH_FUNC);
	REQFUNC("UnregAuthFunc", g_fn_UnregAuthFunc, PFN_UNREG_AUTH_FUNC);

	//Added in 1.75 
	REQFUNC("FindLibrary", g_fn_FindLibrary, PFN_FINDLIBRARY);
	REQFUNC("AddLibraries", g_fn_AddLibraries, PFN_ADDLIBRARIES);
	REQFUNC("RemoveLibraries", g_fn_RemoveLibraries, PFN_REMOVELIBRARIES);
	REQFUNC("OverrideNatives", g_fn_OverrideNatives, PFN_OVERRIDENATIVES);
	REQFUNC("GetLocalInfo", g_fn_GetLocalInfo, PFN_GETLOCALINFO);
	REQFUNC("AmxReregister", g_fn_AmxReRegister, PFN_AMX_REREGISTER);

	REQFUNC("MessageBlock", g_fn_MessageBlock, PFN_MESSAGE_BLOCK);

#ifdef MEMORY_TEST
	// Memory
	REQFUNC_OPT("Allocator", g_fn_Allocator, PFN_ALLOCATOR);
	REQFUNC_OPT("Reallocator", g_fn_Reallocator, PFN_REALLOCATOR);
	REQFUNC_OPT("Deallocator", g_fn_Deallocator, PFN_DEALLOCATOR);
#endif

	REQFUNC("CellToReal", g_fn_CellToReal, PFN_CELL_TO_REAL);
	REQFUNC("RealToCell", g_fn_RealToCell, PFN_REAL_TO_CELL);

#ifdef FN_AMXX_ATTACH
	FN_AMXX_ATTACH();
#endif // FN_AMXX_ATACH

	return AMXX_OK;
}

C_DLLEXPORT int AMXX_Detach()
{
#ifdef FN_AMXX_DETACH
	FN_AMXX_DETACH();
#endif // FN_AMXX_DETACH

	return AMXX_OK;
}

C_DLLEXPORT int AMXX_PluginsLoaded()
{
#ifdef FN_AMXX_PLUGINSLOADED
	FN_AMXX_PLUGINSLOADED();
#endif // FN_AMXX_PLUGINSLOADED
	return AMXX_OK;
}

C_DLLEXPORT void AMXX_PluginsUnloaded()
{
#ifdef FN_AMXX_PLUGINSUNLOADED
	FN_AMXX_PLUGINSUNLOADED();
#endif // FN_AMXX_PLUGINSUNLOADED
}

C_DLLEXPORT void AMXX_PluginsUnloading()
{
#ifdef FN_AMXX_PLUGINSUNLOADING
	FN_AMXX_PLUGINSUNLOADING();
#endif // FN_AMXX_PLUGINSUNLOADING
}

// Advanced MF functions
void MF_Log(const char *fmt, ...)
{
	char msg[3072];
	va_list arglst;
	va_start(arglst, fmt);
	vsnprintf(msg, sizeof(msg) - 1, fmt, arglst);
	va_end(arglst);

	g_fn_Log("[%s] %s", MODULE_LOGTAG, msg);
}

void MF_LogError(AMX *amx, int err, const char *fmt, ...)
{
	char msg[3072];
	va_list arglst;
	va_start(arglst, fmt);
	vsnprintf(msg, sizeof(msg) - 1, fmt, arglst);
	va_end(arglst);

	g_fn_LogErrorFunc(amx, err, "[%s] %s", MODULE_LOGTAG, msg);
}


#ifdef _DEBUG
// validate macros
// Makes sure compiler reports errors when macros are invalid
void ValidateMacros_DontCallThis_Smiley()
{
	const cell str[] = { 's', 't', 'r', '\0' };
	MF_BuildPathname("str", "str", 0);
	MF_BuildPathnameR(nullptr, 0, "%d", 0);
	MF_FormatAmxString(nullptr, nullptr, 0, nullptr);
	MF_GetAmxAddr(nullptr, 0);
	MF_GetAmxVectorNull(nullptr, 0);
	MF_PrintSrvConsole("str", "str", 0);
	MF_GetModname();
	MF_GetScriptName(0);
	MF_GetScriptAmx(0);
	MF_FindScriptByAmx(nullptr);
	MF_FindScriptByName("str");
	MF_SetAmxString(nullptr, 0, "str", 0);
	MF_SetAmxStringUTF8Char(nullptr, 0, "str", 0, 0);
	MF_SetAmxStringUTF8Cell(nullptr, 0, str, 0, 0);
	MF_GetAmxString(nullptr, 0, 0, nullptr);
	MF_GetAmxStringNull(nullptr, 0, 0, nullptr);
	MF_GetAmxStringLen(nullptr);
	MF_CopyAmxMemory(nullptr, nullptr, 0);
	MF_Log("str", "str", 0);
	MF_LogError(nullptr, 0, nullptr);
	MF_RaiseAmxError(nullptr, 0);
	MF_RegisterForward("str", static_cast<ForwardExecType>(0), 0, 0, 0);
	MF_ExecuteForward(0, 0, 0);
	MF_PrepareCellArray(nullptr, 0);
	MF_PrepareCharArray(nullptr, 0);
	MF_PrepareCellArrayA(nullptr, 0, true);
	MF_PrepareCharArrayA(nullptr, 0, true);
	MF_IsPlayerValid(0);
	MF_GetPlayerName(0);
	MF_GetPlayerIP(0);
	MF_IsPlayerIngame(0);
	MF_IsPlayerBot(0);
	MF_IsPlayerAuthorized(0);
	MF_GetPlayerTime(0);
	MF_GetPlayerPlayTime(0);
	MF_GetPlayerCurweapon(0);
	MF_GetPlayerTeamID(0);
	MF_GetPlayerTeam(0);
	MF_GetPlayerDeaths(0);
	MF_GetPlayerMenu(0);
	MF_GetPlayerKeys(0);
	MF_IsPlayerAlive(0);
	MF_GetPlayerFrags(0);
	MF_IsPlayerConnecting(0);
	MF_IsPlayerHLTV(0);
	MF_GetPlayerArmor(0);
	MF_GetPlayerHealth(0);
	MF_AmxExec(nullptr, nullptr, 0);
	MF_AmxExecv(nullptr, nullptr, 0, 0, nullptr);
	MF_AmxFindPublic(nullptr, nullptr, nullptr);
	MF_AmxAllot(nullptr, 0, nullptr, nullptr);
	MF_LoadAmxScript(nullptr, nullptr, nullptr, nullptr, 0);
	MF_LoadAmxScriptEx(nullptr, nullptr, nullptr, nullptr, 0, 0);
	MF_UnloadAmxScript(nullptr, nullptr);
	MF_RegisterSPForward(nullptr, 0, 0, 0, 0, 0);
	MF_RegisterSPForwardByName(nullptr, nullptr, 0, 0, 0, 0);
	MF_UnregisterSPForward(0);
	MF_GetPlayerFrags(0);
	MF_GetPlayerEdict(0);
	MF_Format("", 4, "str");
	MF_RegisterFunction(nullptr, "");
	MF_RegisterFunctionEx(nullptr, "");
	MF_SetPlayerTeamInfo(0, 0, "");
	MF_PlayerPropAddr(0, 0);
	MF_RegAuthFunc(nullptr);
	MF_UnregAuthFunc(nullptr);
	MF_FindLibrary(nullptr, LibType_Class);
	MF_AddLibraries(nullptr, LibType_Class, nullptr);
	MF_RemoveLibraries(nullptr);
	MF_OverrideNatives(nullptr, nullptr);
	MF_MessageBlock(0, 0, nullptr);
	MF_GetConfigManager();
}
#endif

#ifdef MEMORY_TEST

/************* MEMORY *************/
// undef all defined macros
#undef new
#undef delete
#undef malloc
#undef calloc
#undef realloc
#undef free

const		unsigned int	m_alloc_unknown        = 0;
const		unsigned int	m_alloc_new            = 1;
const		unsigned int	m_alloc_new_array      = 2;
const		unsigned int	m_alloc_malloc         = 3;
const		unsigned int	m_alloc_calloc         = 4;
const		unsigned int	m_alloc_realloc        = 5;
const		unsigned int	m_alloc_delete         = 6;
const		unsigned int	m_alloc_delete_array   = 7;
const		unsigned int	m_alloc_free           = 8;

const char *g_Mem_CurrentFilename = "??";
int g_Mem_CurrentLine = 0;
const char *g_Mem_CurrentFunc = "??";

const char *Mem_MakeSourceFile(const char *sourceFile)
{
	static char buffer[512];
	static size_t pos = 0;
	if (!pos)
	{
		// init
		buffer[0] = '[';
		strcpy(buffer + 1, MODULE_NAME);
		pos = strlen(MODULE_NAME) + 1;
		buffer[pos++] = ']';
	}

	// convert from absolute path to [modulename]filename
	const char *ptr = strrchr(sourceFile, '\\');
	if (ptr)
		ptr++;
	else
	{
		ptr = strrchr(sourceFile, '/');
		if (ptr)
			ptr++;
		else
			ptr = sourceFile;
	}
	strcpy(buffer + pos, ptr);
	return buffer;
}

void Mem_SetOwner(const char *filename, int line, const char *function)
{
	g_Mem_CurrentFilename = filename;
	g_Mem_CurrentLine = line;
	g_Mem_CurrentFunc = function;
}

void Mem_ResetGlobals()
{
	Mem_SetOwner("??", 0, "??");
}

// raw (re/de)allocators
void *	Mem_Allocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
			     const unsigned int allocationType, const size_t reportedSize)
{
	if (g_fn_Allocator)
		return g_fn_Allocator(Mem_MakeSourceFile(sourceFile), sourceLine, sourceFunc, allocationType, reportedSize);
	else
		return malloc(reportedSize);
}

void *	Mem_Reallocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
			       const unsigned int reallocationType, const size_t reportedSize, void *reportedAddress)
{
	if (g_fn_Reallocator)
		return g_fn_Reallocator(Mem_MakeSourceFile(sourceFile), sourceLine, sourceFunc, reallocationType, reportedSize, reportedAddress);
	else
		return realloc(reportedAddress, reportedSize);
}

void	Mem_Deallocator(const char *sourceFile, const unsigned int sourceLine, const char *sourceFunc,
			      const unsigned int deallocationType, void *reportedAddress)
{
	// If you you get user breakpoint here, something failed :)
	//  - invalid pointer
	//  - alloc type mismatch	( for example
	//							char *a = new char[5]; delete char;
	//							)
	//  - The allocation unit is damaged (for example
	//							char *a = new char[5]; a[6] = 8;
	//							)
	//  - break on dealloc flag set (somehow)

	if (g_fn_Deallocator)
		g_fn_Deallocator(Mem_MakeSourceFile(sourceFile), sourceLine, sourceFunc, deallocationType, reportedAddress);
	else
		free(reportedAddress);
}

// new and delete operators
void	*operator new(size_t reportedSize)
{
	if (reportedSize == 0)
		reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}

void	*operator new[](size_t reportedSize)
{
	if (reportedSize == 0)
		reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new_array, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}

// Microsoft memory tracking operators
void	*operator new(size_t reportedSize, const char *sourceFile, int sourceLine)
{
	if (reportedSize == 0)
		reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}
void	*operator new[](size_t reportedSize, const char *sourceFile, int sourceLine)
{
	if (reportedSize == 0)
		reportedSize = 1;
	void *ptr = Mem_Allocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_new_array, reportedSize);
	// :TODO: Handler support ?
	if (ptr)
		return ptr;

	// allocation failed
	return NULL;
}

void	operator delete(void *reportedAddress)
{
	if (!reportedAddress)
		return;

	Mem_Deallocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_delete, reportedAddress);
}

void	operator delete[](void *reportedAddress)
{
	if (!reportedAddress)
		return;

	Mem_Deallocator(g_Mem_CurrentFilename, g_Mem_CurrentLine, g_Mem_CurrentFunc, m_alloc_delete_array, reportedAddress);
}

#else

#if !defined NO_ALLOC_OVERRIDES && !defined MEMORY_TEST && !defined WIN32
void * operator new(size_t size) {
	return(calloc(1, size)); 
}

void * operator new[](size_t size) {
	return(calloc(1, size)); 
}

void operator delete(void * ptr) {
	if(ptr)
		free(ptr);
}

void operator delete[](void * ptr) {
	if(ptr)
		free(ptr);
}
#endif

#endif //MEMORY_TEST

/************* stuff from dlls/util.cpp *************/
//				must come here because cbase.h declares it's own operator new

#ifdef USE_METAMOD

// Selected portions of dlls/util.cpp from SDK 2.1.
// Functions copied from there as needed...
// And modified to avoid buffer overflows (argh).

/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== util.cpp ========================================================

  Utility code.  Really not optional after all.

*/

#include <extdll.h>
#include "sdk_util.h"
#include <cbase.h>

#include <string.h>			// for strncpy(), etc

#include "osdep.h"			// win32 vsnprintf, etc

char* UTIL_VarArgs( char *format, ... )
{
	va_list		argptr;
	static char		string[1024];
	
	va_start (argptr, format);
	vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	return string;
}
	

//=========================================================
// UTIL_LogPrintf - Prints a logged message to console.
// Preceded by LOG: ( timestamp ) < message >
//=========================================================
void UTIL_LogPrintf( const char *fmt, ... )
{
	va_list			argptr;
	static char		string[1024];
	
	va_start ( argptr, fmt );
	vsnprintf ( string, sizeof(string), fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	ALERT( at_logged, "%s", string );
}


void UTIL_HudMessage(CBaseEntity *pEntity, const hudtextparms_t &textparms, 
		const char *pMessage)
{
	if ( !pEntity )
		return;

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, nullptr, ENT(pEntity->pev) );
		WRITE_BYTE( TE_TEXTMESSAGE );
		WRITE_BYTE( textparms.channel & 0xFF );

		WRITE_SHORT( FixedSigned16( textparms.x, 1<<13 ) );
		WRITE_SHORT( FixedSigned16( textparms.y, 1<<13 ) );
		WRITE_BYTE( textparms.effect );

		WRITE_BYTE( textparms.r1 );
		WRITE_BYTE( textparms.g1 );
		WRITE_BYTE( textparms.b1 );
		WRITE_BYTE( textparms.a1 );

		WRITE_BYTE( textparms.r2 );
		WRITE_BYTE( textparms.g2 );
		WRITE_BYTE( textparms.b2 );
		WRITE_BYTE( textparms.a2 );

		WRITE_SHORT( FixedUnsigned16( textparms.fadeinTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.fadeoutTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.holdTime, 1<<8 ) );

		if ( textparms.effect == 2 )
			WRITE_SHORT( FixedUnsigned16( textparms.fxTime, 1<<8 ) );
		
		if ( strlen( pMessage ) < 512 )
		{
			WRITE_STRING( pMessage );
		}
		else
		{
			char tmp[512];
			strncpy( tmp, pMessage, 511 );
			tmp[511] = 0;
			WRITE_STRING( tmp );
		}
	MESSAGE_END();
}

short FixedSigned16( float value, float scale )
{
	int output = static_cast<int>(value * scale);

	if ( output > 32767 )
		output = 32767;

	if ( output < -32768 )
		output = -32768;

	return static_cast<short>(output);
}

unsigned short FixedUnsigned16( float value, float scale )
{
	int output = static_cast<int>(value * scale);
	if ( output < 0 )
		output = 0;
	if ( output > 0xFFFF )
		output = 0xFFFF;

	return static_cast<unsigned short>(output);
}
#endif // USE_METAMOD

template unsigned int strncopy<char, char>(char *, const char *src, size_t count);
template unsigned int strncopy<cell, char>(cell *, const char *src, size_t count);
template unsigned int strncopy<cell, cell>(cell *, const cell *src, size_t count);

template <typename D, typename S>
unsigned int strncopy(D *dest, const S *src, size_t count)
{
	if (!count)
	{
		return 0;
	}

	D *start = dest;

	while ((*src) && (--count))
	{
		*dest++ = *(unsigned char*)src++;
	}

	*dest = '\0';

	return (dest - start);
}
