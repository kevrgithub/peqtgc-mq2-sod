//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2AdvPath.cpp
// Author: A_Enchanter_00
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//
#include "../MQ2Plugin.h"

#include <vector>
#include <list>
#include <direct.h>

PreSetup("MQ2AdvPath");
PLUGIN_VERSION(8.1010);

#define       FOLLOW_OFF                                0
#define       FOLLOW_FOLLOWING                  1
#define       FOLLOW_PLAYING                    2
#define       FOLLOW_RECORDING                  3

#define       STATUS_OFF                                0
#define       STATUS_ON                                 1
#define       STATUS_PAUSED                             2

#define       DISTANCE_BETWEN_LOG               5
#define       DISTANCE_OPEN_DOOR_CLOSE  20
#define       ANGEL_OPEN_DOOR_CLOSE             50.0
#define       DISTANCE_OPEN_DOOR_LONG   50
#define       ANGEL_OPEN_DOOR_LONG              95.0

// Timer Structure
struct Position {
        FLOAT X;
        FLOAT Y;
        FLOAT Z;
        FLOAT Heading;
        char CheckPoint[MAX_STRING];
        bool Warping;
} pPosition;

long FollowState = FOLLOW_OFF;          // Active?
long StatusState = STATUS_OFF;          // Active?

long FollowSpawnDistance = 20;          // Active?

long FollowIdle = 0;                            // FollowIdle time when Follow on?
long NextClickDoor = 0;                         // NextClickDoor when Follow on?
long PauseDoor = 0;                                     // PauseDoor paused Follow on and near door?
bool AutoOpenDoor = true;

bool DoOpenDoor = false;

// INI VARS
long AutoStopFollow = 0;
long AutoStopPath = 0;
bool UseStuckLogic = true;

bool PlayReverse = false;                       // Play Reversed ?
bool PlayLoop = false;                          // Play Loop? a->b a->b a->b a->b ....
//bool PlayReturn = false;                      // Play Return? a->b b->a
long PlayWaypoint = 0;

long MonitorID = 0;                                     // Spawn To Monitor and follow
char MonitorName[MAX_STRING] = {NULL};                          // Spawn To Monitor Name used when following across zones
long MonitorZoneID = 0;                         // Monitor Zone ID
FLOAT MonitorX = 0;                                     // Spawn To MonitorX
FLOAT MonitorY = 0;                                     // Spawn To MonitorY
FLOAT MonitorZ = 0;                                     // Spawn To MonitorZ
FLOAT MonitorHeading = 0;                       // Spawn To MonitorHeading
bool MonitorWarp = false;                       // Spawn To Monitor has warped

FLOAT MeMonitorX = 0;                                   // MeMonitorX monitor your self
FLOAT MeMonitorY = 0;                                   // MeMonitorY monitor your self
FLOAT MeMonitorZ = 0;                                   // MeMonitorZ monitor your self

CHAR Buffer[MAX_STRING] = {0};                                  // Buffer for String manipulatsion

PCHAR TloIndex = {0};                                                   // TloIndex for String manipulatsion

CHAR SavePathName[MAX_STRING] = {NULL};                 // Buffer for Save Path Name
CHAR SavePathZone[MAX_STRING] = {NULL};                 // Buffer for Save Zone Name

CHAR PathNameZone[MAX_STRING] = {NULL};                 // Play path across zone lines
CHAR SpawnNameZone[MAX_STRING] = {NULL};                // Follow Spawn Across zonelines

bool MoveBindsLoaded = false;

list<Position>  FollowPath;                     // FollowPath

HMODULE EQWhMod=0; 
typedef HWND   (__stdcall *fEQW_GetDisplayWindow)(VOID); 
fEQW_GetDisplayWindow EQW_GetDisplayWindow=0; 

class MQ2AdvPathType *pAdvPathType=0;


VOID MQFollowCommand(PSPAWNINFO pChar, PCHAR szLine);
void ReleaseKeys();
void DoWalk(bool walk = false);
void DoFwd(bool hold, bool walk = false);
void DoBck(bool hold);
void DoLft(bool hold);
void DoRgt(bool hold);
void DoStop();
void LookAt(FLOAT X,FLOAT Y,FLOAT Z);
VOID ClearAll();
VOID ClearOne(list<Position>::iterator &CurList);
VOID AddWaypoint(long SpawnID,bool Warping=false);
PDOOR ClosestDoor();
bool IsOpenDoor( PDOOR pDoor );
VOID OpenDoor();
bool InFront( float X,float Y, float Angel, bool Reverse=false );
VOID SavePath( PCHAR PathName ,PCHAR PathZone, bool SaveAll=false );
bool LoadPath( PCHAR PathName );
VOID FollowSpawn();
VOID FollowWaypointsInit(float smart);
VOID FollowWaypoints();
VOID ClearLag();
VOID RecordingWaypoints();
void __stdcall WarpingDetect(unsigned int ID, void *pData, PBLECHVALUE pValues);
void CreateBinds();
void DestroyBinds();
void MQCheckFollowCommand(PCHAR Name, BOOL Down);
VOID ReadINI();

void StuckCheck(bool reset=false);

vector<PMAPLINE>  pFollowPath;

unsigned long thisClock = clock();
unsigned long lastClock = clock();
long DistanceMod = 0;

inline PMAPLINE InitLine() {
  typedef PMAPLINE (__cdecl *InitLineCALL) ();
  PMQPLUGIN pLook=pPlugins;
  while(pLook && stricmp(pLook->szFilename,"MQ2Map")) pLook=pLook->pNext;
  if(pLook)
          if(InitLineCALL Request=(InitLineCALL)GetProcAddress(pLook->hModule,"MQ2MapAddLine"))
                  return Request();
  return 0;
}

inline void DeleteLine(PMAPLINE pLine) {
  typedef VOID (__cdecl *DeleteLineCALL) (PMAPLINE);
  PMQPLUGIN pLook=pPlugins;
  while(pLook && stricmp(pLook->szFilename,"MQ2Map")) pLook=pLook->pNext;
  if(pLook)
          if(DeleteLineCALL Request=(DeleteLineCALL)GetProcAddress(pLook->hModule,"MQ2MapDeleteLine"))
                  Request(pLine);
}

void MapClear() {
        if (pFollowPath.size()) {
                for (unsigned long i=0; i<(unsigned long)pFollowPath.size(); i++) DeleteLine(pFollowPath[i]);
                pFollowPath.clear();
        }
}

bool BardClass() {
  return (strncmp(pEverQuest->GetClassDesc(GetCharInfo2()->Class & 0xFF),"Bard",5))?false:true;
}

