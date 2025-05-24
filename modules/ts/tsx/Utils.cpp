// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
// Copyright (C) 2004 Lukasz Wlasinski.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

//
// TSX Module
//

#include "amxxmodule.h"
#include "tsx.h"

weapon_t weaponData[] = { 
	{ true,"Kung Fu","kung_fu",3 }, // id 0 like in WeaponInfo , DeathMsg 
	{ false,"Glock-18","glock-18",1 },
	{ false,"Unk1","Unk1",0 }, // bomb ?
	{ false,"Mini-Uzi","mini-uzi",1 }, 
	{ false,"BENELLI-M3","benelli_m3",1 }, 
	{ false,"M4A1","m4a1",1 },
	{ false,"MP5SD","mp5sd",1 },
	{ false,"MP5K","mp5k",1 },
	{ false,"Akimbo Berettas","akimbo_berettas",1 },
	{ false,"SOCOM-MK23","socom-mk23",1 },
	{ false,"Akimbo MK23","akimbo_mk23",1 },
	{ false,"USAS-12","usas-12",1 },
	{ false,"Desert Eagle","desert_eagle",1 },
	{ false,"AK47","ak47",1 },
	{ false,"Five-seveN","five-seven",1 },
	{ false,"STEYR-AUG","steyr-aug",1 },
	{ false,"Akimbo Mini-Uzi","akimbo_mini-uzi",1 },
	{ false,"STEYR-TMP","steyr-tmp",1 },
	{ false,"Barrett M82A1","barrett_m82a1",1 },
	{ false,"MP7-PDW","mp7-pdw",1 }, 
	{ false,"SPAS-12","spas-12",1 },
	{ false,"Golden Colts","golden_colts",1 },
	{ false,"Glock-20C","glock-20c",1 },
	{ false,"UMP", "ump",1 }, 
	{ false,"M61 Grenade","m61_grenade",1 },
	{ true,"Combat Knife","combat_knife",1 },
	{ false,"MOSSBERG 500","mossberg_500",1 },
	{ false,"M16A4","m16a4",1 },
	{ false,"Ruger-MK1","ruger-mk1",1 },
	{ false,"C4","c4",0 },
	{ false,"Akimbo Five-seveN","akimbo_five-seven",1 },
	{ false,"Raging Bull","raging_bull",1 },
	{ false,"M60E3","m60e3",1 }, 
	{ false,"Sawed-off","sawed-off",1 }, // 33 
	{ true,"Katana", "katana",2 }, 
	{ true,"Seal Knife","seal_knife",1 }, // 35 
	{ true,"Kung Fu","kung_fu",3 }, // Again new id 36 
	{ true,"Throwing Knife","throwing_knife",2 }, // new id 37 
	{ false,"breakable", "breakable", 1 },
};

bool ignoreBots (edict_t *pEnt, edict_t *pOther){
	if ( !rankBots && ( pEnt->v.flags & FL_FAKECLIENT || ( pOther && pOther->v.flags & FL_FAKECLIENT ) ) )
		return true;
	return false;
}

bool isModuleActive(){
	if ( !(int)CVAR_GET_FLOAT("tsstats_pause") )
		return true;
	return false;
}
