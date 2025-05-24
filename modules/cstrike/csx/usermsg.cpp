// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// CSX Module
//

#include "amxxmodule.h"
#include "rank.h"

weaponsVault weaponData[MAX_WEAPONS+MAX_CWEAPONS];

int damage;
int TA;
int weapon;
int aim;
bool ignore;
CPlayer *pAttacker;


void Client_ResetHUD(void* mValue){
	if ( mPlayer ){ 
		mPlayer->clearStats = gpGlobals->time + 0.25f;
	}
}

void Client_DeathMsg(void *mValue)
{
	static int killer_id;
	//static int victim_id;
	static int is_headshot;
	const char *name;

	switch (mState++)
	{
	case 0:
		{
			killer_id = *static_cast<int*>(mValue);
			break;
		}
	//case 1:
	//	{
	//		victim_id = *(int *)mValue;
	//		break;
	//	}
	case 2:
		{
			is_headshot = *static_cast<int*>(mValue);
			break;
		}
	case 3:
		{
			name = static_cast<const char*>(mValue);
			if (killer_id 
				&& (strcmp(name, "knife") == 0))
			{
				CPlayer *pPlayer = GET_PLAYER_POINTER_I(killer_id);
				pPlayer->aiming = is_headshot ? 1 : 0;
			}
			break;
		}
	}
}

void Client_WeaponList(void* mValue){
  static int wpnList;
  static int iSlot;
  static const char* wpnName;

  switch (mState++) {
  case 0:
    wpnName = static_cast<const char*>(mValue);
    break;
  case 1:
    iSlot = *static_cast<int*>(mValue);
    break;
  case 7:
	  const int iId = *static_cast<int*>(mValue);
    if ( (iId < 0 || iId >= MAX_WEAPONS ) || ( wpnList & (1<<iId) ) )
      break;

    wpnList |= (1<<iId);
    weaponData[iId].ammoSlot = iSlot;

	if ( strstr( wpnName,"weapon_") )
	{
		if ( strcmp(wpnName+7,"hegrenade") == 0 )
			strcpy(weaponData[iId].name,wpnName+9);
		else
			strcpy(weaponData[iId].name,wpnName+7);
		strcpy(weaponData[iId].logname,weaponData[iId].name);
	}
  } 
}

void Client_Damage(void* mValue){
  static int bits;
  switch (mState++) {
  case 1: 
	ignore = false;
    damage = *static_cast<int*>(mValue);
    break;
  case 2:
    bits = *static_cast<int*>(mValue);
    break;
  case 3:
	  if (!mPlayer || !damage || bits){
	    ignore = true;
		break;
	  }

    edict_t *enemy;
	enemy = mPlayer->pEdict->v.dmg_inflictor;
    
	if ( FNullEnt( enemy ) ){
		ignore = true;
		break;
	}

	aim = 0;
	weapon = 0;
	pAttacker = nullptr;

	if (enemy->v.flags & (FL_CLIENT | FL_FAKECLIENT) ) {
		pAttacker = GET_PLAYER_POINTER(enemy);
		aim = pAttacker->aiming;
		weapon = pAttacker->current;
		pAttacker->saveHit( mPlayer , weapon , damage, aim);
		break;
	}
    if( g_grenades.find(enemy , &pAttacker , &weapon ) )
        pAttacker->saveHit( mPlayer , weapon , damage, aim );
	else if ( strcmp("grenade",STRING(enemy->v.classname))==0 ) // ? more checks ?
			weapon = CSW_C4;
  }
}

