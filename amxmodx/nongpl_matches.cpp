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

#include <cstring>
#include "nongpl_matches.h"

NONGPL_PLUGIN_T NONGPL_PLUGIN_LIST[] = 
{
	{"Live", "CZ Gun Game", "czgungame.amxx"},
	{"Live", "AMXX Gun Game", "czgungame.amxx"},
	{nullptr, nullptr, nullptr},
};

NONGPL_CVAR_T NONGPL_CVAR_LIST[] = 
{
	{"gg_mode", 0},
	{"gg_warmuptimer", 0},
	{"gg_ff", 0},
	{"gg_fflevel", 0},
	{"gg_stats", 0},
	{"gg_dm", 0},
	{"gg_turbo", 0},
	{"amx_ggreset", 1},
	{"amx_gg", 1},
	{nullptr, 0},
};

bool CheckBadConList(const char *cvar, int type)
{
	int i = 0;
	while (NONGPL_CVAR_LIST[i].cvar != nullptr)
	{
		if (NONGPL_CVAR_LIST[i].type == type
			&& strcmp(NONGPL_CVAR_LIST[i].cvar, cvar) == 0)
		{
			return true;
		}
		i++;
	}

	return false;
}