/*
//      Ingame commands:
//      /afollow    # Follow's your Target
*/
VOID MQFollowCommand(PSPAWNINFO pChar, PCHAR szLine) {
        DebugSpewAlways("MQ2AdvPath::MQFollowCommand()");
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        bool doFollow = false;
        bool doFollowZone = false;
        bool doAutoOpenDoor = true;
        long MyTarget=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):0;

        if ( szLine[0]==0 ) {
                if(MonitorID || FollowPath.size()) {
                        ClearAll();
                        return;
                } else {
                        doFollow = true;
                }
        } else {
                long iParm=0;
                do {
                        GetArg(Buffer,szLine,++iParm);
                        if(Buffer[0]==0) break;
                        if(!strnicmp(Buffer,"on",2)) {
                                doFollow = true;
                        } else if(!strnicmp(Buffer,"off",3)) {
                                ClearAll();
                                return;
                        } else if(!strnicmp(Buffer,"pause",5)) {
                                WriteChatf("[MQ2AdvPath] Follow Paused");
                                DoStop();
                                StatusState = STATUS_PAUSED;
                                return;
                        } else if(!strnicmp(Buffer,"unpause",7)) {
                                WriteChatf("[MQ2AdvPath] Follow UnPaused");
                                StatusState = STATUS_ON;
                                StuckCheck(true);
                                return;
                        } else if(!strnicmp(Buffer,"spawn",5)) {
                                GetArg(Buffer,szLine,++iParm);
                                MyTarget = atol(Buffer);
                                doFollow = true;
                        } else if(!strnicmp(Buffer,"nodoor",6)) {
                                doAutoOpenDoor = false;
                                doFollow = true;
                        } else if(!strnicmp(Buffer,"door",4)) {
                                doAutoOpenDoor = true;
                                doFollow = true;
                        } else if(!strnicmp(Buffer,"nozone",6)) {
                                doFollowZone = false;
                                doFollow = true;
                        } else if(!strnicmp(Buffer,"zone",4)) {
                                doFollowZone = true;
                                doFollow = true;
                        } else if(!strnicmp(Buffer,"loadini",7)) {
                                ReadINI();
                        } else {
                                if( atol(Buffer) ) {
                                        if( atol(Buffer) < 1 ) FollowSpawnDistance = 1;
                                        else FollowSpawnDistance = atol(Buffer);
                                }
                        }
                }
                while(true);
        }
        if( doFollow ) {
                ClearAll();
                if(MyTarget == GetCharInfo()->pSpawn->SpawnID) MonitorID = 0;
                else MonitorID = MyTarget;

                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(MonitorID)) {
                        AutoOpenDoor = doAutoOpenDoor;
                        AddWaypoint(MonitorID);
                        FollowState = FOLLOW_FOLLOWING;
                        StatusState = STATUS_ON;
                        WriteChatf("[MQ2AdvPath] Following %s", pSpawn->Name);

                        if( doFollowZone ) {
                                strcpy(SpawnNameZone,pSpawn->Name);
                                MonitorZoneID = GetCharInfo()->zoneId;
                        }

                        MeMonitorX = GetCharInfo()->pSpawn->X; // MeMonitorX monitor your self
                        MeMonitorY = GetCharInfo()->pSpawn->Y; // MeMonitorY monitor your self
                        MeMonitorZ = GetCharInfo()->pSpawn->Z; // MeMonitorZ monitor your self
                }
        }
}

VOID MQPlayCommand(PSPAWNINFO pChar, PCHAR szLine) {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        DebugSpewAlways("MQ2AdvPath::MQPlayCommand()");
        bool doPlay = false;
        bool doPlayZone = false;
        float doPlaySmart = 0.00;
        char pathName[MAX_STRING] = {NULL};

        long iParm=0;
        do {
                GetArg(Buffer,szLine,++iParm);
                if(Buffer[0]==0) break;
                if(!strnicmp(Buffer,"off",3)) {
                        ClearAll();
                        return;
                } else if(!strnicmp(Buffer,"pause",5)) {
                        WriteChatf("[MQ2AdvPath] Playing Paused");
                        DoStop();
                        StatusState = STATUS_PAUSED;
                } else if(!strnicmp(Buffer,"unpause",7)) {
                        WriteChatf("[MQ2AdvPath] Playing UnPaused");
                        StatusState = STATUS_ON;
                        StuckCheck(true);
                } else if(!strnicmp(Buffer,"loop",4)) {
                        PlayLoop = true;
                } else if(!strnicmp(Buffer,"noloop",6)) {
                        PlayLoop = false;
                } else if(!strnicmp(Buffer,"reverse",7)) PlayReverse = true;
                else if(!strnicmp(Buffer,"normal",6)) PlayReverse = false;
                else if(!strnicmp(Buffer,"smart",5)) doPlaySmart = 100000.00;
                else if(!strnicmp(Buffer,"nosmart",7)) doPlaySmart = 0.00;
                else if(!strnicmp(Buffer,"nodoor",6)) AutoOpenDoor = false;
                else if(!strnicmp(Buffer,"door",4)) AutoOpenDoor = true;
                else if(!strnicmp(Buffer,"nozone",6)) doPlayZone = false;
                else if(!strnicmp(Buffer,"zone",4)) doPlayZone = true;
                else if(!strnicmp(Buffer,"loadini",7)) ReadINI();
                else {
                        if( !FollowPath.size() && FollowState == FOLLOW_OFF ) {
                                ClearAll();
                                strcpy(pathName,Buffer);
                                LoadPath(Buffer);
                                if(FollowPath.size()) {
                                        WriteChatf("[MQ2AdvPath] Playing Path: %s",Buffer);
                                        FollowState = FOLLOW_PLAYING;
                                        StatusState = STATUS_ON;
                                        doPlay = true;
                                } else {
                                        WriteChatf("[MQ2AdvPath] Playing Path: %s Failed",Buffer);
                                }
                        }
                }
        }
        while(true);
        if( doPlayZone && doPlay ) {
                strcpy(PathNameZone,pathName);
                MonitorZoneID = GetCharInfo()->zoneId;
        }

        if(doPlay) FollowWaypointsInit(doPlaySmart);
}

VOID MQRecordCommand(PSPAWNINFO pChar, PCHAR szLine) {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        DebugSpewAlways("MQ2AdvPath::MQPlayCommand()");

        bool doSave = false;
        bool doSaveAll = false;

        long iParm=0;
        GetArg(Buffer,szLine,++iParm);

        if(!strnicmp(Buffer,"save",4)) {
                if(FollowState == FOLLOW_RECORDING && StatusState ) {
                        if(!strnicmp(Buffer,"saveall",7)) doSaveAll = true;
                        GetArg(Buffer,szLine,++iParm);
                        if(Buffer[0]!= NULL) {
                                strcpy(SavePathName,Buffer);
                                strcpy(SavePathZone,GetShortZone(GetCharInfo()->zoneId));
                        }
                        doSave = true;
                }
        } else if(!strnicmp(Buffer,"checkpoint",10)) {
                GetArg(Buffer,szLine,++iParm);
                if(Buffer[0]==0) return;
                if ( FollowPath.size() && FollowState == FOLLOW_RECORDING ) {
                        int i = 1;
                        list<Position>::iterator CurList=FollowPath.begin();
                        list<Position>::iterator EndList=FollowPath.end();
                        while(CurList!=EndList) {
                                if( FollowPath.size() == i ) {
                                        strcpy(CurList->CheckPoint,Buffer);
                                        return;
                                }
                                i++;
                                CurList++;
                        }
                }
        } else {
                ClearAll();
                //              GetArg(Buffer,szLine,++iParm);
                if(Buffer[0]==0) {
                        SavePathZone[0] = NULL;
                        SavePathName[0] = NULL;
                } else {
                        strcpy(SavePathName,Buffer);
                        strcpy(SavePathZone,GetShortZone(GetCharInfo()->zoneId));
                }

                WriteChatf("[MQ2AdvPath] Recording Path: %s Zone: %s",SavePathName,SavePathZone);
                MonitorID = GetCharInfo()->pSpawn->SpawnID;
                AddWaypoint(MonitorID);
                FollowState = FOLLOW_RECORDING;
                StatusState = STATUS_ON;
                MeMonitorX = GetCharInfo()->pSpawn->X; // MeMonitorX monitor your self
                MeMonitorY = GetCharInfo()->pSpawn->Y; // MeMonitorY monitor your self
                MeMonitorZ = GetCharInfo()->pSpawn->Z; // MeMonitorZ monitor your self
        }
        if( doSave && SavePathName[0]  && SavePathZone[0] != NULL ) {
                SavePath(SavePathName,SavePathZone,doSaveAll);
        }
}

