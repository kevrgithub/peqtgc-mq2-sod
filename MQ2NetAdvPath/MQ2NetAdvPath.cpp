//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Project: MQ2NetAdvPath.cpp
// Original Author of MQ2AdvPath: A_Enchanter_00
// Author of MQ2NetAdvPath: Itchybottom
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
//
// List of contributors: 
//
//	Vastlee (initial idea)
//	Drakk (reminding me about static_cast to get rid of a compiler warning)
//  Cornnuts (beta testing, bitching about characters walking, etc)
//  Lengingo (beta testing, bothering me to release a new version)
//
// changelog:
//	1.0 :: version reset to prevent confusion
//	       initial re-write, added modified NetBots interaction for X, Y, Z, Heading
//
//	1.1 :: removed map line drawing to trim the fat (use normal MQ2AdvPath for this)
//         added Animation TLO from NetBots
//
//	1.2 :: added jump and duck thanks to the addition of Animation TLO
//         added a check for /netfollow to be sure target is connected to NetBots
//         cleaned up some typecasts to remove new compiler warnings
//
//  1.3 :: removed the INI record and playback functions
//         added a help command for the idiots
//
//  1.4 :: Took out the random distance calculation, tightened some logic,
//         called Draelus a fag because he keeps bothering me for this plug-in.
//
//  1.5 :: Misc clean-up, got rid of DoWalk, stuck logic from MQ2AdvPath imported.
//
// TODO list :: STILL remove Evaluate() and replace it with DLL imports via EQLIB_API
//              Remove MQ2NetBots dependence (easy, already done on my beta version)
//				Actually make stuck logic useful
//				Add features in testing from 2.0 to the public version
//					Moonwalking
//					Course correction (CheckLOS, then clear waypoints and follow)
//					Clock cycle check to adjust waypoint path locations up or down
//					
//
// Complaints >/dev/null 2>/dev/null
//
// USE AT YOUR OWN RISK!
//

#include "../MQ2Plugin.h"

#include <vector>
#include <list>
#include <direct.h>

PreSetup("MQ2NetAdvPath");
PLUGIN_VERSION(1.5);

#define       FOLLOW_OFF				0
#define       FOLLOW_FOLLOWING			1

#define       STATUS_OFF				0
#define       STATUS_ON					1
#define       STATUS_PAUSED				2

#define       DISTANCE_BETWEEN_LOG		5

struct Position {
	FLOAT X;
	FLOAT Y;
	FLOAT Z;
	FLOAT Heading;
	char CheckPoint[MAX_STRING];
	bool Warping;
	long Animation;
} pPosition;

float __cdecl Evaluate(PCHAR zFormat, ...) { 
  char zOutput[MAX_STRING]={0}; va_list vaList; va_start(vaList,zFormat); 
  vsprintf(zOutput,zFormat,vaList); if(!zOutput[0]) return 1; 
  ParseMacroData(zOutput); return static_cast<float>(atof(zOutput)); 
}

long FollowState = FOLLOW_OFF;
long StatusState = STATUS_OFF;

long FollowSpawnDistance = 20;

long FollowIdle = 0;

long PlayWaypoint = 0;

long MonitorID = 0;
FLOAT MonitorX = 0;
FLOAT MonitorY = 0;
FLOAT MonitorZ = 0;
FLOAT MonitorHeading = 0;
long MonitorAnimation;
bool UseStuckLogic = true;
bool MonitorWarp = false;

FLOAT MeMonitorX = 0;
FLOAT MeMonitorY = 0;
FLOAT MeMonitorZ = 0;

CHAR Buffer[MAX_STRING] = {0};

list<Position>	FollowPath;

HMODULE EQWhMod=0; 
typedef HWND   (__stdcall *fEQW_GetDisplayWindow)(VOID); 
fEQW_GetDisplayWindow EQW_GetDisplayWindow=0; 

class MQ2NetAdvPathType *pNetAdvPathType=0;

