// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*  Date/time module for the Small AMX
 *
 *  Copyright (c) ITB CompuPhase, 2001-2002
 *  This file may be freely used. No warranties of any kind.
 */
#include <ctime>
#include <cassert>
#if defined __WIN32__ || defined _WIN32
  #include <Windows.h>
#include <MMSystem.h>
#endif

#ifdef __GNUC__
#	ifndef CLK_TCK
#		define CLK_TCK	CLOCKS_PER_SEC
#	endif
#endif

#include <cstdlib>

// this file does not include amxmodx.h, so we have to include the memory manager here
#ifdef MEMORY_TEST
#include "mmgr/mmgr.h"
#endif // MEMORY_TEST

#include "amx.h"

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _time(AMX *amx, cell *params)
{
  time_t sec1970;
  cell *cptr;

  assert(params[0]==3*sizeof(cell));

  time(&sec1970);

  /* on DOS/Windows, the timezone is usually not set for the C run-time
   * library; in that case gmtime() and localtime() return the same value
   */
  struct tm gtm = *localtime(&sec1970);
  if (amx_GetAddr(amx,params[1],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_hour;
  if (amx_GetAddr(amx,params[2],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_min;
  if (amx_GetAddr(amx,params[3],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_sec;

  /* the time() function returns the number of seconds since January 1 1970
   * in Universal Coordinated Time (the successor to Greenwich Mean Time)
   */
  return static_cast<cell>(sec1970);
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _date(AMX *amx, cell *params)
{
  time_t sec1970;
  cell *cptr;

  assert(params[0]==3*sizeof(cell));

  time(&sec1970);

  struct tm gtm = *localtime(&sec1970);
  if (amx_GetAddr(amx,params[1],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_year+1900;
  if (amx_GetAddr(amx,params[2],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_mon+1;
  if (amx_GetAddr(amx,params[3],&cptr)==AMX_ERR_NONE)
    *cptr=gtm.tm_mday;

  return 0;
}

#if defined __BORLANDC__ || defined __WATCOMC__
  #pragma argsused
#endif
static cell AMX_NATIVE_CALL _tickcount(AMX *amx, cell *params)
{
 // #if defined __WIN32__ || defined _WIN32
 //   static int timerset = 0;
 // #endif

 assert(params[0]==sizeof(cell));

  //#if defined __WIN32__ || defined _WIN32
  //  if (!timerset) {
  //    timeBeginPeriod(1);       /* timeGetTime() is more accurate on WindowsNT
//                                 * if timeBeginPeriod(1) is set */
  //    timerset=1;
  //  } /* if */
  //  value=timeGetTime();        /* this value is already in milliseconds */
 // #else
    cell value = static_cast<cell>(clock());
    /* convert to milliseconds */
    value=static_cast<cell>((1000L * (value + CLK_TCK / 2)) / CLK_TCK);
  //#endif
  return value;
}


AMX_NATIVE_INFO time_Natives[] = {
  { "time",      _time },
  { "date",      _date },
  { "tickcount", _tickcount },
  {nullptr, nullptr}        /* terminator */
};