//Movement Related Functions
void ReleaseKeys() {
        DoWalk(false);
        DoFwd(false);
        DoBck(false);
        DoRgt(false);
        DoLft(false);
}

void MQCheckFollowCommand(PCHAR Name, BOOL Down) {
        if(StatusState == STATUS_ON) {
                if (FollowState == FOLLOW_FOLLOWING && AutoStopFollow) {
                        if (AutoStopFollow == 1) {
                                WriteChatf("[MQ2AdvPath] Follow Paused");
                                StatusState = STATUS_PAUSED;
                                ReleaseKeys();
                        } else ClearAll();
                } else if (FollowState == FOLLOW_PLAYING && AutoStopPath) {
                        if (AutoStopPath == 1) {
                                WriteChatf("[MQ2AdvPath] Playing Paused");
                                StatusState = STATUS_PAUSED;
                                ReleaseKeys();
                        }else ClearAll();
                }
        }
    return;
} 

void DoWalk(bool walk) {
        bool state_walking = (*EQADDR_RUNWALKSTATE) ? false : true;
        float SpeedMultiplier = *((float*) &(((PSPAWNINFO) pLocalPlayer)->SpeedMultiplier));
        if (SpeedMultiplier < 0) walk = false; // we're snared, dont go into walk mode no matter what
        if ( (walk && !state_walking) || (!walk && state_walking) ) {
                MQ2Globals::ExecuteCmd(FindMappableCommand("run_walk"),1,0);
                MQ2Globals::ExecuteCmd(FindMappableCommand("run_walk"),0,0);
        }
}

void DoFwd(bool hold, bool walk) {
        static bool held = false;
        if ( hold ) {
                if( GetCharInfo()->pSpawn->CastingData.SpellID != -1 && !BardClass() ) return;
                DoWalk(walk);
                DoBck(false);
                if (!held) MQ2Globals::ExecuteCmd(FindMappableCommand("forward"),1,0);
                held = true;
        } else {
                DoWalk(false);
                if (held) MQ2Globals::ExecuteCmd(FindMappableCommand("forward"),0,0);
                held = false;
        }
}

void DoBck(bool hold) {
        static bool held = false;
        if( hold ) {
                DoFwd(false);
                if (!held) MQ2Globals::ExecuteCmd(FindMappableCommand("back"),1,0);
                held = true;
        } else {
                if (held) MQ2Globals::ExecuteCmd(FindMappableCommand("back"),0,0);
                held = false;
        }
}

void DoLft(bool hold) {
        static bool held = false;
        if( hold ) {
                DoRgt(false);
                if (!held) MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_left"),1,0);
                held = true;
        } else {
                if (held) MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_left"),0,0);
                held = false;
        }
}

void DoRgt(bool hold) {
        static bool held = false;
        if( hold ) {
                DoLft(false);
                if (!held) MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_right"),1,0);
                held = true;
        } else {
                if (held) MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_right"),0,0);
                held = false;
        }
}

void DoStop() {
        if ( !FollowIdle ) FollowIdle = (long)clock();
        DoBck(true);
        ReleaseKeys();
}

void LookAt(FLOAT X,FLOAT Y,FLOAT Z) {
        gFaceAngle = ( atan2(X - GetCharInfo()->pSpawn->X, Y - GetCharInfo()->pSpawn->Y)  * 256.0f / PI);
        if (gFaceAngle>=512.0f) gFaceAngle -= 512.0f;
        if (gFaceAngle<0.0f) gFaceAngle += 512.0f;
        ((PSPAWNINFO)pCharSpawn)->Heading = (FLOAT)gFaceAngle;
        gFaceAngle=10000.0f;

        if (GetCharInfo()->pSpawn->UnderWater==5 || GetCharInfo()->pSpawn->FeetWet==5) GetCharInfo()->pSpawn->CameraAngle = (FLOAT)( atan2(Z - GetCharInfo()->pSpawn->Z, (FLOAT)GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y, X,Y)) * 256.0f / PI);
        else if (GetCharInfo()->pSpawn->Levitate==2) {
                if ( Z < GetCharInfo()->pSpawn->Z-5) GetCharInfo()->pSpawn->CameraAngle = -45.0f;
                else if ( Z > GetCharInfo()->pSpawn->Z+5) GetCharInfo()->pSpawn->CameraAngle = 45.0f;
                else GetCharInfo()->pSpawn->CameraAngle = 0.0f;
        } else GetCharInfo()->pSpawn->CameraAngle = 0.0f;
        gLookAngle=10000.0f;
}

VOID ClearAll() {
        if( MonitorID || FollowPath.size() ) WriteChatf("[MQ2AdvPath] Stopped");
        FollowPath.clear();
        NextClickDoor = PauseDoor = FollowIdle = MonitorID = 0;
        DoStop();

        FollowState = FOLLOW_OFF;               // Active?
        StatusState = STATUS_OFF;               // Active?
        MonitorWarp = PlayReverse = PlayLoop = false;
        MonitorZoneID = 0;

        AutoOpenDoor = true;

        SavePathZone[0] = NULL;
        SavePathName[0] = NULL;
        PathNameZone[0] = NULL;
        SpawnNameZone[0] = NULL;

        MapClear();

        StuckCheck(true);
}

VOID ClearOne(list<Position>::iterator &CurList) {
        list<Position>::iterator PosList;
        PosList=CurList;
        CurList++;
        FollowPath.erase(PosList);
}

