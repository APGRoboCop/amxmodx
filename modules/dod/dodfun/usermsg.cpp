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
// DoD Fun Module
//

#include "amxxmodule.h"
#include "dodfun.h"

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
	mPlayer->current = iId;
	break;
  }
}

void Client_InitObj(void* mValue){
  static int num;

  if ( mDest != MSG_ALL )
	  return;

  switch (mState++){
  case 0:
	  num = 0;
	  mObjects.count = *static_cast<int*>(mValue);
	  if ( mObjects.count == 0 )
		  mObjects.Clear();
	  break;
  case 1:
	  mObjects.obj[num].pEdict = GETEDICT(*(int*)mValue);
	  break;
  case 2:
	  mObjects.obj[num].index = *static_cast<int*>(mValue);
	  break;
  case 3:
	  mObjects.obj[num].default_owner = *static_cast<int*>(mValue);
	  mObjects.obj[num].owner = mObjects.obj[num].default_owner;
	  break;
  case 4:
	  mObjects.obj[num].visible = *static_cast<int*>(mValue); 
	  break;
  case 5:
	  mObjects.obj[num].icon_neutral = *static_cast<int*>(mValue);
	  break;
  case 6:
	  mObjects.obj[num].icon_allies = *static_cast<int*>(mValue);
	  break;
  case 7:
	  mObjects.obj[num].icon_axis = *static_cast<int*>(mValue);
	  break;
  case 8:
	  mObjects.obj[num].origin_x = *static_cast<float*>(mValue);
	  break;
  case 9: // 8,9 coord
	  mObjects.obj[num].origin_y = *static_cast<float*>(mValue);
	  mState = 1;
	  num++;
	  if ( num == mObjects.count ){
		  mObjects.Sort();
		  MF_ExecuteForward( iFInitCP );
	  }

	  break;
  }
}

void Client_SetObj(void* mValue){
  static int id;
  switch (mState++){
  case 0:
	  id = *static_cast<int*>(mValue); 
     break;
  case 1:
	  mObjects.obj[id].owner = *static_cast<int*>(mValue);
	break;
  }
}
