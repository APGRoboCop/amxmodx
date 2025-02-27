// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// Fakemeta Module
//

#include "fakemeta_amxx.h"
#include <engine_strucs.h>

TraceResult g_tr;

ke::AString LightStyleBuffers[MAX_LIGHTSTYLES];

//by mahnsawce from his NS module
static cell AMX_NATIVE_CALL engfunc(AMX *amx, cell *params)
{
	// Variables I will need throughout all the different calls.
	int type = params[1];
//	LOG_CONSOLE(PLID,"Called: %i %i",type,*params/sizeof(cell));
	int len;
	char *temp;
	char *temp2;
	char *temp3;
	cell *cRet;
	vec3_t	Vec1;
	vec3_t	Vec2;
	vec3_t	Vec3;
	vec3_t	Vec4;
	int iparam1;
	int iparam2;
	int iparam3;
	int iparam4;
	int iparam5;
	int iparam6;
	float fparam1;
	float fparam2;
	float fparam3;
//	float fTemp[3];
	int index;
	unsigned char *pset;
	edict_t *pRet= nullptr;
	// Now start calling.. :/
	switch (type)
	{
		// pfnPrecacheModel
	case 	EngFunc_PrecacheModel:	// int  )			(const char* s);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		if (temp[0]==0)
			return 0;
		return (*g_engfuncs.pfnPrecacheModel)((char *)STRING(ALLOC_STRING(temp)));


		// pfnPrecacheSound
	case	EngFunc_PrecacheSound:	// int  )			(const char* s);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		if (temp[0]==0)
			return 0;
		return (*g_engfuncs.pfnPrecacheSound)((char*)STRING(ALLOC_STRING(temp)));


		// pfnSetModel
	case	EngFunc_SetModel:		// void )				(edict_t *e, const char *m);
		temp = MF_GetAmxString(amx,params[3],0,&len);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		(*g_engfuncs.pfnSetModel)(TypeConversion.id_to_edict(index),(char*)STRING(ALLOC_STRING(temp)));
		return 1;


		// pfnModelIndex
	case 	EngFunc_ModelIndex:
		temp = MF_GetAmxString(amx,params[2],0,&len);
		return (*g_engfuncs.pfnModelIndex)(temp);


		// pfnModelFrames
	case	EngFunc_ModelFrames:	// int	)			(int modelIndex);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		return (*g_engfuncs.pfnModelFrames)(index);


		// pfnSetSize
	case	EngFunc_SetSize:		// void )				(edict_t *e, const float *rgflMin, const float *rgflMax);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		(*g_engfuncs.pfnSetSize)(TypeConversion.id_to_edict(index),Vec1,Vec2);
		return 1;


		// pfnChangeLevel (is this needed?)
	case		EngFunc_ChangeLevel:			// void )			(char* s1, char* s2);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		temp2 = MF_GetAmxStringNull(amx,params[3],1,&len);
		(*g_engfuncs.pfnChangeLevel)(temp,temp2);
		return 1;
		

		// pfnVecToYaw
	case		EngFunc_VecToYaw:			// float)				(const float *rgflVector);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		fparam1= (*g_engfuncs.pfnVecToYaw)(Vec1);
		cRet[0] = amx_ftoc(fparam1);
		return 1;


		// pfnVecToAngles
	case		EngFunc_VecToAngles:			// void )			(const float *rgflVectorIn, float *rgflVectorOut);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);

		(*g_engfuncs.pfnVecToAngles)(Vec1,Vec2);
		cRet = MF_GetAmxAddr(amx,params[3]);
		cRet[0]=amx_ftoc(Vec2[0]);
		cRet[1]=amx_ftoc(Vec2[1]);
		cRet[2]=amx_ftoc(Vec2[2]);
		return 1;


		// pfnMoveToOrigin
	case		EngFunc_MoveToOrigin:		// void )			(edict_t *ent, const float *pflGoal, float dist, int iMoveType);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		fparam1=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam1=cRet[0];
		CHECK_ENTITY(index);
		(*g_engfuncs.pfnMoveToOrigin)(TypeConversion.id_to_edict(index),Vec1,fparam1,iparam1);
		return 1;


		// pfnChangeYaw
	case		EngFunc_ChangeYaw:			// void )				(edict_t* ent);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		(*g_engfuncs.pfnChangeYaw)(TypeConversion.id_to_edict(index));
		return 1;


		// pfnChangePitch
	case		EngFunc_ChangePitch:			// void )			(edict_t* ent);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		(*g_engfuncs.pfnChangePitch)(TypeConversion.id_to_edict(index));
		return 1;


		// pfnFindEntityByString
	case		EngFunc_FindEntityByString:	// edict)	(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		temp = MF_GetAmxString(amx,params[3],0,&len);
		temp2 = MF_GetAmxString(amx,params[4],1,&len);
		pRet = (*g_engfuncs.pfnFindEntityByString)(index == -1 ? nullptr : TypeConversion.id_to_edict(index),temp,temp2);
		if (pRet)
			return ENTINDEX(pRet);
		return -1;


		// pfnGetEntityIllum
	case	EngFunc_GetEntityIllum:		// int	)		(edict_t* pEnt);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		return (*g_engfuncs.pfnGetEntityIllum)(TypeConversion.id_to_edict(index));


		// pfnFindEntityInSphere
	case 	EngFunc_FindEntityInSphere:	// edict)	(edict_t *pEdictStartSearchAfter, const float *org, float rad);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		fparam1 = amx_ctof(cRet[0]);
		pRet = (*g_engfuncs.pfnFindEntityInSphere)(index == -1 ? nullptr : TypeConversion.id_to_edict(index),Vec1,fparam1);
		if (pRet)
				return ENTINDEX(pRet);
		return -1;


		// pfnFindClientsInPVS
	case	EngFunc_FindClientInPVS:		// edict)		(edict_t *pEdict);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		pRet=(*g_engfuncs.pfnFindClientInPVS)(TypeConversion.id_to_edict(index));
		return ENTINDEX(pRet);


		// pfnEntitiesInPVS
	case	EngFunc_EntitiesInPVS:		// edict)			(edict_t *pplayer);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		pRet=(*g_engfuncs.pfnEntitiesInPVS)(TypeConversion.id_to_edict(index));
		return ENTINDEX(pRet);


		// pfnMakeVectors
	case	EngFunc_MakeVectors:			// void )			(const float *rgflVector);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		(*g_engfuncs.pfnMakeVectors)(Vec1);
		return 1;

		
		// pfnAngleVectors
	case	EngFunc_AngleVectors:		// void )			(const float *rgflVector, float *forward, float *right, float *up);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		(*g_engfuncs.pfnAngleVectors)(Vec1,Vec2,Vec3,Vec4);
		cRet = MF_GetAmxAddr(amx,params[3]);
		cRet[0] = amx_ftoc(Vec2[0]);
		cRet[1] = amx_ftoc(Vec2[1]);
		cRet[2] = amx_ftoc(Vec2[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		cRet[0] = amx_ftoc(Vec3[0]);
		cRet[1] = amx_ftoc(Vec3[1]);
		cRet[2] = amx_ftoc(Vec3[2]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		cRet[0] = amx_ftoc(Vec4[0]);
		cRet[1] = amx_ftoc(Vec4[1]);
		cRet[2] = amx_ftoc(Vec4[2]);
		return 1;


		// pfnCreateEntity
	case	EngFunc_CreateEntity:		// edict)			(void);
		pRet = (*g_engfuncs.pfnCreateEntity)();
		if (pRet)
			return ENTINDEX(pRet);
		return 0;


		// pfnRemoveEntity
	case	EngFunc_RemoveEntity:		// void )			(edict_t* e);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		if (index == 0)
			return 0;
		(*g_engfuncs.pfnRemoveEntity)(TypeConversion.id_to_edict(index));
		return 1;


		// pfnCreateNamedEntity
	case	EngFunc_CreateNamedEntity:	// edict)		(int className);
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		pRet = (*g_engfuncs.pfnCreateNamedEntity)(iparam1);
		if (pRet)
			return ENTINDEX(pRet);
		return 0;


		// pfnMakeStatic
	case	EngFunc_MakeStatic:			// void )			(edict_t *ent);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		(*g_engfuncs.pfnMakeStatic)(TypeConversion.id_to_edict(index));
		return 1;


		// pfnEntIsOnFloor
	case	EngFunc_EntIsOnFloor:		// int  )			(edict_t *e);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		return (*g_engfuncs.pfnEntIsOnFloor)(TypeConversion.id_to_edict(index));


		// pfnDropToFloor
	case	EngFunc_DropToFloor:			// int  )			(edict_t* e);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		return (*g_engfuncs.pfnDropToFloor)(TypeConversion.id_to_edict(index));


		// pfnWalkMove
	case	EngFunc_WalkMove:			// int  )				(edict_t *ent, float yaw, float dist, int iMode);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		fparam1 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		fparam2 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam1 = cRet[0];
		return (*g_engfuncs.pfnWalkMove)(TypeConversion.id_to_edict(index),fparam1,fparam2,iparam1);


		// pfnSetOrigin
	case	EngFunc_SetOrigin:			// void )				(edict_t *e, const float *rgflOrigin);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		(*g_engfuncs.pfnSetOrigin)(TypeConversion.id_to_edict(index),Vec1);
		return 1;


		// pfnEmitSound
	case	EngFunc_EmitSound:			// void )				(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1=cRet[0];
		temp = MF_GetAmxString(amx,params[4],0,&len);
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam1=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[6]);
		fparam2=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[7]);
		iparam2=cRet[0];
		cRet = MF_GetAmxAddr(amx,params[8]);
		iparam3=cRet[0];
		(*g_engfuncs.pfnEmitSound)(TypeConversion.id_to_edict(index),iparam1,temp,fparam1,fparam2,iparam2,iparam3);
		return 1;


		// pfnEmitAmbientSound
	case	EngFunc_EmitAmbientSound:	// void )		(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		temp = MF_GetAmxString(amx,params[4],0,&len);
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam1=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[6]);
		fparam2=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[7]);
		iparam1=cRet[0];
		cRet = MF_GetAmxAddr(amx,params[8]);
		iparam2=cRet[0];
		(*g_engfuncs.pfnEmitAmbientSound)(TypeConversion.id_to_edict(index),Vec1,temp,fparam1,fparam2,iparam1,iparam2);
		return 1;

		// pfnTraceLine
	case EngFunc_TraceLine:			// void )				(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam1=cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		index=cRet[0];
		TraceResult *tr;
		if (params[0] / sizeof(cell) == 6)
		{
			cell *ptr = MF_GetAmxAddr(amx, params[6]);
			if (*ptr == 0)
				tr = &g_tr_2;
			else
				tr = reinterpret_cast<TraceResult *>(*ptr);
		} else {
			tr = &g_tr;
		}
		(*g_engfuncs.pfnTraceLine)(Vec1,Vec2,iparam1,index != -1 ? TypeConversion.id_to_edict(index) : nullptr, tr);
		return 1;


		// pfnTraceToss
	case	EngFunc_TraceToss:			// void )				(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1 = cRet[0];
		CHECK_ENTITY(index);
		if (params[0] / sizeof(cell) == 4)
		{
			cell *ptr = MF_GetAmxAddr(amx, params[4]);
			if (*ptr == 0)
				tr = &g_tr_2;
			else
				tr = reinterpret_cast<TraceResult *>(*ptr);
		} else {
			tr = &g_tr;
		}
		(*g_engfuncs.pfnTraceToss)(TypeConversion.id_to_edict(index),iparam1 == -1 ? nullptr : TypeConversion.id_to_edict(iparam1),tr);
		return 1;


		// pfnTraceMonsterHull
	case	EngFunc_TraceMonsterHull:	// int  )		(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam1=cRet[0];
		cRet = MF_GetAmxAddr(amx,params[6]);
		iparam2=cRet[0];
		if (params[0] / sizeof(cell) == 7)
		{
			cell *ptr = MF_GetAmxAddr(amx, params[7]);
			if (*ptr == 0)
				tr = &g_tr_2;
			else
				tr = reinterpret_cast<TraceResult *>(*ptr);
		} else {
			tr = &g_tr;
		}
		(*g_engfuncs.pfnTraceMonsterHull)(TypeConversion.id_to_edict(index),Vec1,Vec2,iparam1,iparam2 == 0 ? nullptr : TypeConversion.id_to_edict(iparam2),tr);
		return 1;


		// pfnTraceHull
	case	EngFunc_TraceHull:			// void )				(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[6]);
		iparam3 = cRet[0];
		if (params[0] / sizeof(cell) == 7)
		{
			cell *ptr = MF_GetAmxAddr(amx, params[7]);
			if (*ptr == 0)
				tr = &g_tr_2;
			else
				tr = reinterpret_cast<TraceResult *>(*ptr);
		} else {
			tr = &g_tr;
		}
		(*g_engfuncs.pfnTraceHull)(Vec1,Vec2,iparam1,iparam2,iparam3 == 0 ? nullptr : TypeConversion.id_to_edict(iparam3),tr);
		return 1;


		// pfnTraceModel
	case	EngFunc_TraceModel:			// void )			(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam2 = cRet[0];
		if (params[0] / sizeof(cell) == 6)
		{
			cell *ptr = MF_GetAmxAddr(amx, params[6]);
			if (*ptr == 0)
				tr = &g_tr_2;
			else
				tr = reinterpret_cast<TraceResult *>(*ptr);
		} else {
			tr = &g_tr;
		}
		(*g_engfuncs.pfnTraceModel)(Vec1,Vec2,iparam1,iparam2 == 0 ? nullptr : TypeConversion.id_to_edict(iparam2),tr);
		return 1;


		// pfnTraceTexture
	case EngFunc_TraceTexture:		// const char *)			(edict_t *pTextureEntity, const float *v1, const float *v2 );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		temp = (char*)(*g_engfuncs.pfnTraceTexture)(TypeConversion.id_to_edict(index),Vec1,Vec2);
		cRet = MF_GetAmxAddr(amx,params[6]);
		MF_SetAmxString(amx, params[5], temp == nullptr ? "NoTexture" : temp, cRet[0]);
		return temp != nullptr;

		
		// pfnTraceSphere
	case EngFunc_TraceSphere:			// void )			(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam1 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[6]);
		index = cRet[0];
		(*g_engfuncs.pfnTraceSphere)(Vec1,Vec2,iparam1,fparam1,index == 0 ? nullptr : TypeConversion.id_to_edict(index),&g_tr);
		return 1;


		// pfnGetAimVector
	case	EngFunc_GetAimVector:		// void )			(edict_t* ent, float speed, float *rgflReturn);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		fparam1 = amx_ctof(cRet[0]);
		(*g_engfuncs.pfnGetAimVector)(TypeConversion.id_to_edict(index),fparam1,Vec1);
		cRet = MF_GetAmxAddr(amx,params[4]);
		cRet[0] = amx_ftoc(Vec1[0]);
		cRet[1] = amx_ftoc(Vec1[1]);
		cRet[2] = amx_ftoc(Vec1[2]);
		return 1;


		// pfnParticleEffect
	case	EngFunc_ParticleEffect:		// void )		(const float *org, const float *dir, float color, float count);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		fparam1=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam2=amx_ctof(cRet[0]);
		(*g_engfuncs.pfnParticleEffect)(Vec1,Vec2,fparam1,fparam2);
		return 1;

		
		// pfnLightStyle
	case	EngFunc_LightStyle:			// void )			(int style, const char* val);
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1=cRet[0];
		//different signedness: ‘int’ and ‘unsigned int’ [APG]RoboCop[CL]
		if (iparam1 < 0 || iparam1 >= ARRAYSIZE(LightStyleBuffers))
		{
			MF_LogError(amx, AMX_ERR_NATIVE, "Invalid style %d", iparam1);
			return 0;
		}
		LightStyleBuffers[iparam1] = MF_GetAmxString(amx, params[3], 0, &len);
		(*g_engfuncs.pfnLightStyle)(iparam1, LightStyleBuffers[iparam1].chars());
		return 1;


		// pfnDecalIndex
	case	EngFunc_DecalIndex:			// int  )			(const char *name);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		return (*g_engfuncs.pfnDecalIndex)(temp);


		// pfnPointContents
	case	EngFunc_PointContents:		// int )			(const float *rgflVector);
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		return (*g_engfuncs.pfnPointContents)(Vec1);


		// pfnFreeEntPrivateData
	case	EngFunc_FreeEntPrivateData:	// void )	(edict_t *pEdict);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		(*g_engfuncs.pfnFreeEntPrivateData)(TypeConversion.id_to_edict(index));
		return 1;


		// pfnSzFromIndex
	case	EngFunc_SzFromIndex:			// const char * )			(int iString);
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		temp = (char*)(*g_engfuncs.pfnSzFromIndex)(iparam1);
		cRet = MF_GetAmxAddr(amx,params[4]);
		MF_SetAmxString(amx, params[3], temp, cRet[0]);
		return 1;
		

		// pfnAllocString
	case	EngFunc_AllocString:			// int )			(const char *szValue);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		return (*g_engfuncs.pfnAllocString)((const char *)temp);


		// pfnRegUserMsg
	case	EngFunc_RegUserMsg:			// int	)			(const char *pszName, int iSize);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1 = cRet[0];
		return (*g_engfuncs.pfnRegUserMsg)(temp,iparam1);


		// pfnAnimationAutomove
	case	EngFunc_AnimationAutomove:	// void )		(const edict_t* pEdict, float flTime);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		fparam1 = amx_ctof(cRet[0]);
		(*g_engfuncs.pfnAnimationAutomove)(TypeConversion.id_to_edict(index),fparam1);
		return 1;


		// pfnGetBonePosition
	case	EngFunc_GetBonePosition:		// void )		(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1=cRet[0];
		(*g_engfuncs.pfnGetBonePosition)(TypeConversion.id_to_edict(index),iparam1,Vec1,Vec2);
		cRet = MF_GetAmxAddr(amx,params[4]);
		cRet[0]=amx_ftoc(Vec1[0]);
		cRet[1]=amx_ftoc(Vec1[1]);
		cRet[2]=amx_ftoc(Vec1[2]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		cRet[0]=amx_ftoc(Vec2[0]);
		cRet[1]=amx_ftoc(Vec2[1]);
		cRet[2]=amx_ftoc(Vec2[2]);
		return 1;


		// pfnGetAttachment
	case	EngFunc_GetAttachment:		// void	)			(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index=cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1=cRet[0];
		(*g_engfuncs.pfnGetAttachment)(TypeConversion.id_to_edict(index),iparam1,Vec1,Vec2);
		cRet = MF_GetAmxAddr(amx,params[4]);
		cRet[0]=amx_ftoc(Vec1[0]);
		cRet[1]=amx_ftoc(Vec1[1]);
		cRet[2]=amx_ftoc(Vec1[2]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		cRet[0]=amx_ftoc(Vec2[0]);
		cRet[1]=amx_ftoc(Vec2[1]);
		cRet[2]=amx_ftoc(Vec2[2]);
		return 1;


		// pfnSetView
	case	EngFunc_SetView:				// void )				(const edict_t *pClient, const edict_t *pViewent );
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam2 = cRet[0];
		CHECK_ENTITY(iparam1);
		CHECK_ENTITY(iparam2);
		(*g_engfuncs.pfnSetView)(TypeConversion.id_to_edict(iparam1),TypeConversion.id_to_edict(iparam2));
		return 1;


		// pfnTime
	case	EngFunc_Time:				// float)					( void );
		fparam1 = (*g_engfuncs.pfnTime)();
		return amx_ftoc(fparam1);


		// pfnCrosshairAngle
	case	EngFunc_CrosshairAngle:		// void )		(const edict_t *pClient, float pitch, float yaw);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		fparam1 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		fparam2 = amx_ctof(cRet[0]);
		(*g_engfuncs.pfnCrosshairAngle)(TypeConversion.id_to_edict(index),fparam1,fparam2);
		return 1;


		// pfnFadeClientVolume
	case	EngFunc_FadeClientVolume:	// void )      (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam3 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[6]);
		iparam4 = cRet[0];
		(*g_engfuncs.pfnFadeClientVolume)(TypeConversion.id_to_edict(index),iparam1,iparam2,iparam3,iparam4);
		return 1;


		// pfnSetClientMaxSpeed
	case	EngFunc_SetClientMaxspeed:	// void )     (const edict_t *pEdict, float fNewMaxspeed);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		fparam1 = amx_ctof(cRet[0]);
		(*g_engfuncs.pfnSetClientMaxspeed)(TypeConversion.id_to_edict(index),fparam1);
		return 1;


		// pfnCreateFakeClient
	case	EngFunc_CreateFakeClient:	// edict)		(const char *netname);	// returns NULL if fake client can't be created
		temp = MF_GetAmxString(amx,params[2],0,&len);
		pRet = (*g_engfuncs.pfnCreateFakeClient)(STRING(ALLOC_STRING(temp)));
		if (pRet == nullptr)
			return 0;
		return ENTINDEX(pRet);

		
		// pfnRunPlayerMove
	case	EngFunc_RunPlayerMove:		// void )			(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[4]);
		fparam1=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam2=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[6]);
		fparam3=amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[7]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[8]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[9]);
		iparam3 = cRet[0];
		(*g_engfuncs.pfnRunPlayerMove)(TypeConversion.id_to_edict(index),Vec1,fparam1,fparam2,fparam3,iparam1,iparam2,iparam3);
		return 1;


		// pfnNumberOfEntities
	case	EngFunc_NumberOfEntities:	// int  )		(void);
		return (*g_engfuncs.pfnNumberOfEntities)();


		// pfnStaticDecal
	case	EngFunc_StaticDecal:			// void )			( const float *origin, int decalIndex, int entityIndex, int modelIndex );
		cRet = MF_GetAmxAddr(amx,params[2]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam3 = cRet[0];
		(*g_engfuncs.pfnStaticDecal)(Vec1,iparam1,iparam2,iparam3);
		return 1;


		// pfnPrecacheGeneric
	case	EngFunc_PrecacheGeneric:		// int  )		(char* s);
		temp = MF_GetAmxString(amx,params[2],0,&len);
		return (*g_engfuncs.pfnPrecacheGeneric)((char*)STRING(ALLOC_STRING(temp)));


		// pfnBuildSoundMsg
	case	EngFunc_BuildSoundMsg:		// void )			(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam1 = cRet[0];
		temp = MF_GetAmxString(amx,params[4],0,&len);
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam1 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[6]);
		fparam2 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[7]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[8]);
		iparam3 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[9]);
		iparam4 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[10]);
		iparam5 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[11]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet=MF_GetAmxAddr(amx,params[12]);
		iparam6=cRet[0];
		/* don't check, it might not be included 
		CHECK_ENTITY(iparam5); 
		*/
		(*g_engfuncs.pfnBuildSoundMsg)(TypeConversion.id_to_edict(index),iparam1,temp,fparam1,fparam2,iparam2,iparam3,iparam4,iparam5,Vec1,iparam6 == 0 ? nullptr : TypeConversion.id_to_edict(iparam6));
		return 1;


		// pfnGetPhysicsKeyValue
	case	EngFunc_GetPhysicsKeyValue:	// const char* )	( const edict_t *pClient, const char *key );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		temp = MF_GetAmxString(amx,params[3],0,&len);
		temp2 = (char*)(*g_engfuncs.pfnGetPhysicsKeyValue)(TypeConversion.id_to_edict(index),(const char *)temp);
		cRet = MF_GetAmxAddr(amx,params[5]);
		MF_SetAmxString(amx,params[4],temp2,cRet[0]);
		return 1;


		// pfnSetPhysicsKeyValue
	case	EngFunc_SetPhysicsKeyValue:	// void )	( const edict_t *pClient, const char *key, const char *value );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		temp = MF_GetAmxString(amx,params[3],0,&len);
		temp2 = MF_GetAmxString(amx,params[4],1,&len);
		(*g_engfuncs.pfnSetPhysicsKeyValue)(TypeConversion.id_to_edict(index),STRING(ALLOC_STRING(temp)),STRING(ALLOC_STRING(temp2)));
		return 1;


		// pfnGetPhysicsInfoString
	case	EngFunc_GetPhysicsInfoString:	// const char* )	( const edict_t *pClient );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		temp = (char*)(*g_engfuncs.pfnGetPhysicsInfoString)(TypeConversion.id_to_edict(index));
		cRet = MF_GetAmxAddr(amx,params[4]);

		MF_SetAmxString(amx,params[3],temp,cRet[0]);
		return 1;


		// pfnPrecacheEvent
	case	EngFunc_PrecacheEvent:		// unsigned short )		( int type, const char*psz );
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		temp = MF_GetAmxString(amx,params[3],0,&len);
		return (*g_engfuncs.pfnPrecacheEvent)(iparam1,(char*)STRING(ALLOC_STRING(temp)));


		// pfnPlaybackEvent (grr)
	case	EngFunc_PlaybackEvent:		// void )			
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		index = cRet[0];
		if (index != -1)
		{
			CHECK_ENTITY(index);
			pRet = TypeConversion.id_to_edict(index);
		}
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[5]);
		fparam1 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[6]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[7]);
		Vec2[0]=amx_ctof(cRet[0]);
		Vec2[1]=amx_ctof(cRet[1]);
		Vec2[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[8]);
		fparam2 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[9]);
		fparam3 = amx_ctof(cRet[0]);
		cRet = MF_GetAmxAddr(amx,params[10]);
		iparam3 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[11]);
		iparam4 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[12]);
		iparam5 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[13]);
		iparam6 = cRet[0];
		(*g_engfuncs.pfnPlaybackEvent)(iparam1,pRet,iparam2,fparam1,Vec1,Vec2,fparam2,fparam3,iparam3,iparam4,iparam5,iparam6);
		return 1;

		//pfnCheckVisibility
	case	EngFunc_CheckVisibility:			// int )		( const edict_t *entity, unsigned char *pset );
		cRet = MF_GetAmxAddr(amx, params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		cRet = MF_GetAmxAddr(amx, params[3]);
		pset = (unsigned char *)cRet[0];
		return (*g_engfuncs.pfnCheckVisibility)(TypeConversion.id_to_edict(index), pset);

		// pfnGetCurrentPlayer
	case	EngFunc_GetCurrentPlayer:			// int )		( void );
		return (*g_engfuncs.pfnGetCurrentPlayer)();


		// pfnCanSkipPlayer
	case	EngFunc_CanSkipPlayer:			// int )			( const edict_t *player );
		cRet = MF_GetAmxAddr(amx,params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);
		return (*g_engfuncs.pfnCanSkipPlayer)(TypeConversion.id_to_edict(index));


		// pfnSetGroupMask
	case	EngFunc_SetGroupMask:				// void )			( int mask, int op );
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam2 = cRet[0];
		(*g_engfuncs.pfnSetGroupMask)(iparam1,iparam2);
		return 1;


		// pfnGetClientListening
	case	EngFunc_GetClientListening:	// bool (int iReceiver, int iSender)
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam2 = cRet[0];
		return (*g_engfuncs.pfnVoice_GetClientListening)(iparam1,iparam2);


		// pfnSetClientListening
	case	EngFunc_SetClientListening:	// bool (int iReceiver, int iSender, bool Listen)
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[4]);
		iparam3 = cRet[0];
		return (*g_engfuncs.pfnVoice_SetClientListening)(iparam1,iparam2,iparam3);


		// pfnMessageBegin (AMX doesn't support MSG_ONE_UNRELIABLE, so I should add this incase anyone needs it.)
	case	EngFunc_MessageBegin:	// void (int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
		cRet = MF_GetAmxAddr(amx,params[2]);
		iparam1 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[3]);
		iparam2 = cRet[0];
		cRet = MF_GetAmxAddr(amx,params[4]);
		Vec1[0]=amx_ctof(cRet[0]);
		Vec1[1]=amx_ctof(cRet[1]);
		Vec1[2]=amx_ctof(cRet[2]);
		cRet = MF_GetAmxAddr(amx,params[5]);
		index = cRet[0];
		(*g_engfuncs.pfnMessageBegin)(iparam1,iparam2,Vec1,index == 0 ? nullptr : TypeConversion.id_to_edict(index));
		return 1;


		// pfnWriteCoord
	case	EngFunc_WriteCoord:		// void (float)
		cRet = MF_GetAmxAddr(amx,params[2]);
		fparam1 = amx_ctof(cRet[0]);
		(*g_engfuncs.pfnWriteCoord)(fparam1);
		return 1;


		// pfnWriteAngle
	case	EngFunc_WriteAngle:		// void (float)
		cRet = MF_GetAmxAddr(amx,params[2]);
		fparam1 = amx_ctof(cRet[0]);
		(*g_engfuncs.pfnWriteAngle)(fparam1);
		return 1;
	case	EngFunc_InfoKeyValue:	// char*	)			(char *infobuffer, char *key);
		cRet = MF_GetAmxAddr(amx,params[2]);
		temp = reinterpret_cast<char *>(cRet[0]);
		temp2 = MF_GetAmxString(amx,params[3],0,&len);

		temp = (*g_engfuncs.pfnInfoKeyValue)(temp, temp2);

		cRet = MF_GetAmxAddr(amx,params[5]);
		iparam1 = cRet[0];
		MF_SetAmxString(amx,params[4],temp,iparam1);

		return 1;

	case	EngFunc_SetKeyValue:	// void )			(char *infobuffer, char *key, char *value);
		cRet = MF_GetAmxAddr(amx, params[2]);
		temp3 = reinterpret_cast<char *>(cRet[0]);
		temp = MF_GetAmxString(amx, params[3], 1, &len);
		temp2 = MF_GetAmxString(amx, params[4], 2, &len);
		(*g_engfuncs.pfnSetKeyValue)(temp3, temp, temp2);
		return 1;

	case	EngFunc_SetClientKeyValue:	 // void )		(int clientIndex, char *infobuffer, char *key, char *value);
		cRet = MF_GetAmxAddr(amx, params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);

		cRet = MF_GetAmxAddr(amx, params[3]);
		temp = reinterpret_cast<char *>(cRet[0]);

		temp2 = MF_GetAmxString(amx, params[4], 0, &len);
		temp3 = MF_GetAmxString(amx, params[5], 1, &len);
		(*g_engfuncs.pfnSetClientKeyValue)(index, temp, temp2, temp3);
		return 1;
	case EngFunc_CreateInstancedBaseline:	// int )		(int classname, struct entity_state_s *baseline);
		cRet = MF_GetAmxAddr(amx, params[2]);
		iparam1 = cRet[0];

		entity_state_t *es;	

		cRet = MF_GetAmxAddr(amx, params[3]);

		if (*cRet == 0)
			es = &g_es_glb;
		else
			es = reinterpret_cast<entity_state_t *>(*cRet);

		return (*g_engfuncs.pfnCreateInstancedBaseline)(iparam1, es);
	case EngFunc_GetInfoKeyBuffer:		// char *)			(edict_t *e);
		cRet = MF_GetAmxAddr(amx, params[2]);
		index = cRet[0];

		if (index != -1)
		{
			CHECK_ENTITY(index);
		}

		temp = (*g_engfuncs.pfnGetInfoKeyBuffer)(index == -1 ? nullptr : TypeConversion.id_to_edict(index));
		return reinterpret_cast<cell>(temp);
	case EngFunc_AlertMessage:			// void )			(ALERT_TYPE atype, char *szFmt, ...);
		cRet = MF_GetAmxAddr(amx, params[2]);
		iparam1 = cRet[0];
		temp = MF_FormatAmxString(amx, params, 3, &len);
		
		(*g_engfuncs.pfnAlertMessage)(static_cast<ALERT_TYPE>(iparam1), temp);
		return 1;
	case EngFunc_ClientPrintf:			// void )			(edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg);
		cRet = MF_GetAmxAddr(amx, params[2]);
		index = cRet[0];
		CHECK_ENTITY(index);

		cRet = MF_GetAmxAddr(amx, params[3]);
		iparam1 = cRet[0];
		temp = MF_GetAmxString(amx,params[4], 0, &len);

		(*g_engfuncs.pfnClientPrintf)(TypeConversion.id_to_edict(index), static_cast<PRINT_TYPE>(iparam1), temp);
		return 1;
	case EngFunc_ServerPrint:			// void )			(const char *szMsg);
		temp = MF_GetAmxString(amx, params[2], 0, &len);

		(*g_engfuncs.pfnServerPrint)(temp);
		return 1;
	default:
		MF_LogError(amx, AMX_ERR_NATIVE, "Unknown engfunc type %d", type);
		return 0;
	}
}

AMX_NATIVE_INFO engfunc_natives[] = {
	{"engfunc",			engfunc},
	{nullptr, nullptr},
};

