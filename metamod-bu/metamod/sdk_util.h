// vi: set ts=4 sw=4 :
// vim: set tw=75 :

// sdk_util.h - wrapper & extension of util.h from HL SDK

/*
 * Copyright (c) 2001-2003 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

// Wrap util.h from SDK with ifndef/endif, to avoid problems from multiple
// inclusions.  Dunno why Valve didn't do that in util.h themselves..

#ifndef SDK_UTIL_H
#define SDK_UTIL_H

// We're not including the DBG_EntOfVars and DBG_AssertFunction routines
// mentioned in the SDK util.h, so we're going to unset DEBUG here so that
// we don't get "unresolved symbol" errors.
#ifdef DEBUG
#undef DEBUG
#endif /* DEBUG */

// Inlcude local enginecallbacks wrapper *first* so that the g_engfuncs
// type is correct and the <enginecallback.h> header protection is already
// defined.
#include "enginecallbacks.h"

#include <util.h>



// Add overloaded ENTINDEX() version for const edict_t pointer.
// The pfnIndexOfEdict() function takes a const edict_t pointer
// as parameter anyway, so there is no reason why ENTINDEX()
// shouldn't.
inline  int ENTINDEX(const edict_t *pEdict) { 
	return (*g_engfuncs.pfnIndexOfEdict)(pEdict);
}


// Also, create some nice inlines for engine callback combos.

// Get a setinfo value from a player entity.
inline char *ENTITY_KEYVALUE(edict_t *entity, const char *key) {
	char *ifbuf=GET_INFOKEYBUFFER(entity);
	return(INFOKEY_VALUE(ifbuf, (char*)key));
}

// Set a setinfo value for a player entity.
inline void ENTITY_SET_KEYVALUE(edict_t *entity, const char *key, const char *value) {
	char *ifbuf=GET_INFOKEYBUFFER(entity);
	SET_CLIENT_KEYVALUE(ENTINDEX(entity), ifbuf, (char*)key, (char*)value);
}

// Get a "serverinfo" value.
inline char *SERVERINFO(const char *key) {
	edict_t *server=INDEXENT(0);
	return(ENTITY_KEYVALUE(server, key));
}

// Set a "serverinfo" value.
inline void SET_SERVERINFO(const char *key, const char *value) {
	edict_t *server=INDEXENT(0);
	char *ifbuf=GET_INFOKEYBUFFER(server);
	SET_SERVER_KEYVALUE(ifbuf, (char*)key, (char*)value);
}

// Get a "localinfo" value.
inline char *LOCALINFO(const char *key) {
	edict_t *server=NULL;
	return(ENTITY_KEYVALUE(server, (char *)key));
}

// Set a "localinfo" value.
inline void SET_LOCALINFO(const char *key, const char *value) {
	edict_t *server=NULL;
	char *ifbuf=GET_INFOKEYBUFFER(server);
	SET_SERVER_KEYVALUE(ifbuf, (char*)key, (char*)value);
}

short FixedSigned16(float value, float scale);
unsigned short FixedUnsigned16(float value, float scale);

// Our slightly modified version, using an edict_t pointer instead of a
// CBaseEntity pointer.
void META_UTIL_HudMessage(edict_t *pEntity, const hudtextparms_t &textparms, const char *pMessage);

char* UTIL_VarArgs( const char *format, ... );

#endif /* SDK_UTIL_H */