void CreateBinds() {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn || MoveBindsLoaded) return;
        MoveBindsLoaded = true;
        AddMQ2KeyBind("STOP_BCK",MQCheckFollowCommand); 
        AddMQ2KeyBind("STOP_LFT",MQCheckFollowCommand); 
        AddMQ2KeyBind("STOP_RGT",MQCheckFollowCommand); 
        AddMQ2KeyBind("STOP_STRAFE_LFT",MQCheckFollowCommand); 
        AddMQ2KeyBind("STOP_STRAFE_RGT",MQCheckFollowCommand); 
        SetMQ2KeyBind("STOP_BCK",false,pKeypressHandler->NormalKey[FindMappableCommand("back")]); 
        SetMQ2KeyBind("STOP_BCK",true,pKeypressHandler->AltKey[FindMappableCommand("back")]); 
        SetMQ2KeyBind("STOP_LFT",false,pKeypressHandler->NormalKey[FindMappableCommand("left")]); 
        SetMQ2KeyBind("STOP_LFT",true,pKeypressHandler->AltKey[FindMappableCommand("left")]); 
        SetMQ2KeyBind("STOP_RGT",false,pKeypressHandler->NormalKey[FindMappableCommand("right")]); 
        SetMQ2KeyBind("STOP_RGT",true,pKeypressHandler->AltKey[FindMappableCommand("right")]); 
        SetMQ2KeyBind("STOP_STRAFE_LFT",false,pKeypressHandler->NormalKey[FindMappableCommand("strafe_left")]); 
        SetMQ2KeyBind("STOP_STRAFE_LFT",true,pKeypressHandler->AltKey[FindMappableCommand("strafe_left")]); 
        SetMQ2KeyBind("STOP_STRAFE_RGT",false,pKeypressHandler->NormalKey[FindMappableCommand("strafe_right")]); 
        SetMQ2KeyBind("STOP_STRAFE_RGT",true,pKeypressHandler->AltKey[FindMappableCommand("strafe_right")]); 
} 

void DestroyBinds() {
        if (!MoveBindsLoaded) return;
        RemoveMQ2KeyBind("STOP_BCK"); 
        RemoveMQ2KeyBind("STOP_LFT"); 
        RemoveMQ2KeyBind("STOP_RGT"); 
        RemoveMQ2KeyBind("STOP_STRAFE_LFT"); 
        RemoveMQ2KeyBind("STOP_STRAFE_RGT"); 
}

VOID AddWaypoint(long SpawnID,bool Warping) {
        if( PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(SpawnID) ) {
                Position MonitorPosition;

                MonitorPosition.Warping = Warping;

                MonitorPosition.X = MonitorX = pSpawn->X;
                MonitorPosition.Y = MonitorY = pSpawn->Y;
                MonitorPosition.Z = MonitorZ = pSpawn->Z;
                MonitorPosition.Heading = MonitorHeading = pSpawn->Heading;
                strcpy(MonitorPosition.CheckPoint,"");

                FollowPath.push_back(MonitorPosition);
        }
}

PDOOR ClosestDoor() {
        PDOORTABLE pDoorTable = (PDOORTABLE)pSwitchMgr;
        FLOAT Distance = 100000.00;
        PDOOR pDoor = NULL;
        for (DWORD Count=0; Count<pDoorTable->NumEntries; Count++) {
                if( Distance > GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,pDoorTable->pDoor[Count]->DefaultX,pDoorTable->pDoor[Count]->DefaultY,pDoorTable->pDoor[Count]->DefaultZ) ) {
                        Distance = GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,pDoorTable->pDoor[Count]->DefaultX,pDoorTable->pDoor[Count]->DefaultY,pDoorTable->pDoor[Count]->DefaultZ);
                        pDoor = pDoorTable->pDoor[Count];
                }
        }
        return pDoor;
}

bool IsOpenDoor( PDOOR pDoor ) {
        if(pDoor->DefaultHeading!=pDoor->Heading || pDoor->Y!=pDoor->DefaultY || pDoor->X!=pDoor->DefaultX  || pDoor->Z!=pDoor->DefaultZ )return true;
        return false;
}

VOID OpenDoor() {
        if(!AutoOpenDoor) return;

        if(PDOOR pDoor = (PDOOR)ClosestDoor()) {
                if(!IsOpenDoor(pDoor) && NextClickDoor < (long)clock()) {
                        if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,pDoor->DefaultX,pDoor->DefaultY,pDoor->DefaultZ) <  DISTANCE_OPEN_DOOR_CLOSE ) {
                                if( InFront(pDoor->X,pDoor->Y, ANGEL_OPEN_DOOR_CLOSE,false) ) {
                                        DoOpenDoor = true;
                                        NextClickDoor = (long)clock() + 100;
                                        if ((PauseDoor - (long)clock()) < 0) {
                                                PauseDoor = (long)clock() + 1000;
                                                DoStop();
                                        }
                                }
                        } else if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,pDoor->DefaultX,pDoor->DefaultY,pDoor->DefaultZ) <  DISTANCE_OPEN_DOOR_LONG ) {
                                if( InFront(pDoor->X,pDoor->Y, ANGEL_OPEN_DOOR_LONG,false) ) {
                                        DoOpenDoor = true;
                                        NextClickDoor = (long)clock() + 100;
                                }
                        }
                }
        }
}

bool InFront(float X,float Y, float Angel, bool Reverse) {
        FLOAT Angle = (FLOAT)((atan2f(X - GetCharInfo()->pSpawn->X, Y - GetCharInfo()->pSpawn->Y) * 180.0f / PI));
        if(Angle<0)     Angle +=360;
        Angle = Angle*1.42f;

        if(Reverse) {
                if( Angle + 256 > 512 ) {
                        Angle = Angle-256;
                } else if( Angle - 256 < 0 ) {
                        Angle = Angle+256;
                }
        }

        bool Low = false;
        bool High = false;

        FLOAT Angle1 = GetCharInfo()->pSpawn->Heading - Angel;
        if(Angle1<0) {
                Low = true;
                Angle1 +=512.0f;
        }

        FLOAT Angle2 = GetCharInfo()->pSpawn->Heading + Angel;
        if(Angle2>512.0f) {
                High = true;
                Angle2 -=512.0f;
        }

        if( Low ) {
                if( Angle1 < (Angle + 512.0f) && Angle2 > Angle ) return true;
        } else if( High ) {
                if( Angle1 < Angle && Angle2 > (Angle - 512.0f) ) return true;
        } else if( Angle1 < Angle && Angle2 > Angle ) {
                return true;
        }

//      if( Angle1 < Angle && Angle2 > Angle ) return true;
        return false;
}

VOID SavePath( PCHAR PathName, PCHAR PathZone, bool SaveAll ) {
        WriteChatf("[MQ2AdvPath] Saveing Path: %s Zone: %s",PathName,PathZone);
        CHAR INIFileNameTemp[400];
        char szTemp[MAX_STRING];
        char szTemp2[MAX_STRING];

        if( !SaveAll ) {
                unsigned long thisWaypoint = 0;
                float lastHeading = 0;

                unsigned long DeleteWaypoint = 0;

                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator EndList=FollowPath.end();

                while(CurList!=EndList) {
                        thisWaypoint++;
                         if( FollowPath.size() == thisWaypoint && CurList->Warping ) {
                                ClearOne(CurList);
                                break;
                        } else if( lastHeading == CurList->Heading && thisWaypoint > 2 && !CurList->Warping && !CurList->CheckPoint[0] ) {
                                DeleteWaypoint = thisWaypoint - 1;
                                CurList=FollowPath.begin();
                                thisWaypoint = 0;
                        } else if( DeleteWaypoint == thisWaypoint ) {
                                ClearOne(CurList);
                                CurList=FollowPath.begin();
                                thisWaypoint = 0;
                                DeleteWaypoint = 0;
                        } else {
                                lastHeading = CurList->Heading;
                                CurList++;
                        }
                }
        }

        sprintf(INIFileNameTemp,"%s\\%s\\%s.ini",gszINIPath,"MQ2AdvPath",PathZone);
        if( FollowPath.size() ) {
                WritePrivateProfileString(PathName,NULL,      NULL,INIFileNameTemp);
                int i = 1;
                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator EndList=FollowPath.end();
                while(CurList!=EndList) {
                        sprintf(szTemp2,"%d",i);
                        sprintf(szTemp,"%.2f %.2f %.2f %s",CurList->Y,CurList->X,CurList->Z,CurList->CheckPoint);
                        WritePrivateProfileString(PathName,szTemp2,      szTemp,INIFileNameTemp);
                        i++;
                        CurList++;
                }
        }
        ClearAll();
}