VOID MQFollowCommand(PSPAWNINFO pChar, PCHAR szLine);
void ReleaseKeys();
void DoFwd(bool hold);
void DoBck(bool hold);
void DoLft(bool hold);
void DoRgt(bool hold);
void DoStop();
void LookAt(FLOAT X,FLOAT Y,FLOAT Z);
VOID ClearAll();
VOID ClearOne(list<Position>::iterator &CurList);
VOID AddWaypoint(long SpawnID,bool Warping=false);
VOID FollowSpawn();
VOID FollowWaypoints();
VOID RecordingWaypoints();
void StuckCheck(bool reset=false);
void __stdcall WarpingDetect(unsigned int ID, void *pData, PBLECHVALUE pValues);

unsigned long thisClock = clock();
unsigned long lastClock = clock();
long DistanceMod = 0;

VOID MQFollowCommand(PSPAWNINFO pChar, PCHAR szLine) {
	DebugSpewAlways("MQ2NetAdvPath::MQFollowCommand()");
	if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
	bool doFollow = false;
	long MyTarget=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):0;

	if ( szLine[0]==0 ) {
		if(MonitorID || FollowPath.size()) {
			ClearAll();
			return;
		} else {
			if(!Evaluate("${NetBots[${Target}].ID}")) { 
				WriteChatf("[MQ2NetAdvPath] Target not connected to NetBots or needs location update");
				return;
				}
			doFollow = true;
		}
	} else {
		long iParm=0;
		do {
			GetArg(Buffer,szLine,++iParm);
			if(Buffer[0]==0) break;
			if(!strnicmp(Buffer,"on",2)) {
			if(!Evaluate("${NetBots[${Target}].ID}")) { 
				WriteChatf("[MQ2NetAdvPath] Target not connected to NetBots");
				return;
				}
				doFollow = true;
			} else if(!strnicmp(Buffer,"help",4)) {
				WriteChatf("Syntax: /netfollow <arg>");
				WriteChatf("Valid arguments:");
				WriteChatf("on | off/stop | pause | unpause/resume | clear");
				return;
			} else if(!strnicmp(Buffer,"clear",5)) {
				WriteChatf("[MQ2NetAdvPath] Waypoints cleared");
				ClearAll();
				doFollow = true;
				return;
			} else if(!strnicmp(Buffer,"off",3) || !strnicmp(Buffer,"stop",4)) {
				ClearAll();
				return;
			} else if(!strnicmp(Buffer,"pause",5)) {
				WriteChatf("[MQ2NetAdvPath] Follow paused");
				DoStop();
				StatusState = STATUS_PAUSED;
				return;
			} else if(!strnicmp(Buffer,"unpause",7) || !strnicmp(Buffer,"resume",6)) {
				WriteChatf("[MQ2NetAdvPath] Follow resumed");
				StatusState = STATUS_ON;
				return;
				doFollow = true;
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
			AddWaypoint(MonitorID);
			FollowState = FOLLOW_FOLLOWING;
			StatusState = STATUS_ON;
			WriteChatf("[MQ2NetAdvPath] Following %s", pSpawn->Name);

			MeMonitorX = GetCharInfo()->pSpawn->X;
			MeMonitorY = GetCharInfo()->pSpawn->Y;
			MeMonitorZ = GetCharInfo()->pSpawn->Z;
		}
	}
}

void ReleaseKeys() {
	DoFwd(false);
	DoBck(false);
	DoRgt(false);
	DoLft(false);
}



void DoFwd(bool hold) {
	static bool held = false;
	if ( hold ) {
		DoBck(false);
		if (!held) MQ2Globals::ExecuteCmd(FindMappableCommand("forward"),1,0);
		held = true;
	} else {
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

	if (GetCharInfo()->pSpawn->UnderWater==5) GetCharInfo()->pSpawn->CameraAngle = (FLOAT)( atan2(Z - GetCharInfo()->pSpawn->Z, (FLOAT)GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y, X,Y)) * 256.0f / PI);
	else if (GetCharInfo()->pSpawn->Levitate==2) {
		if ( Z < GetCharInfo()->pSpawn->Z-5) GetCharInfo()->pSpawn->CameraAngle = -45.0f;
		else if ( Z > GetCharInfo()->pSpawn->Z+5) GetCharInfo()->pSpawn->CameraAngle = 45.0f;
		else GetCharInfo()->pSpawn->CameraAngle = 0.0f;
	} else GetCharInfo()->pSpawn->CameraAngle = 0.0f;
	gLookAngle=10000.0f;
}

