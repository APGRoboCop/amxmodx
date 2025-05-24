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

#include "amxmodx.h"
#include "CMenu.h"

int gmsgAmmoPickup;
int gmsgAmmoX;
int gmsgBattery;
int gmsgCurWeapon;
int gmsgDamage;
int gmsgDeathMsg;
int gmsgHealth;
int gmsgMOTD;
int gmsgScoreInfo;
int gmsgSendAudio;
int gmsgServerName;
int gmsgShowMenu;
int gmsgTeamInfo;
int gmsgTextMsg;
int gmsgVGUIMenu;
int gmsgWeapPickup;
int gmsgWeaponList;
int gmsgintermission;
int gmsgResetHUD;
int gmsgRoundTime;
int gmsgSayText;
int gmsgInitHUD;

TeamIds g_teamsIds;
WeaponsVault g_weaponsData[MAX_WEAPONS];

void Client_VGUIMenu(void* mValue)
{
	if (!mPlayer) return;

	mPlayer->vgui = true;

	switch (mState++)
	{
		case 0:
			mPlayer->menu = -(*static_cast<int*>(mValue));
			mPlayer->newmenu = -1;
			break;
		case 1:
			mPlayer->keys = *static_cast<int*>(mValue);
	}
}

void Client_ShowMenu(void* mValue)
{
	if (!mPlayer) return;

	mPlayer->vgui = true;

	switch (mState++)
	{
		case 0:
			mPlayer->keys = *static_cast<int*>(mValue);
			break;
		case 3:
			{
			mPlayer->menu = g_menucmds.findMenuId(static_cast<char*>(mValue));
			mPlayer->newmenu = -1;
			break;
			}
	}
}

extern bool g_bmod_tfc;
void Client_TeamInfo(void* mValue)
{
	if (mPlayer && !g_bmod_tfc) return;
	static int index;

	switch (mState++)
	{
		case 0:
			index = *static_cast<int*>(mValue);
			break;
		case 1:
			if (index < 1 || index > gpGlobals->maxClients) break;
			const char* msg = static_cast<char*>(mValue);
			if (!msg) break;

			const auto pPlayer = GET_PLAYER_POINTER_I(index);
		
			pPlayer->team = msg;
			g_teamsIds.registerTeam(msg, -1);
			pPlayer->teamId = g_teamsIds.findTeamId(msg);
						
			if (pPlayer->teamId == -1)
			{
				/**
				 * CS fix for SPECTATOR team. 
				 * -
				 * When a player chooses spectator, ScoreInfo is sent before TeamInfo and with 0 as index.
				 * This means for the first round of first spectator, SPECTATOR name is not associated with its index.
				 *  The following fix manually sets the team index when we hit SPECTATOR team.
				 */
				if (g_bmod_cstrike && !strcmp(msg, "SPECTATOR"))
				{
					pPlayer->teamId = 3;
					g_teamsIds.registerTeam(msg, 3);
				}

				/**
				 * Fixes in-between situation where the team name is not yet associated with a valid index
				 * and ScoreInfo is executed later (used to retrieve the index). E.g. a player is dead,
				 * then changes team. Index will return -1 until ScoreInfo is sent, usually at the next spawn.
				 */
				else if ((g_bmod_cstrike || g_bmod_dod || g_bmod_tfc || g_bmod_gearbox || g_bmod_valve) 
					&&  pPlayer->pEdict->pvPrivateData 
					&& !pPlayer->IsAlive())
				{
					GET_OFFSET_NO_ERROR("CBasePlayer", m_iTeam)

					const auto teamId = get_pdata<int>(pPlayer->pEdict, m_iTeam);
					
					pPlayer->teamId = teamId;
					g_teamsIds.registerTeam(msg, teamId);
				}
			}

			break;
	}
}

void Client_TextMsg(void* mValue)
{
	if (mPlayer) return;
	
	switch (mState++)
	{
		case 1:
		{
			const char * msg = static_cast<char*>(mValue);
			if (!msg) break;
			
			if (!strncmp("#Game_C", msg, 7))
			{
				g_game_timeleft = g_game_restarting = gpGlobals->time + 3;
				//g_endround_time = gpGlobals->time;
				//g_newround_time = gpGlobals->time + CVAR_GET_FLOAT("mp_freezetime") + 3;
			}
			else if (!strncmp("#Game_w", msg, 7))
			{
				g_game_timeleft = -2;
			}
			else if (!strncmp("#game_clan_s", msg, 12))
			{
				g_game_timeleft = -3;
			}
			
			break;
		}
		case 2:
		{
			const char * msg = static_cast<char*>(mValue);
			if (!msg) break;
			
			if (g_game_timeleft == -2)
			{
				g_game_timeleft = g_game_restarting = gpGlobals->time + atoi(msg);
				//g_newround_time = g_game_timeleft + CVAR_GET_FLOAT("mp_freezetime");
			}
			else if (g_game_timeleft == -3)
				g_game_restarting = atoi(msg) * 60.0f;
			
			break;
		}
		case 3:
		{
			const char * msg = static_cast<char*>(mValue);
			if (!msg) break;
			if (g_game_timeleft != -3) break;
			g_game_restarting += atoi(msg);
			g_game_timeleft = g_game_restarting	= gpGlobals->time + g_game_restarting;
			
			break;
		}
	}
}