bool LoadPath( PCHAR PathName ) {
        FollowPath.clear();
        CHAR INIFileNameTemp[400];
        sprintf(INIFileNameTemp,"%s\\%s\\%s.ini",gszINIPath,"MQ2AdvPath",GetShortZone(GetCharInfo()->zoneId));

        int i = 1;

        char szTemp[MAX_STRING] = {0};
        char szTemp3[MAX_STRING] = {0};
        do {
                char szTemp2[MAX_STRING] = {0};
                char szTemp3[MAX_STRING] = {0};
                sprintf(szTemp,"%d",i);
                GetPrivateProfileString(PathName,szTemp,NULL,szTemp2,MAX_STRING,INIFileNameTemp);
                if( szTemp2[0]==0 ) break;
                Position TempPosition;

                GetArg(szTemp3,szTemp2,1);
                        TempPosition.Y = (FLOAT)atof(szTemp3);
                GetArg(szTemp3,szTemp2,2);
                        TempPosition.X = (FLOAT)atof(szTemp3);
                GetArg(szTemp3,szTemp2,3);
                        TempPosition.Z = (FLOAT)atof(szTemp3);
                GetArg(szTemp3,szTemp2,4);
                        strcpy(TempPosition.CheckPoint,szTemp3);

                TempPosition.Heading = 0;
                TempPosition.Warping = 0;

                FollowPath.push_back(TempPosition);

                i++;
        }
        while(true);
        return FollowPath.size()?true:false;
}

VOID FollowSpawn() {
        if( FollowPath.size() ) {
                if( GetDistance3D(MeMonitorX,MeMonitorY,MeMonitorZ,GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z) > 50 ) {
                        list<Position>::iterator CurList=FollowPath.begin();
                        list<Position>::iterator EndList=FollowPath.end();
                        do{
                                if(CurList == EndList ) break;
                                if(CurList->Warping) break;
                                ClearOne(CurList);
                        }
                        while(true);
                        WriteChatf("[MQ2AdvPath] Warping Detected on SELF");
                }
                if( !FollowPath.size() ) return;

                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator EndList=FollowPath.end();

                HWND EQhWnd=*(HWND*)EQADDR_HWND;
                if (EQW_GetDisplayWindow) EQhWnd=EQW_GetDisplayWindow();

                bool run = false;

                if( CurList->Warping && GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,CurList->X,CurList->Y,CurList->Z) > 50 ) {
                        if ( !MonitorWarp ) {
                                WriteChatf("[MQ2AdvPath] Warping Wating");
                                DoStop();
                        }
                        MonitorWarp = true;
                        return;
                }
                MonitorWarp = false;

                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(MonitorID)) {
                        if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,pSpawn->X,pSpawn->Y,pSpawn->Z) >= 50 ) run = true;
                        if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,pSpawn->X,pSpawn->Y,pSpawn->Z) <= FollowSpawnDistance ) {
                                DoStop();
                                return;
                        }
                } else if( (FollowPath.size()*DISTANCE_BETWEN_LOG) >= 20 ) run = true;
//               else if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,CurList->X,CurList->Y,CurList->Z) >= 20 ) run = true;

//              if( ( GetForegroundWindow()==EQhWnd && GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > DISTANCE_BETWEN_LOG ) || ( GetForegroundWindow()!=EQhWnd && GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > (DISTANCE_BETWEN_LOG+10) ) ) {
                if( GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > DISTANCE_BETWEN_LOG+DistanceMod ) {
                        LookAt(CurList->X,CurList->Y,CurList->Z);
                        if( (PauseDoor - (long)clock()) < 300 ) {
                                if ( run ) DoFwd(true);
                                else DoFwd(true,true);
                        }

                        OpenDoor();
                        FollowIdle = 0;
                } else {
                                ClearOne(CurList);
                                if( CurList != EndList ) {
                                        if(CurList->Warping) return;
                                        // Clean up lag
                                        ClearLag();
                                } else {
                                        DoStop();
                                        if( !MonitorID ) {
                                                DoOpenDoor = true;
                                                ClearAll();
                                        }
                                }
                }
        }
}

VOID FollowWaypointsInit(float smart) {
        if(smart) {
                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator EndList=FollowPath.end();
                int i = 1;
                while(CurList!=EndList) {                       
                        if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,CurList->X,CurList->Y,CurList->Z) < smart ) {
                                smart = GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,CurList->X,CurList->Y,CurList->Z);
                                PlayWaypoint = i;
                        }
                        i++;
                        CurList++;
                }
        } else {
                if(PlayReverse) PlayWaypoint = FollowPath.size();
                else PlayWaypoint = 1;
        }
}

VOID FollowWaypoints() {
        if( FollowPath.size() ) {
                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator EndList=FollowPath.end();

                HWND EQhWnd=*(HWND*)EQADDR_HWND;
                if (EQW_GetDisplayWindow) EQhWnd=EQW_GetDisplayWindow(); 

                long WaypointIndex=1;
                do{
                        if(CurList == EndList) break;
                        if(WaypointIndex == PlayWaypoint) {
//                              if( ( GetForegroundWindow()==EQhWnd && GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > DISTANCE_BETWEN_LOG ) || ( GetForegroundWindow()!=EQhWnd && GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > (DISTANCE_BETWEN_LOG+10) ) ) 
                                if( GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > DISTANCE_BETWEN_LOG+DistanceMod ) {
                                        LookAt(CurList->X,CurList->Y,CurList->Z);
                                        if( (PauseDoor - (long)clock()) < 300 ) DoFwd(true);
                                        OpenDoor();
                                        FollowIdle = 0;
                                        break;
                                } else {
                                        if( ( !PlayReverse && FollowPath.size() == PlayWaypoint ) || ( PlayReverse && WaypointIndex == 1 ) ) {
                                                if(PlayLoop) {
                                                        if(PlayReverse) PlayWaypoint = FollowPath.size();
                                                        else PlayWaypoint = 1;
                                                } else {
                                                        ClearAll();
                                                }
                                        }
                                        if (GetForegroundWindow()==EQhWnd) {
                                                if(PlayReverse) PlayWaypoint--;
                                                else PlayWaypoint++;
                                        } else {
                                                if(PlayReverse) PlayWaypoint = PlayWaypoint -3;
                                                else PlayWaypoint = PlayWaypoint +3;

                                                if((long)FollowPath.size() < PlayWaypoint ) PlayWaypoint = FollowPath.size();
                                                else if(PlayWaypoint < 1 ) PlayWaypoint = 1;
                                        }
                                        break;
                                }
                        }
                        WaypointIndex++;
                        CurList++;
                }
                while(true);
        }
}