VOID ClearAll() {
	if( MonitorID || FollowPath.size() ) WriteChatf("[MQ2NetAdvPath] Stopped");
	FollowPath.clear();
	FollowIdle = MonitorID = 0;
	DoStop();

	FollowState = FOLLOW_OFF;
	StatusState = STATUS_OFF;
	MonitorWarp = false;	

	StuckCheck(true);
}

VOID ClearOne(list<Position>::iterator &CurList) {
	list<Position>::iterator PosList;
	PosList=CurList;
	CurList++;
	FollowPath.erase(PosList);
}

VOID AddWaypoint(long SpawnID,bool Warping) {
	if( PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(SpawnID) ) {	
		Position MonitorPosition;

		MonitorPosition.Warping = Warping;

		MonitorPosition.X = MonitorX = Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[2,:]}");
		MonitorPosition.Y = MonitorY = Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[1,:]}");
		MonitorPosition.Z = MonitorZ = Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[3,:]}");
		MonitorPosition.Heading = MonitorHeading = Evaluate("${NetBots[${NetAdvPath.Monitor}].Heading}");
		MonitorPosition.Animation = MonitorAnimation = (LONG)Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[4,:]}");
		strcpy(MonitorPosition.CheckPoint,"");

		FollowPath.push_back(MonitorPosition);
	}
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
			WriteChatf("[MQ2NetAdvPath] Warping Detected on SELF");
		}
		if( !FollowPath.size() ) return;

		list<Position>::iterator CurList=FollowPath.begin();
		list<Position>::iterator EndList=FollowPath.end();

		HWND EQhWnd=*(HWND*)EQADDR_HWND;
		if (EQW_GetDisplayWindow) EQhWnd=EQW_GetDisplayWindow();

		bool run = false;

		if( CurList->Warping && GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,CurList->X,CurList->Y,CurList->Z) > 50 ) {
			if ( !MonitorWarp ) {
				WriteChatf("[MQ2NetAdvPath] Warp detected, waiting...");
				DoStop();
			}
			MonitorWarp = true;
			return;
		}
		MonitorWarp = false;

		if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(MonitorID)) {
			if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[2,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[1,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[3,:]}")) >= 50 ) run = true;
			if( GetDistance3D(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,GetCharInfo()->pSpawn->Z,Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[2,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[1,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[3,:]}")) <= FollowSpawnDistance ) {				
				DoStop();
				return;
			}
		} else if( (FollowPath.size()*DISTANCE_BETWEEN_LOG) >= (unsigned int)FollowSpawnDistance ) run = true;

		if( GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > DISTANCE_BETWEEN_LOG+DistanceMod ) {
			LookAt(CurList->X,CurList->Y,CurList->Z);
			
			switch(CurList->Animation) {
				case 19: case 20: 
					if(GetCharInfo()->standstate==STANDSTATE_STAND) MQ2Globals::ExecuteCmd(FindMappableCommand("jump"),1,0);
					break;
				case 22: case 24:
					if(GetCharInfo()->standstate==STANDSTATE_STAND) MQ2Globals::ExecuteCmd(FindMappableCommand("duck"),1,0); 
					break;
				default: 
					if(GetCharInfo()->standstate==STANDSTATE_DUCK) MQ2Globals::ExecuteCmd(FindMappableCommand("duck"),1,0); 
					break;
			}

			DoFwd(true);
			FollowIdle = 0;
		} else {
				ClearOne(CurList);
				if( CurList != EndList ) {
					if(CurList->Warping) return;
				} else {
					DoStop();
					if( !MonitorID ) {
						ClearAll();
					}
				}
		}
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
				if( GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y) > DISTANCE_BETWEEN_LOG+DistanceMod ) {
					LookAt(CurList->X,CurList->Y,CurList->Z);
					FollowIdle = 0;
					break;
				} else {
					if( FollowPath.size() == PlayWaypoint ) {
					    PlayWaypoint = 1;
						} else {
							ClearAll();
						}

					if (GetForegroundWindow()==EQhWnd) {
						PlayWaypoint++;
					} else {
						PlayWaypoint = PlayWaypoint +1;

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

VOID StuckCheck(bool reset) {
	static time_t JumpTimer = time(NULL);
	static time_t StuckMonitor = time(NULL);

	static bool Jump = true;

	static float StuckMonitorX = 0;
	static float StuckMonitorY = 0;
	if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
	if( FollowPath.size() && UseStuckLogic && !reset && StatusState == STATUS_ON && ( FollowState == FOLLOW_FOLLOWING ) ) {
		list<Position>::iterator CurList=FollowPath.begin();

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
				}

				if ( GetDistance(StuckMonitorX,StuckMonitorY) > 2 ) {
					reset = true;
				} else if ( ( !pulseMoved || ( pulseMoved < 0.1 && pulseMoved < expectedPulseMoved ) ) && StuckMonitor+TimerMod < now ) {
					StuckMonitor = time(NULL);
					WriteChatf("[MQ2NetAdvPath] Aborting because I'm stuck.");			
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
				if( GetDistance3D(MonitorX,MonitorY,MonitorZ,Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[2,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[1,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[3,:]}")) > 100 ) {
				WriteChatf("[MQ2NetAdvPath] Warping detected on %s",pSpawn->Name);
				AddWaypoint(MonitorID,true);
			} else if( GetDistance3D(MonitorX,MonitorY,MonitorZ,Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[2,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[1,:]}"),Evaluate("${NetBots[${NetAdvPath.Monitor}].Location.Arg[3,:]}")) >= DISTANCE_BETWEEN_LOG ) AddWaypoint(MonitorID);
		}
	}
}

class MQ2NetAdvPathType : public MQ2Type {
	private:
		char Temps[MAX_STRING];
	public:
		enum NetAdvPathMembers {
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
			Status=14,
			Paused=15,
			WaitingWarp=16,
			StopDistance=17,
			CheckPoint=18
		};
		MQ2NetAdvPathType():MQ2Type("NetAdvPath") {
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

			PMQ2TYPEMEMBER pMember=MQ2NetAdvPathType::FindMember(Member); 
			if(pMember) switch((NetAdvPathMembers)pMember->ID) {
				case Active:										// Plugin on and Ready
					Dest.DWord=(gbInZone && GetCharInfo() && GetCharInfo()->pSpawn);
					Dest.Type=pBoolType;
					return true;
				case State:											// FollowState, 0 = off, 1 = Following, 2 = Playing, 3 = Recording
					Dest.DWord=FollowState;
					Dest.Type=pIntType;
					return true;
				case Waypoints:										// Number of Waypoints
					Dest.DWord=FollowPath.size();
					Dest.Type=pIntType;
					return true;
				case NextWaypoint:									// Next Waypoint
					Dest.DWord=PlayWaypoint;
					Dest.Type=pIntType;
					return true;
				case Y:												// Waypoint Y
					while(CurList!=EndList) {						
						if(  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) {
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
				case X:												// Waypoint X
					while(CurList!=EndList) {						
						if(  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) {
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
				case Z:												// Waypoint Z
					while(CurList!=EndList) {
						if(  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) {
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
				case Monitor:										// Spawn your following
					Dest.Ptr=(PSPAWNINFO)GetSpawnByID(MonitorID);
					Dest.Type=pSpawnType;
					return true;
				case Idle:											// FollowIdle time when following and not moving
					Dest.DWord=(FollowState && FollowIdle)?(((long)clock()-FollowIdle)/1000):0;
					Dest.Type=pIntType;
					return true;
				case Length:										// Estimated length off the follow path
					if( FollowPath.size() ) {
						list<Position>::iterator CurList=FollowPath.begin();
						TheLength = GetDistance(GetCharInfo()->pSpawn->X,GetCharInfo()->pSpawn->Y,CurList->X,CurList->Y);
						if( FollowPath.size() > 1 )	TheLength = ((FollowPath.size()-1)*DISTANCE_BETWEEN_LOG)+TheLength;
					}
					Dest.Float=TheLength;
					Dest.Type=pFloatType;
					return true;
				case Following:
					Dest.DWord=(FollowState==FOLLOW_FOLLOWING);
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
					Dest.DWord=(DISTANCE_BETWEEN_LOG+DistanceMod);
					Dest.Type=pIntType;
					return true;
				case CheckPoint:
					strcpy(Temps,"NULL");
					while(CurList!=EndList) {						
						if( ( i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) || (  i == atol(Index) || ( Index[0] != 0 && !stricmp(Index,CurList->CheckPoint) ) ) ) {
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
		~MQ2NetAdvPathType() { } 
}; 

BOOL dataNetAdvPath(PCHAR szName, MQ2TYPEVAR &Dest) {
	Dest.DWord=1;
	Dest.Type=pNetAdvPathType;
	return true;
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID) {
	DebugSpewAlways("Initializing MQ2NetAdvPath");
	if (EQWhMod=GetModuleHandle("eqw.dll")) EQW_GetDisplayWindow=(fEQW_GetDisplayWindow)GetProcAddress(EQWhMod,"EQW_GetDisplayWindow");

    AddCommand("/netfollow",MQFollowCommand);
	pNetAdvPathType = new MQ2NetAdvPathType;
	AddMQ2Data("NetAdvPath",dataNetAdvPath);
	srand(unsigned(time(NULL)));
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID) {
	DebugSpewAlways("Shutting down MQ2NetAdvPath");

	RemoveCommand("/netfollow");
	delete pNetAdvPathType;
	RemoveMQ2Data("NetAdvPath");

	ClearAll();
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID) {
	if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;

	thisClock = (unsigned long)clock();
	DistanceMod = ((thisClock-lastClock)*(thisClock-lastClock))/1000/2;
	StuckCheck();
	lastClock = (unsigned long)clock();

	RecordingWaypoints();
	if (FollowState == FOLLOW_FOLLOWING && StatusState == STATUS_ON) FollowSpawn();
	lastClock = (unsigned long)clock();

	MeMonitorX = GetCharInfo()->pSpawn->X; 
	MeMonitorY = GetCharInfo()->pSpawn->Y; 
	MeMonitorZ = GetCharInfo()->pSpawn->Z; 

	if ( FollowPath.size() ) {
		list<Position>::iterator CurList=FollowPath.begin();
		list<Position>::iterator EndList=FollowPath.end();

		Position LastList;
		
		if (FollowState == FOLLOW_FOLLOWING) {
			LastList.Z = GetCharInfo()->pSpawn->Z;
			LastList.Y = GetCharInfo()->pSpawn->Y;
			LastList.X = GetCharInfo()->pSpawn->X;
		} else {
			LastList.Z = CurList->Z;
			LastList.Y = CurList->Y;
			LastList.X = CurList->X;
		}
		while(CurList!=EndList) {

			LastList.Z = CurList->Z;
			LastList.Y = CurList->Y;
			LastList.X = CurList->X;

			CurList++;
		}
	}
}



PLUGIN_API VOID OnEndZone(VOID) {
	DebugSpewAlways("MQ2NetAdvPath::OnZoned()");
	if( StatusState )
	ClearAll();
}

PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn) {
	DebugSpewAlways("MQ2NetAdvPath::OnRemoveSpawn(%s)",pSpawn->Name);
	if( pSpawn->SpawnID == MonitorID ) MonitorID = 0;
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color) {
	if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return 0;
	if(!stricmp(Line,"You have been summoned!") && FollowState) {
		WriteChatf("[MQ2NetAdvPath] Summon detected");
		ClearAll();
	} else if(!strnicmp(Line,"You will now auto-follow",24)) DoLft(true);
	else if(!strnicmp(Line,"You are no longer auto-follow",29) || !strnicmp(Line,"You must first target a group member to auto-follow.",52) ) {
		if (FollowState) MQFollowCommand(GetCharInfo()->pSpawn, "off");	
		else MQFollowCommand(GetCharInfo()->pSpawn, "on");	
	}
	return 0;
}