void Client_WeaponList(void* mValue)
{
	static int wpnList = 0;
	//static int wpnList2;
	static int iSlot;
	static const char* wpnName;
	
	switch (mState++)
	{
		case 0:
			wpnName = static_cast<char*>(mValue);
			break;
		case 1:
			iSlot = *static_cast<int*>(mValue);
			break;
		case 7:
			const int iId = *static_cast<int*>(mValue);
			if ((iId < 0 || iId >= MAX_WEAPONS) || (wpnList & (1<<iId)))
			break;
			wpnList |= (1<<iId);
			g_weaponsData[iId].iId = iId;
			g_weaponsData[iId].ammoSlot = iSlot;
			g_weaponsData[iId].fullName = wpnName;
	}
}

void Client_CurWeapon(void* mValue)
{
	static int iState;
	static int iId;
	
	switch (mState++)
	{
		case 0:
			iState = *static_cast<int*>(mValue);
			break;
		case 1:
			if (!iState) break;
			iId = *static_cast<int*>(mValue);
			break;
		case 2:
			if (!mPlayer) return;
			if (!iState || (iId < 1 || iId >= MAX_WEAPONS)) break;
			mPlayer->current = iId;

			
			if (*static_cast<int*>(mValue) < mPlayer->weapons[iId].clip && // Only update the lastHit vector if the clip size is decreasing
				*static_cast<int*>(mValue) != -1) // But not if it's a melee weapon
			{
				mPlayer->lastHit = mPlayer->lastTrace;
			}
			mPlayer->weapons[iId].clip = *static_cast<int*>(mValue);
	}
}

void Client_AmmoX(void* mValue)
{
	static int iAmmo;
	
	switch (mState++)
	{
		case 0:
			iAmmo = *static_cast<int*>(mValue);
			break;
		case 1:
			if (!mPlayer) return;
			for (int i = 1; i < MAX_WEAPONS; ++i)
				if (iAmmo == g_weaponsData[i].ammoSlot)
					mPlayer->weapons[i].ammo = *static_cast<int*>(mValue);
	}
}

void Client_AmmoPickup(void* mValue)
{
	static int iSlot;
	
	switch (mState++)
	{
		case 0:
			iSlot = *static_cast<int*>(mValue);
			break;
		case 1:
			if (!mPlayer) return;
			for (int i = 1; i < MAX_WEAPONS; ++i)
				if (g_weaponsData[i].ammoSlot==iSlot)
					mPlayer->weapons[i].ammo += *static_cast<int*>(mValue);
	}
}

void Client_ScoreInfo(void* mValue)
{
	static int index;
	static int deaths;

	switch (mState++)
	{
		case 0:
			index = *static_cast<int*>(mValue);
			break;
		case 2:
			deaths = *static_cast<int*>(mValue);
			break;
		case 4:
			if (index < 1 || index > gpGlobals->maxClients) break;
			CPlayer*pPlayer = GET_PLAYER_POINTER_I(index);
			pPlayer->deaths = deaths;
			pPlayer->teamId = *static_cast<int*>(mValue);
			if (g_teamsIds.isNewTeam())
				g_teamsIds.registerTeam(pPlayer->team.chars(), pPlayer->teamId);
	}
}

void Client_DamageEnd(void* mValue)
{
	CPlayer* dead = mPlayer;

	if (dead && dead->death_killer)
	{
		g_events.parserInit(CS_DEATHMSG, &gpGlobals->time, mPlayer = nullptr, mPlayerIndex = 0);
		g_events.parseValue(dead->death_killer);
		g_events.parseValue(dead->index);
		g_events.parseValue(dead->death_headshot);
		g_events.parseValue(dead->death_weapon.chars());
		g_events.parseValue(dead->death_tk ? 1 : 0);
		g_events.executeEvents();
		dead->death_killer = 0;
	}
}

void Client_DeathMsg(void* mValue)
{
	static CPlayer *killer;
	static CPlayer *victim;
	static int killer_id;
	static int victim_id;
	static int hs;
	
	switch (mState++)
	{
		case 0:
			killer_id = *static_cast<int*>(mValue);
			killer = (killer_id > 0 && killer_id < 33) ? GET_PLAYER_POINTER_I(killer_id) : nullptr;
			break;
		case 1:
			victim_id = *static_cast<int*>(mValue);
			victim = (victim_id > 0 && victim_id < 33) ? GET_PLAYER_POINTER_I(victim_id) : nullptr;
			break;
		case 2:	
			hs = *static_cast<int*>(mValue);
			break;
		case 3:
			if (!killer || !victim) break;
			victim->death_killer = killer_id;
			victim->death_weapon = static_cast<char*>(mValue);
			victim->death_headshot = hs;
			victim->death_tk = (killer->teamId == victim->teamId);
	}
}

void Client_InitHUDEnd(void* mValue)
{
	if (!g_bmod_cstrike)
		return;

	CPlayer *pPlayer = mPlayer;

	if (!pPlayer->teamIdsInitialized && !pPlayer->IsBot())
	{
		// This creates specific indexes (> maxplayers) for print_chat_color().
		// 33 : print_team_grey / spectator
		// 34 : print_team_red  / terrorist
		// 35 : print_team_blue / ct
		UTIL_TeamInfo(pPlayer->pEdict, 33 + 1, "TERRORIST"); // print_team_red
		UTIL_TeamInfo(pPlayer->pEdict, 33 + 2, "CT");		 // print_team_blue
		pPlayer->teamIdsInitialized = true;
	}
}

/*
void Client_SendAudio(void* mValue)
{

}

void Client_SendAudioEnd(void* mValue)
{


}

void Client_RoundTimeEnd(void* mValue)
{

}

void Client_RoundTime(void* mValue)
{

}


void Client_ResetHUD(void* mValue)
{

}
*/