void Client_Damage_End(void* mValue){
	if ( ignore )
		return;

	if ( !pAttacker ) pAttacker = mPlayer;
		TA = 0;
	if ( (mPlayer->teamId == pAttacker->teamId) && (mPlayer != pAttacker) )
		TA = 1;

	MF_ExecuteForward( iFDamage, static_cast<cell>(pAttacker->index) , static_cast<cell>(mPlayer->index) ,
		static_cast<cell>(damage), static_cast<cell>(weapon), static_cast<cell>(aim), static_cast<cell>(TA) );
	 
	if ( !mPlayer->IsAlive() ){
		if ( weapon != CSW_C4 )
			pAttacker->saveKill(mPlayer,weapon,( aim == 1 ) ? 1:0 ,TA);
		MF_ExecuteForward( iFDeath, static_cast<cell>(pAttacker->index), static_cast<cell>(mPlayer->index),
			static_cast<cell>(weapon), static_cast<cell>(aim), static_cast<cell>(TA) );
	}
}

void Client_CurWeapon(void* mValue){
  static int iState;
  static int iId;
  switch (mState++){
  case 0: 
    iState = *static_cast<int*>(mValue);
    break;
  case 1:
    if (!iState) break; 
    iId = *static_cast<int*>(mValue);
    break;
  case 2:
	if (!mPlayer || !iState ) break;
    const int iClip = *static_cast<int*>(mValue);
    if ((iClip > -1) && (iClip < mPlayer->weapons[iId].clip)) 
      mPlayer->saveShot(iId);
    mPlayer->weapons[iId].clip = iClip;
	mPlayer->current = iId;
  }
}

void Client_AmmoX(void* mValue){
  static int iAmmo;
  switch (mState++){
  case 0:
    iAmmo = *static_cast<int*>(mValue);
    break;
  case 1:
	if (!mPlayer ) break;
    for(int i = 1; i < MAX_WEAPONS ; ++i) 
      if (iAmmo == weaponData[i].ammoSlot)
        mPlayer->weapons[i].ammo = *static_cast<int*>(mValue);
  }
}

void Client_AmmoPickup(void* mValue){
  static int iSlot;
  switch (mState++){
  case 0:
    iSlot = *static_cast<int*>(mValue);
    break;
  case 1:
	if (!mPlayer ) break;
    for(int i = 1; i < MAX_WEAPONS ; ++i)
      if (weaponData[i].ammoSlot == iSlot)
        mPlayer->weapons[i].ammo += *static_cast<int*>(mValue);
  }
}

void Client_ScoreInfo(void* mValue){
  static int index;
  switch (mState++){
  case 0:
    index = *static_cast<int*>(mValue);
    break;
  case 4:
	if ( index > 0 && index <= gpGlobals->maxClients )
		GET_PLAYER_POINTER_I( index )->teamId = *static_cast<int*>(mValue);
  }
}

void Client_SendAudio(void* mValue){
	static const char* szText;
	if ( mState == 1 ){
		szText = static_cast<const char*>(mValue);
		if ( !mPlayer && szText[7]=='B' ) {
			if ( szText[11]=='P' && g_Planter ){
				GET_PLAYER_POINTER_I(g_Planter)->saveBPlant();
				g_bombAnnounce = BOMB_PLANTED;
			}
			else if ( szText[11]=='D' && g_Defuser ){
				GET_PLAYER_POINTER_I(g_Defuser)->saveBDefused();
				g_bombAnnounce = BOMB_DEFUSED;
			}

		}
	}
	mState++;
}

void Client_TextMsg(void* mValue){
	static const char* szText;
	if ( !mPlayer && mState==1 ){
		szText = static_cast<const char*>(mValue);
		if ( szText[1]=='T' && szText[8]=='B' && g_Planter ){
			GET_PLAYER_POINTER_I(g_Planter)->saveBExplode();
			g_bombAnnounce = BOMB_EXPLODE;
		}
	}
	mState++;
}

void Client_BarTime(void* mValue){
	const int iTime = *static_cast<int*>(mValue);
	if ( !iTime || !mPlayer->IsAlive() ) return;
	if ( iTime == 3 ){
		g_Planter = mPlayerIndex;
		g_bombAnnounce = BOMB_PLANTING;
		g_Defuser = 0;
	}
	else {
		mPlayer->saveBDefusing();
		g_Defuser = mPlayerIndex;
		g_bombAnnounce = BOMB_DEFUSING;
	}
}

