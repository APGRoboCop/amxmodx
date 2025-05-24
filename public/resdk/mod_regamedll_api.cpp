// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "mod_regamedll_api.h"

IReGameApi*          ReGameApi;
const ReGameFuncs_t* ReGameFuncs;
IReGameHookchains *  ReGameHookchains;

bool RegamedllApi_Init()
{
	const auto library = GET_GAME_INFO(PLID, GINFO_DLL_FULLPATH);

	if (!library || !GET_IFACE<IReGameApi>(library, ReGameApi, VRE_GAMEDLL_API_VERSION, false) || !ReGameApi)
	{
		return false;
	}

	const int majorVersion = ReGameApi->GetMajorVersion();
	const int minorVersion = ReGameApi->GetMinorVersion();

	if (majorVersion != REGAMEDLL_API_VERSION_MAJOR || minorVersion < REGAMEDLL_API_VERSION_MINOR)
	{
		return false;
	}

	ReGameFuncs      = ReGameApi->GetFuncs();
	ReGameHookchains = ReGameApi->GetHookchains();

	return true;
}