VOID ClearLag() {
        if( FollowPath.size() ) {
                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator LastList;
                list<Position>::iterator EndList=FollowPath.end();

                if( CurList != EndList ) {
                        if(CurList->Warping) return;
                        if( InFront(CurList->X,CurList->Y, 100, true) ) {
                                CurList++;
                                for(int LagCount=0; LagCount<15; LagCount++) {
                                        if( CurList != EndList ) {
                                                if( ( InFront(CurList->X,CurList->Y, 100, false) && LagCount < 10  ) || ( InFront(CurList->X,CurList->Y, 50, false) && LagCount >= 10 ) ) {
                                                        DebugSpewAlways("MQ2AdvPath::Removing lag() %d",LagCount);
                                                        CurList=FollowPath.begin();
                                                        for(int LagCount2=0; LagCount2<=LagCount; LagCount2++) ClearOne(CurList);
                                                        return;
                                                }
                                        } else {
                                                return;
                                        }
                                        CurList++;
                                }
                        }
                }
        }
}

VOID StuckCheck(bool reset) {
        static time_t JumpTimer = time(NULL);
        static time_t StuckMonitor = time(NULL);

        static bool Jump = true;

        static float StuckMonitorX = 0;
        static float StuckMonitorY = 0;
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        if( FollowPath.size() && UseStuckLogic && !reset && StatusState == STATUS_ON && ( FollowState == FOLLOW_PLAYING || FollowState == FOLLOW_FOLLOWING ) ) {
                list<Position>::iterator CurList=FollowPath.begin();
                if (FollowState == FOLLOW_PLAYING) {
                        list<Position>::iterator EndList=FollowPath.end();
                        long WaypointIndex=1;
                        do{
                                if(CurList == EndList) return;
                                if(WaypointIndex != PlayWaypoint) {
                                        WaypointIndex++;
                                        CurList++;
                                } else break;
                        }
                        while(true);
                }

                int TimerMod = 2;
                if( CurList->Z > GetCharInfo()->pSpawn->Z+5 ) TimerMod = 5;

                if ( FindSpeed(GetCharInfo()->pSpawn) && StatusState == STATUS_ON && !GetCharInfo()->Stunned && GetCharInfo()->pSpawn->SpeedMultiplier != -10000 ) {
                                float pulseMoved=GetDistance(MeMonitorX,MeMonitorY);
                                float expectedPulseMoved = FindSpeed(GetCharInfo()->pSpawn)/6/(thisClock-lastClock);
                                time_t now = time(NULL);
                                if ( Jump && !GetCharInfo()->pSpawn->Levitate && !GetCharInfo()->pSpawn->UnderWater && ( !pulseMoved || ( pulseMoved < 0.1 && pulseMoved < expectedPulseMoved ) ) && JumpTimer+1 < now ) {
                                        Jump = false;
                                        JumpTimer = time(NULL);
                                        MQ2Globals::ExecuteCmd(FindMappableCommand("JUMP"),1,0);
                                        MQ2Globals::ExecuteCmd(FindMappableCommand("JUMP"),0,0);
                                        DoOpenDoor = true;
                                }

                                if ( GetDistance(StuckMonitorX,StuckMonitorY) > 2 ) {
                                        reset = true;
                                } else if ( ( !pulseMoved || ( pulseMoved < 0.1 && pulseMoved < expectedPulseMoved ) ) && StuckMonitor+TimerMod < now ) {
                                        StuckMonitor = time(NULL);
                                        WriteChatf("[MQ2AdvPath] Stuck Detected!!");
                                        ClearAll();
                                }
                } else reset = true;
        } else reset = true;
        if( reset ) {
                Jump = true;
                StuckMonitorX = GetCharInfo()->pSpawn->X;
                StuckMonitorY = GetCharInfo()->pSpawn->Y;
                StuckMonitor = time(NULL);
                JumpTimer = time(NULL);
        }
}

VOID RecordingWaypoints() {
        if(MonitorID) {
                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(MonitorID)) {
                        if( GetDistance3D(MonitorX,MonitorY,MonitorZ,pSpawn->X,pSpawn->Y,pSpawn->Z) > 100 ) {
                                WriteChatf("[MQ2AdvPath] Warping Detected on %s",pSpawn->Name);
                                AddWaypoint(MonitorID,true);
                        } else if( GetDistance3D(MonitorX,MonitorY,MonitorZ,pSpawn->X,pSpawn->Y,pSpawn->Z) >= DISTANCE_BETWEN_LOG ) AddWaypoint(MonitorID);
                }
        }
}

class MQ2AdvPathType : public MQ2Type {
        private:
                char Temps[MAX_STRING];
        public:
                enum AdvPathMembers {
                        Active=1,
                        State=2,
                        Waypoints=3,
                        NextWaypoint=4,
                        Y=5,
                        X=6,
                        Z=7,
                        Monitor=8,
                        Idle=9,
                        Length=10,
                        Following=11,
                        Playing=12,
                        Recording=13,
                        Status=14,
                        Paused=15,
                        WaitingWarp=16,
                        StopDistance=17,
                        CheckPoint=18
                };
                MQ2AdvPathType():MQ2Type("AdvPath") {
                        TypeMember(Active);
                        TypeMember(State);
                        TypeMember(Waypoints);
                        TypeMember(NextWaypoint);
                        TypeMember(Y);
                        TypeMember(X);
                        TypeMember(Z);
                        TypeMember(Monitor);
                        TypeMember(Idle);
                        TypeMember(Length);
                        TypeMember(Following);
                        TypeMember(Playing);
                        TypeMember(Recording);
                        TypeMember(Status);
                        TypeMember(Paused);
                        TypeMember(WaitingWarp);
                        TypeMember(StopDistance);
                        TypeMember(CheckPoint);
                }
                bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
                        list<Position>::iterator CurList=FollowPath.begin();
                        list<Position>::iterator EndList=FollowPath.end();
                        int i = 1;
                        float TheLength = 0;

