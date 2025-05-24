// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef _INCLUDE_LIBRARIES_H
#define _INCLUDE_LIBRARIES_H

#include <cstring>
#include "amxmodx.h"

enum LibSource : std::uint8_t
{
	LibSource_Plugin,
	LibSource_Module
};

enum LibType : std::uint8_t
{
	LibType_Library,
	LibType_Class
};

struct Library
{
	ke::AString name;
	LibSource src;
	LibType type;
	void *parent;
};

enum LibCmd : std::uint8_t
{
	LibCmd_ReqLib,
	LibCmd_ReqClass,
	LibCmd_ForceLib,
	LibCmd_ExpectLib,
	LibCmd_ExpectClass,
	LibCmd_DefaultLib,
};

enum LibError : std::uint8_t
{
	LibErr_None = 0,
	LibErr_NoLibrary,
	LibErr_NoClass,
};

class LibDecoder
{
public:
	LibDecoder() : buffer(nullptr), param1(nullptr), param2(nullptr), cmd()
	{
	}

	~LibDecoder()
	{
		free(buffer);
		buffer = nullptr;
		param1 = nullptr;
		param2 = nullptr;
	}
	char *buffer;
	char *param1;
	char *param2;
	LibCmd cmd;
};

bool AddLibrary(const char *name, LibType type, LibSource src, void *parent= nullptr);
bool DecodeLibCmdString(const char *str, LibDecoder *cmd);
size_t AddLibrariesFromString(const char *name, LibType type, LibSource src, void *parent= nullptr);
size_t ClearLibraries(LibSource src);
LibError RunLibCommand(const LibDecoder *enc);
size_t RemoveLibraries(void *parent);
bool FindLibrary(const char *name, LibType type);


#endif //_INCLUDE_LIBRARIES_H
