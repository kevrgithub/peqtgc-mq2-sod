// PetCharFnc.cpp
// Author: Shire

#include "../MQ2Plugin.h"
#include "string.h"

PSPAWNINFO GetMaster(DWORD PLAYERID)
{
    PSPAWNINFO pSpawn = NULL;
   char str[80] = {0};

    if (ppSpawnList && pSpawnList) {
        pSpawn = (PSPAWNINFO)pSpawnList;
    }
    while (pSpawn) {
      if ((pSpawn->SpawnID == PLAYERID) && (pSpawn->Type == SPAWN_PLAYER) && (pSpawn->Type != SPAWN_CORPSE)){
         return pSpawn;
        }
        pSpawn = pSpawn->pNext;
    }
    return NULL;
}

PSPAWNINFO GetCharFromPetName(char* Name)
{
    PSPAWNINFO pSpawn = NULL;
   char str[80] = {0};
   char tmp_str[80] = {0};

    if (ppSpawnList && pSpawnList) {
        pSpawn = (PSPAWNINFO)pSpawnList;
    }
    while (pSpawn) {
      strcpy(tmp_str,pSpawn->Name);
      CleanupName(tmp_str,FALSE);
         //sprintf (str,"%s,%s,%d",tmp_str, Name, strstr(tmp_str, Name) );
         //DebugSpewAlways(str);
         //WriteChatColor(str);
      if (strstr(tmp_str, "#") != NULL) strcpy(tmp_str,&tmp_str[1]);
      if ((strcmp(tmp_str, Name) == 0) && (pSpawn->Type != SPAWN_PLAYER) && (pSpawn->Type != SPAWN_CORPSE)){
            //sprintf (str,"%s,%s,%d",tmp_str, Name, strstr(tmp_str, Name) );
         //DebugSpewAlways(str);
         //WriteChatColor(str);
         return GetMaster(pSpawn->MasterID);
      }
        pSpawn = pSpawn->pNext;
    }
    return NULL;
}

PSPAWNINFO GetPetFromCharName(char* Name)
{
    PSPAWNINFO pSpawn = NULL;

    if (ppSpawnList && pSpawnList) {
        pSpawn = (PSPAWNINFO)pSpawnList;
    }
    while (pSpawn) {
      if ((strcmp(pSpawn->Name, Name) == 0) && (pSpawn->Type == SPAWN_PLAYER) && (pSpawn->Type != SPAWN_CORPSE)){
         return (PSPAWNINFO)GetSpawnByID(pSpawn->pActorInfo->PetID);
        }
        pSpawn = pSpawn->pNext;
    }
    return NULL;
}

char* GetPetNameFromCharName(char* Name)
{
    PSPAWNINFO pSpawn = NULL;
   PSPAWNINFO pPet = NULL;

    if (ppSpawnList && pSpawnList) {
        pSpawn = (PSPAWNINFO)pSpawnList;
    }
    while (pSpawn) {
      if ((strcmp(pSpawn->Name, Name) == 0) && (pSpawn->Type == SPAWN_PLAYER) && (pSpawn->Type != SPAWN_CORPSE)){
            pPet = (PSPAWNINFO)GetSpawnByID(pSpawn->pActorInfo->PetID);
         return pPet->Name;
        }
        pSpawn = pSpawn->pNext;
    }
    return "";
}

bool IsNpc(char* Name)
{
   PSPAWNINFO pSpawn = NULL;
   char str[80] = {0};
   char tmp_str[80] = {0};
   char tmp_Name[80] = {0};

    if (ppSpawnList && pSpawnList) {
        pSpawn = (PSPAWNINFO)pSpawnList;
    }
    while (pSpawn) {
      strcpy(tmp_str,pSpawn->Name);
      CleanupName(tmp_str,FALSE);

         //sprintf (str,"%s,%s,%d",tmp_str, Name, strstr(tmp_str, Name) );
         //DebugSpewAlways(str);
         //WriteChatColor(str);
      if (strstr(tmp_str, "#") != NULL) strcpy(tmp_str,&tmp_str[1]);
      strcpy(tmp_Name,&Name[1]);
      //sprintf (str,"%s,%s,%d",tmp_str, tmp_Name, strstr(tmp_str, tmp_Name) );
      //WriteChatColor(str);
      if ( (strstr(tmp_str, tmp_Name) > 0) && (GetCharFromPetName(Name) == NULL) && (pSpawn->Type == SPAWN_NPC) && (pSpawn->Type != SPAWN_CORPSE) ){
            //sprintf (str,"%s,%s,%d %d",tmp_str, tmp_Name, strstr(tmp_str, tmp_Name),pSpawn->Type == SPAWN_NPC );
         //DebugSpewAlways(str);
         //WriteChatColor(str);
         return 1;
      }
        pSpawn = pSpawn->pNext;
    }
    return 0;
}