                        PMQ2TYPEMEMBER pMember=MQ2AdvPathType::FindMember(Member); 
                        if(pMember) switch((AdvPathMembers)pMember->ID) {
                                case Active:                                                                            // Plugin on and Ready
                                        Dest.DWord=(gbInZone && GetCharInfo() && GetCharInfo()->pSpawn);
                                        Dest.Type=pBoolType;
                                        return true;
                                case State:                                                                                     // FollowState, 0 = off, 1 = Following, 2 = Playing, 3 = Recording
                                        Dest.DWord=FollowState;
                                        Dest.Type=pIntType;
                                        return true;
                                case Waypoints:                                                                         // Number of Waypoints
                                        Dest.DWord=FollowPath.size();
                                        Dest.Type=pIntType;
                                        return true;
                                case NextWaypoint:                                                                      // Next Waypoint
                                        Dest.DWord=PlayWaypoint;
                                        Dest.Type=pIntType;
                                        return true;
                                case Y:                                                                                         // Waypoint Y
                                        while(CurList!=EndList) {                                               
                                                if( ( i == atol(TloIndex) || ( TloIndex[0] != 0 && !stricmp(TloIndex,CurList->CheckPoint) ) ) || (  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) ) {
                                                        Dest.Float=CurList->Y;
                                                        Dest.Type=pFloatType;
                                                        return true;
                                                }
                                                i++;
                                                CurList++;
                                        }
                                        strcpy(Temps,"NULL");
                                        Dest.Type=pStringType;
                                        Dest.Ptr=Temps;
                                        return true;
                                case X:                                                                                         // Waypoint X
                                        while(CurList!=EndList) {
                                                if( ( i == atol(TloIndex) || ( TloIndex[0] != 0 && !stricmp(TloIndex,CurList->CheckPoint) ) ) || (  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) ) {
                                                        Dest.Float=CurList->X;
                                                        Dest.Type=pFloatType;
                                                        return true;
                                                }
                                                i++;
                                                CurList++;
                                        }
                                        strcpy(Temps,"NULL");
                                        Dest.Type=pStringType;
                                        Dest.Ptr=Temps;
                                        return true;
                                case Z:                                                                                         // Waypoint Z
                                        while(CurList!=EndList) {
                                                if( ( i == atol(TloIndex) || ( TloIndex[0] != 0 && !stricmp(TloIndex,CurList->CheckPoint) ) ) || (  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) ) {
                                                        Dest.Float=CurList->Z;
                                                        Dest.Type=pFloatType;
                                                        return true;
                                                }
                                                i++;
                                                CurList++;
                                        }
                                        strcpy(Temps,"NULL");
                                        Dest.Type=pStringType;
                                        Dest.Ptr=Temps;
                                        return true;
                                case Monitor:                                                                           // Spawn your following
                                        Dest.Ptr=(PSPAWNINFO)GetSpawnByID(MonitorID);
                                        Dest.Type=pSpawnType;
                                        return true;
                                case Idle:                                                                                      // FollowIdle time when following and not moving
                                        Dest.DWord=(FollowState && FollowIdle)?(((long)clock()-FollowIdle)/1000):0;
                                        Dest.Type=pIntType;
                                        return true;
                                case Length:                                                                            // Estimated length off the follow path
                                        if( FollowPath.size() ) {
                                                list<Position>::iterator CurList=FollowPath.begin();
                                                TheLength = GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y);
                                                if( FollowPath.size() > 1 )     TheLength = ((FollowPath.size()-1)*DISTANCE_BETWEN_LOG)+TheLength;
                                        }
                                        Dest.Float=TheLength;
                                        Dest.Type=pFloatType;
                                        return true;
                                case Following:
                                        Dest.DWord=(FollowState==FOLLOW_FOLLOWING);
                                        Dest.Type=pBoolType;
                                        return true;
                                case Playing:
                                        Dest.DWord=(FollowState==FOLLOW_PLAYING);
                                        Dest.Type=pBoolType;
                                        return true;
                                case Recording:
                                        Dest.DWord=(FollowState==FOLLOW_RECORDING);
                                        Dest.Type=pBoolType;
                                        return true;
                                case Status:
                                        Dest.DWord=StatusState;
                                        Dest.Type=pIntType;
                                        return true;
                                case Paused:
                                        Dest.DWord=(StatusState==STATUS_PAUSED);
                                        Dest.Type=pBoolType;
                                        return true;
                                case WaitingWarp:
                                        Dest.DWord=MonitorWarp;
                                        Dest.Type=pBoolType;
                                        return true;
                                case StopDistance:
                                        Dest.DWord=(DISTANCE_BETWEN_LOG+DistanceMod);
                                        Dest.Type=pIntType;
                                        return true;
                                case CheckPoint:
                                        strcpy(Temps,"NULL");
                                        while(CurList!=EndList) {                                               
                                                if( ( i == atol(TloIndex) || ( TloIndex[0] != 0 && !stricmp(TloIndex,CurList->CheckPoint) ) ) || (  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) ) {
                                                        strcpy(Temps,CurList->CheckPoint);
                                                        break;
                                                }
                                                i++;
                                                CurList++;
                                        }

                                        Dest.Type=pStringType;
                                        Dest.Ptr=Temps;
                                        return true;
                        }
                        strcpy(Temps,"NULL");
                        Dest.Type=pStringType;
                        Dest.Ptr=Temps;
                        return true;
                }
                bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
                        strcpy(Destination,"TRUE");
                        return true;
                } 
                bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
                        return false;
                } 
                bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
                        return false;
                } 
                ~MQ2AdvPathType() { } 
}; 

BOOL dataAdvPath(PCHAR szName, MQ2TYPEVAR &Dest) {
        TloIndex = szName;

        Dest.DWord=1;
        Dest.Type=pAdvPathType;
        return true;
}

VOID ReadINI() {
        DebugSpewAlways("MQ2CastTimer::ReadINI()");
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        char szSettingINISection[MAX_STRING];
        sprintf(szSettingINISection,"Settings.%s.%s",EQADDR_SERVERNAME,GetCharInfo()->pSpawn->Name);

        AutoStopFollow = GetPrivateProfileInt(szSettingINISection,"AutoStopFollow",      0,INIFileName);
        AutoStopPath = GetPrivateProfileInt(szSettingINISection,"AutoStopPath",      0,INIFileName);
        UseStuckLogic = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"UseStuckLogic",      1,INIFileName);

        WritePrivateProfileString(szSettingINISection,"AutoStopFollow",      itoa(AutoStopFollow,         Buffer,10),INIFileName);
        WritePrivateProfileString(szSettingINISection,"AutoStopPath",      itoa(AutoStopPath,            Buffer,10),INIFileName);
        WritePrivateProfileString(szSettingINISection,"UseStuckLogic",      itoa(UseStuckLogic,         Buffer,10),INIFileName);
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID) {
        DebugSpewAlways("Initializing MQ2AdvPath");
        if (EQWhMod=GetModuleHandle("eqw.dll")) EQW_GetDisplayWindow=(fEQW_GetDisplayWindow)GetProcAddress(EQWhMod,"EQW_GetDisplayWindow");

    AddCommand("/afollow",MQFollowCommand);
        AddCommand("/play",MQPlayCommand);
        AddCommand("/record",MQRecordCommand);
        AddCommand("/arecord",MQRecordCommand);
        pAdvPathType = new MQ2AdvPathType;
        AddMQ2Data("AdvPath",dataAdvPath);
        sprintf(Buffer,"%s\\%s",gszINIPath,"MQ2AdvPath");
        mkdir(Buffer);
        CreateBinds();
        ReadINI();
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID) {
        DebugSpewAlways("Shutting down MQ2AdvPath");

        RemoveCommand("/afollow");
        RemoveCommand("/play");
        RemoveCommand("/record");
        RemoveCommand("/arecord");
        delete pAdvPathType;
        RemoveMQ2Data("AdvPath");
        DestroyBinds();

        ClearAll();
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID) {
        static int CameraMode = 0;
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;

        thisClock = (unsigned long)clock();
        DistanceMod = ((thisClock-lastClock)*(thisClock-lastClock))/1000/2;

        if ( PathNameZone[0] && MonitorZoneID != GetCharInfo()->zoneId ) {
                WriteChatf("[MQ2AdvPath] Zoned Loading next path (%s)",PathNameZone);
                MonitorZoneID = GetCharInfo()->zoneId;
                if( LoadPath(PathNameZone) ) {
                        FollowWaypointsInit(100000.00);
                } else {
                        ClearAll();
                }
        } else if( SpawnNameZone[0] && MonitorZoneID != GetCharInfo()->zoneId ) {
                PSPAWNINFO sSpawn = NULL;                       // Search Spawn
                PSPAWNINFO pSpawn = NULL;                       // Output Spawn

                SEARCHSPAWN ssSpawn;                            // Search String ( Spawn )
                ClearSearchSpawn(&ssSpawn);
                ssSpawn.ZRadius=100;
                ssSpawn.FRadius=100;
                for (int i=0; i<50; i++) {
                        if (pSpawn = NthNearestSpawn(&ssSpawn,i,(PSPAWNINFO)pCharSpawn)) if (pSpawn->Type == SPAWN_PLAYER) {
                                if (!stricmp(SpawnNameZone,pSpawn->Name)) {
                                        MonitorZoneID = GetCharInfo()->zoneId;
                                        MonitorID = pSpawn->SpawnID;
                                        AddWaypoint(MonitorID);

                                        MeMonitorX = GetCharInfo()->pSpawn->X; // MeMonitorX monitor your self
                                        MeMonitorY = GetCharInfo()->pSpawn->Y; // MeMonitorY monitor your self
                                        MeMonitorZ = GetCharInfo()->pSpawn->Z; // MeMonitorZ monitor your self
                                        StuckCheck(true);
                                }
                        }
                }
                return;
        }

        if ( DoOpenDoor ) {
                #define CAMERA_MODE *(int*)(((char*)pinstViewActor)+0x14)
                if ( (int)CAMERA_MODE != 0 ) {
                        CameraMode = (int)CAMERA_MODE;
                        CAMERA_MODE=0;
                } else {
                        MQ2Globals::ExecuteCmd(FindMappableCommand("USE"),0,0);
                        MQ2Globals::ExecuteCmd(FindMappableCommand("USE"),1,0);
                        MQ2Globals::ExecuteCmd(FindMappableCommand("USE"),0,0);

                        if ( CameraMode != 0 ) {
                                CAMERA_MODE = CameraMode;
                                CameraMode = 0;
                        }
                        DoOpenDoor=false;
                }
        }

        RecordingWaypoints();
        if (FollowState == FOLLOW_FOLLOWING && StatusState == STATUS_ON ) FollowSpawn();
        else if (FollowState == FOLLOW_PLAYING && StatusState == STATUS_ON ) FollowWaypoints();

        StuckCheck();
        lastClock = (unsigned long)clock();

        MeMonitorX = GetCharInfo()->pSpawn->X; // MeMonitorX monitor your self
        MeMonitorY = GetCharInfo()->pSpawn->Y; // MeMonitorY monitor your self
        MeMonitorZ = GetCharInfo()->pSpawn->Z; // MeMonitorZ monitor your self

        if ( FollowPath.size() ) {
                list<Position>::iterator CurList=FollowPath.begin();
                list<Position>::iterator EndList=FollowPath.end();

                Position LastList;                      // FollowPath
                
                if (FollowState == FOLLOW_FOLLOWING) {
                        LastList.Z = GetCharInfo()->pSpawn->Z;
                        LastList.Y = GetCharInfo()->pSpawn->Y;
                        LastList.X = GetCharInfo()->pSpawn->X;
                } else {
                        LastList.Z = CurList->Z;
                        LastList.Y = CurList->Y;
                        LastList.X = CurList->X;
                }
                MapClear();
                while(CurList!=EndList) {
                        pFollowPath.push_back(InitLine());
                        if(!pFollowPath[pFollowPath.size()-1]) break;

                        pFollowPath[pFollowPath.size()-1]->Layer=3;
                        if ( CurList->Warping ){
                                pFollowPath[pFollowPath.size()-1]->Color.ARGB=0xFFFF0000;
                        } else {
                                pFollowPath[pFollowPath.size()-1]->Color.ARGB=0xFF00FF00;
                        }
                        pFollowPath[pFollowPath.size()-1]->Start.Z=LastList.Z;
                        pFollowPath[pFollowPath.size()-1]->End.Z=CurList->Z;

                        pFollowPath[pFollowPath.size()-1]->Start.X=-LastList.X;
                        pFollowPath[pFollowPath.size()-1]->End.X=-CurList->X;

                        pFollowPath[pFollowPath.size()-1]->Start.Y=-LastList.Y;
                        pFollowPath[pFollowPath.size()-1]->End.Y=-CurList->Y;

                        LastList.Z = CurList->Z;
                        LastList.Y = CurList->Y;
                        LastList.X = CurList->X;

                        CurList++;
                }
        } else if (pFollowPath.size()) {
                MapClear();
        }
}

PLUGIN_API VOID OnEndZone(VOID) {
        DebugSpewAlways("MQ2AdvPath::OnZoned()");
        if( FollowState == FOLLOW_RECORDING && StatusState && SavePathName[0] != NULL && SavePathZone[0] != NULL )
                SavePath(SavePathName,SavePathZone);

        ReadINI();

        CreateBinds();
        if( PathNameZone[0] ) {
                MonitorZoneID = 0;
                FollowPath.clear();
                DoStop();
                MapClear();
                StuckCheck(true);
        } else if( SpawnNameZone[0] ) {
                WriteChatf("[MQ2AdvPath] Zoned now wating on (%s)",SpawnNameZone);
                FollowPath.clear();
                DoStop();
                MapClear();
                MonitorID = 0;
                MonitorWarp = false;
                StuckCheck(true);
        } else {
                ClearAll();
        }
}

PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn) {
        DebugSpewAlways("MQ2AdvPath::OnRemoveSpawn(%s)",pSpawn->Name);
        if( pSpawn->SpawnID == MonitorID ) MonitorID = 0;
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color) {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return 0;
        if(!stricmp(Line,"You have been summoned!") && FollowState) {
                WriteChatf("[MQ2AdvPath] summon Detected");
                ClearAll();
        } else if(!strnicmp(Line,"You will now auto-follow",24)) DoLft(true);
        else if(!strnicmp(Line,"You are no longer auto-follow",29) || !strnicmp(Line,"You must first target a group member to auto-follow.",52) ) {
                if (FollowState) MQFollowCommand(GetCharInfo()->pSpawn, "off"); 
                else MQFollowCommand(GetCharInfo()->pSpawn, "on");      
        }
        return 0;
}
