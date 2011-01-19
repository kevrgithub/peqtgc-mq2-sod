/*
MQ2MoveUtils plugin (2004.06.23) - tonio
- Updated 2005.06.26 by Quagmire
- Updated 2005.07.09 by Outlander
- Updated 2005.07.14 
- Updated 2005.07.19 
- Updated 2005.09.19 
- Updated 2005.09.21 
- Updated 2005.09.26 
- Updated 2005.09.28 
- Updated 2005.10.22 
- Updated 2005.10.23 
- Updated 2005.12.13 based off VZMule's post
- Updated 2005.12.19 based off somelamedruid's post
- Updated 2005.12.22
- Updated 2006.02.27
- updated 2006.02.28
- Updated 2006.03.01
- Updated 2006.03.13
- Updated 2006.03.16
- Updated 2006.04.04
- Updated 2006.04.23
- Updated 2006.04.24
- Updated 2007.05.10 by rswiders (added clockwise/counterclockwise circling; added forward and kiting (backwards) facing circling)
- Updated 2007.05.18 added loose to /moveto
- Updated 2007.08.11 added followpath (fp) command 

* 2007.08.11 - Added fp command to go to a person or location in a zone 
* Additions 2007.5.18
Added loose option to /moveto to be similar to /stick loose and use
  gFaceAngle instead of directly setting heading.  This also just
  turns without moving if angle is to great to avoid circling moveto
  location.
Fixed some 180deg turns which were += 265 instead of += 256.
* Missed a return in new TLO StickTarget member
* Added TLO object StickTarget
* MoveTo, and Stick will stand you up if sitting
  Changed a message display routine so it would not give you an incorrect break stick message
* Fixed the CTD bug for the third time.
* Fixed the CTD bug (again)
Opened the StickCommand, CircleCommand, MoveToCommand, and MakeCampCommand functions for other plugins for Sorcier
Added StandUp on /stick for A_Druid_00 (thanks Sorcier, I just copied your StandUp() code!)
* Fixed a CTD bug in the command processor for Stick.
Added in Sorcier's run/walk logic
* Corrected a bug with /stick if your target was further away than your BreakDist from the MQ2MoveUtils.ini file
then /stick would think your target had warped and would not stick. Added some code to the bottom of the /stick command to account for this issue.
Added save of mPause setting so you can save your preference for mPause.
* Changes
Reduced Sensitivity on MoveStuck Logic by default from 0.8 to 0.1
Added Delay to resume movement after manually moving with mpause enabled.
While each command has mindelay and maxdelay they all set the same variables so this only
needs to be done once.
Added checks to stick, moveto, and circle to not kick in the MoveStuck Logic if you are stunned
* Additions 2005.12.22
Added OnRemoveSpawn sub to help with crashes as posted by DKAA
Added /moveto id <spawn id | empty for current target> functionality as discussed on boards.
* Corrections 2005.12.19
added stickOn=true; to /stick hold and /stick id command parsers
* Corrections 2005.12.13
Structure changes in last patch corrected in this version
* Corrections 2005.10.23
/makecamp is now a toggle for on/off
with mpause enabled any movement keys would spew a "no longer sticking to anything" message this has been fixed.
mpause option is now a toggle so if it was on doing /makecamp mpause will turn it off
INI load and save were reversed, this is now corrected
maxdelay variable being loaded from mindelay INI value, this has been corrected
* Addition 2005.10.22
Added /makecamp functionality
Added /stick help, /circle help, /moveto help, and of course /makecamp help
Added some commands to all the functions
Changed the way some commands are parsed and changed the help command look
* Corrections 2005.09.28
Corrected problem with IsBardClass function, thanks BardOMatic and DKAA
* Addition 2005.09.28
Added HoTT check to stick, if sticking to a MOB or PC and you are their target
then if you are doing stick !front, behind, or pin you just do a a normal stick until
you are not the HoTT target then you go back to what ever option you had before.
* Addition 2005.09.26
Added new option:
/stick !front
This option will keep you not in front of the mob, so off to the side or behind.
This option should reduce the amount your toon moves around during a fight with a mob when you are not the
target as your will only adjust your position if you you are in the front 180 degrees of the mob, as long as you are
in the behind 180 degrees you will not move around just because the mob shifts.
* Corrections 2005.09.21
Changed movestuck logic for underwater to not kick in till you are not moving at all; pulse average = 0.
* Corrections 2005.09.19
Changed IsBardClass function to use GetCharInfo2() instead of the old GetCharInfo() function.
* Corrections 2005.07.19
Modified /stick movestuck logic when you are close to mob and are switched to walk mode, this was causing
movestuck logic to be executed incorrectly.
* Corrections and additions 2005.07.14
Added break Circle functionality so that when you manually move Circle is turned off
Added MoveStuck logic to MoveTo and Circle commands
Adjusted the stuck distance to take into account the SpeedMultiplier currently on the character.
Adjusted the the MoveStuck stuckDist to be 1/3 of what it normally is if your under water.
* Corrections and additions 2005.07.08
Added Version Number at top of help commands
Added /stick id <spawn id> functionality
Added break MoveTo functionality so that when you manually move MoveTo is turned off

/stick" command by tonio
/circle" command lifted from CyberTech's MQ2Twister plugin
/moveto" command from rswiders
/moveto loose command from deadchicken
/makecamp command by Outlander
*/

#include "../MQ2Plugin.h"

PLUGIN_VERSION(7.0518);
PreSetup("MQ2MoveUtils");
CHAR szVersion[30]="MQ2MoveUtils Version 20070518";
CHAR szMsg[MAX_STRING]={0};

float TurnIncrement=10.0f;
bool bDebug=false;
bool bCircling=false;
bool bCirclingWasOn=false;
bool bDrunken=false;
bool bClockwise=true;
bool bBackwards=false;
float CircleX=0.0f;
float CircleY=0.0f;
float CircleRadius=0.0f;
SYSTEMTIME stPrevCirc;

float getRand(float n);
int millisDiff(SYSTEMTIME &stCurr, SYSTEMTIME &stPrev);
//void CircleCommand(PSPAWNINFO pChar, PCHAR szLine);
//void StickCommand(PSPAWNINFO pChar, PCHAR szLine);
//void MakeCampCommand(PSPAWNINFO pChar, PCHAR szLine);
//void MoveToCommand(PSPAWNINFO pChar, PCHAR szLine);
// FOLLOWPATH 
bool bMoveToTrader=false; 
bool fpdebug=false; 
PLUGIN_API VOID InitializeFollowPath(PSPAWNINFO pChar, PCHAR szLine); 
void HandleMoveToTrader(); 
// END FOLLOWPATH 
PLUGIN_API VOID CircleCommand(PSPAWNINFO pChar, PCHAR szLine);
PLUGIN_API VOID StickCommand(PSPAWNINFO pChar, PCHAR szLine);
PLUGIN_API VOID MakeCampCommand(PSPAWNINFO pChar, PCHAR szLine);
PLUGIN_API VOID MoveToCommand(PSPAWNINFO pChar, PCHAR szLine); 
VOID DoUnstickBind(PCHAR Name, BOOL Down);
void HandleMakeCamp();
void HandleMoveTo();
void ReleaseKeys();
void DoWalk(bool walk = false);
void DoFwd(bool hold, bool walk = false);
void DoBck(bool hold);
void DoLft(bool hold);
void DoRgt(bool hold);
void Load_INI(void);
void Save_INI(void);
void StandUp();
bool IsBardClass(void);
void stickText();
void breakStick(bool stopMoving = true, bool quite = false);
void breakCircle(bool stopMoving = true, bool quite = false);
void breakMoveTo(bool stopMoving = true, bool quite = false);
float angularDistance(float h1, float h2);
void WriteSetting(PCHAR Setting, int Current, PCHAR ShowHelp, bool BoolType);
void DebugSpew();

bool MoveBindsLoaded=false;
bool stickOn=false;
bool stickWasOn=false;
bool setDist=false;
bool PauseLogic=false;
bool stickHold=false;
bool moveBehind=false;
bool moveBehindOnce=false;
bool moveBack=false;
bool movePin=false;
bool moveNotFront=false;
bool casting=false;
bool mPause=false;
bool prevMoveBehind=false;
bool prevMovePin=false;
bool looseStick=false;
bool looseMoveto=false;
bool underwater=false;
bool stickhasmovedfwd=false;
short stickVerbosity=0;
short keysDown=0;
float stickDist=0.0;
float breakDist=250.0;
float currentDist=0.0;
float stickDistMod=0.0;
float stickDistModP=1.0;
PSPAWNINFO stickTarget;
eSpawnType stickTarget_Type;
SYSTEMTIME stPrevStick;

bool autoPauseEnabled=true;
bool breakDistEnabled=true;
bool breakOnWarpEnabled=true;
bool breakOnGateEnabled=true;

// stuck added by Outlander
int stuck=0;
int stuckCheck=0;
int stuckFree=0;
float prevX=0.0f;
float prevY=0.0f;
float prevZ=0.0f;
float stuckDist=0.0f;
float turnDirection=0.0f;
float pulseAvg=10.0f;
bool stuckLogic;

bool bMoveToOn=false;
float LocX=0.0f;
float LocY=0.0f;
float moveDist=10.0;
float moveDistMod=0.0;

//Make Camp Variables
bool bMakeCamp=false;
bool bLeash=false;
bool bMakeCampReturn=false;
float anchorX=0.0f;
float anchorY=0.0f;
float leashLength=0.0f;
float campRadius=0.0f;
int mindelay=0;
int maxdelay=0;
int returnTime=0;
SYSTEMTIME stReturnTime;

void DebugSpew() {
	char szTemp[MAX_STRING];

	sprintf(szTemp,"%s",bCircling?"true":"false");
	WritePrivateProfileString("Debug","bCircling",szTemp,INIFileName);
	sprintf(szTemp,"%s",bCirclingWasOn?"true":"false");
	WritePrivateProfileString("Debug","bCirclingWasOn",szTemp,INIFileName);
	sprintf(szTemp,"%s",bDrunken?"true":"false");
	WritePrivateProfileString("Debug","bDrunken",szTemp,INIFileName);
	sprintf(szTemp,"%s",MoveBindsLoaded?"true":"false");
	WritePrivateProfileString("Debug","MoveBindsLoaded",szTemp,INIFileName);
	sprintf(szTemp,"%s",stickOn?"true":"false");
	WritePrivateProfileString("Debug","stickOn",szTemp,INIFileName);
	sprintf(szTemp,"%s",stickWasOn?"true":"false");
	WritePrivateProfileString("Debug","stickWasOn",szTemp,INIFileName);
	sprintf(szTemp,"%s",setDist?"true":"false");
	WritePrivateProfileString("Debug","setDist",szTemp,INIFileName);
	sprintf(szTemp,"%s",PauseLogic?"true":"false");
	WritePrivateProfileString("Debug","PauseLogic",szTemp,INIFileName);
	sprintf(szTemp,"%s",stickHold?"true":"false");
	WritePrivateProfileString("Debug","stickHold",szTemp,INIFileName);
	sprintf(szTemp,"%s",moveBehind?"true":"false");
	WritePrivateProfileString("Debug","moveBehind",szTemp,INIFileName);
	sprintf(szTemp,"%s",moveBehindOnce?"true":"false");
	WritePrivateProfileString("Debug","moveBehindOnce",szTemp,INIFileName);
	sprintf(szTemp,"%s",moveBack?"true":"false");
	WritePrivateProfileString("Debug","moveBack",szTemp,INIFileName);
	sprintf(szTemp,"%s",movePin?"true":"false");
	WritePrivateProfileString("Debug","movePin",szTemp,INIFileName);
	sprintf(szTemp,"%s",moveNotFront?"true":"false");
	WritePrivateProfileString("Debug","moveNotFront",szTemp,INIFileName);
	sprintf(szTemp,"%s",casting?"true":"false");
	WritePrivateProfileString("Debug","casting",szTemp,INIFileName);
	sprintf(szTemp,"%s",mPause?"true":"false");
	WritePrivateProfileString("Debug","mPause",szTemp,INIFileName);
	sprintf(szTemp,"%s",prevMoveBehind?"true":"false");
	WritePrivateProfileString("Debug","prevMoveBehind",szTemp,INIFileName);
	sprintf(szTemp,"%s",prevMovePin?"true":"false");
	WritePrivateProfileString("Debug","prevMovePin",szTemp,INIFileName);
	sprintf(szTemp,"%s",looseStick?"true":"false");
	WritePrivateProfileString("Debug","looseStick",szTemp,INIFileName);
	sprintf(szTemp,"%s",underwater?"true":"false");
	WritePrivateProfileString("Debug","underwater",szTemp,INIFileName);
	sprintf(szTemp,"%s",stickhasmovedfwd?"true":"false");
	WritePrivateProfileString("Debug","stickhasmovedfwd",szTemp,INIFileName);
	sprintf(szTemp,"%s",autoPauseEnabled?"true":"false");
	WritePrivateProfileString("Debug","autoPauseEnabled",szTemp,INIFileName);
	sprintf(szTemp,"%s",breakDistEnabled?"true":"false");
	WritePrivateProfileString("Debug","breakDistEnabled",szTemp,INIFileName);
	sprintf(szTemp,"%s",breakOnWarpEnabled?"true":"false");
	WritePrivateProfileString("Debug","breakOnWarpEnabled",szTemp,INIFileName);
	sprintf(szTemp,"%s",breakOnGateEnabled?"true":"false");
	WritePrivateProfileString("Debug","breakOnGateEnabled",szTemp,INIFileName);
	sprintf(szTemp,"%s",bMoveToOn?"true":"false");
	WritePrivateProfileString("Debug","bMoveToOn",szTemp,INIFileName);
	sprintf(szTemp,"%s",stuckLogic?"true":"false");
	WritePrivateProfileString("Debug","stuckLogic",szTemp,INIFileName);
	sprintf(szTemp,"%s",bMakeCamp?"true":"false");
	WritePrivateProfileString("Debug","bMakeCamp",szTemp,INIFileName);
	sprintf(szTemp,"%s",bLeash?"true":"false");
	WritePrivateProfileString("Debug","bLeash",szTemp,INIFileName);
	sprintf(szTemp,"%s",bMakeCampReturn?"true":"false");
	WritePrivateProfileString("Debug","bMakeCampReturn",szTemp,INIFileName);

	sprintf(szTemp,"%.1f",CircleX);
	WritePrivateProfileString("Debug","CircleX",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",CircleY);
	WritePrivateProfileString("Debug","CircleY",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",CircleRadius);
	WritePrivateProfileString("Debug","CircleRadius",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",stickDist);
	WritePrivateProfileString("Debug","stickDist",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",breakDist);
	WritePrivateProfileString("Debug","breakDist",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",currentDist);
	WritePrivateProfileString("Debug","currentDist",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",stickDistMod);
	WritePrivateProfileString("Debug","stickDistMod",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",stickDistModP);
	WritePrivateProfileString("Debug","stickDistModP",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",prevX);
	WritePrivateProfileString("Debug","prevX",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",prevY);
	WritePrivateProfileString("Debug","prevY",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",prevZ);
	WritePrivateProfileString("Debug","prevZ",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",stuckDist);
	WritePrivateProfileString("Debug","stuckDist",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",turnDirection);
	WritePrivateProfileString("Debug","turnDirection",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",pulseAvg);
	WritePrivateProfileString("Debug","pulseAvg",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",LocX);
	WritePrivateProfileString("Debug","LocX",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",LocY);
	WritePrivateProfileString("Debug","LocY",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",moveDist);
	WritePrivateProfileString("Debug","moveDist",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",moveDistMod);
	WritePrivateProfileString("Debug","moveDistMod",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",anchorX);
	WritePrivateProfileString("Debug","anchorX",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",anchorY);
	WritePrivateProfileString("Debug","anchorY",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",leashLength);
	WritePrivateProfileString("Debug","leashLength",szTemp,INIFileName);
	sprintf(szTemp,"%.1f",campRadius);
	WritePrivateProfileString("Debug","campRadius",szTemp,INIFileName);

	sprintf(szTemp,"%d",stickVerbosity);
	WritePrivateProfileString("Debug","stickVerbosity",szTemp,INIFileName);
	sprintf(szTemp,"%d",keysDown);
	WritePrivateProfileString("Debug","keysDown",szTemp,INIFileName);

	sprintf(szTemp,"%i",stuck);
	WritePrivateProfileString("Debug","stuck",szTemp,INIFileName);
	sprintf(szTemp,"%i",stuckCheck);
	WritePrivateProfileString("Debug","stuckCheck",szTemp,INIFileName);
	sprintf(szTemp,"%i",stuckFree);
	WritePrivateProfileString("Debug","stuckFree",szTemp,INIFileName);
	sprintf(szTemp,"%i",mindelay);
	WritePrivateProfileString("Debug","mindelay",szTemp,INIFileName);
	sprintf(szTemp,"%i",maxdelay);
	WritePrivateProfileString("Debug","maxdelay",szTemp,INIFileName);
	sprintf(szTemp,"%i",returnTime);
	WritePrivateProfileString("Debug","returnTime",szTemp,INIFileName);
	sprintf(szTemp,"%i",bMoveToTrader);
	WritePrivateProfileString("Debug","bMoveToTrader",szTemp,INIFileName);
	bDebug=false;
	//SYSTEMTIME stReturnTime;
	//SYSTEMTIME stPrevCirc;
	//PSPAWNINFO stickTarget;
	//eSpawnType stickTarget_Type;
	//SYSTEMTIME stPrevStick;

}

void StandUp() {
  if(GetCharInfo()->standstate == STANDSTATE_STAND) return;
  EzCommand("/stand");
  //MQ2Globals::ExecuteCmd(FindMappableCommand("SIT_STAND"),1,0);
  //MQ2Globals::ExecuteCmd(FindMappableCommand("SIT_STAND"),0,0);
}

//MakeCamp Related Functionality
class MQ2MakeCampType *pMakeCampType = 0;

class MQ2MakeCampType : public MQ2Type {
public:
	enum MakeCampMembers {
		Status=1,
		Leash=2,
		AnchorX=3,
		AnchorY=4,
		LeashLength=5,
		CampRadius=6,
		MinDelay=7,
		MaxDelay=8,
		Returning=9,
	};

	MQ2MakeCampType():MQ2Type("makecamp")
	{
		TypeMember(Status);
		TypeMember(Leash);
		TypeMember(AnchorX);
		TypeMember(AnchorY);
		TypeMember(LeashLength);
		TypeMember(CampRadius);
		TypeMember(MinDelay);
		TypeMember(MaxDelay);
		TypeMember(Returning);
	}

	~MQ2MakeCampType()
	{
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
	{
		PMQ2TYPEMEMBER pMember=MQ2MakeCampType::FindMember(Member);
		if (!pMember)
			return false;
		switch((MakeCampMembers)pMember->ID)
		{
		case Status:
			strcpy(DataTypeTemp,"OFF");
			if( bMakeCamp ) {
				strcpy(DataTypeTemp,"ON");
		 }
			if( PauseLogic ) {
				strcpy(DataTypeTemp,"PAUSED");
		 }
			Dest.Ptr=DataTypeTemp;
			Dest.Type=pStringType;
			return true;
		case Leash:
			Dest.DWord=bLeash;
			Dest.Type=pBoolType;
			return true;
		case AnchorX:
			Dest.Float=anchorX;
			Dest.Type=pFloatType;
			return true;
		case AnchorY:
			Dest.Float=anchorY;
			Dest.Type=pFloatType;
			return true;
		case LeashLength:
			Dest.Float=leashLength;
			Dest.Type=pFloatType;
			return true;
		case CampRadius:
			Dest.Float=campRadius;
			Dest.Type=pFloatType;
			return true;
		case MinDelay:
			Dest.DWord=mindelay;
			Dest.Type=pIntType;
			return true;
		case MaxDelay:
			Dest.DWord=maxdelay;
			Dest.Type=pIntType;
			return true;
		case Returning:
			Dest.DWord=bMakeCampReturn;
			Dest.Type=pBoolType;
			return true;
		}
		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination)
	{
		strcpy(Destination,"OFF");
		if( bMakeCamp ) {
			strcpy(Destination,"ON");
		}
		if( PauseLogic ) {
			strcpy(Destination,"PAUSED");
		}
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source)
	{
		return false;
	}
	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source)
	{
		return false;
	}
};

BOOL dataMakeCamp(PCHAR szName, MQ2TYPEVAR &Ret)
{
	Ret.DWord=1;
	Ret.Type=pMakeCampType;
	return true;
}

void MakeCampHelp() {
	char Buffer[64];
	WriteChatColor(szVersion,CONCOLOR_YELLOW);
	sprintf(Buffer,(bMakeCamp)?"\agon\ax":"\agoff\ax");
	WriteChatf("/makecamp [on|off] (%s) on drops anchor at current location", Buffer);
	WriteChatf("/makecamp [<y> <x>] (\ag%.2f\ax \ag%.2f\ax) anchor location",anchorY, anchorX);
	WriteChatf("/makecamp [mindelay <time in ms>] (\ag%d\ax) return to camp", mindelay);
	WriteChatf("/makecamp [maxdelay <time in ms>] (\ag%d\ax) return to camp", maxdelay);
	sprintf(Buffer,(bLeash)?"\agon\ax":"\agoff\ax");
	WriteChatf("/makecamp [leash] (%s) toggle", Buffer);
	WriteChatf("/makecamp [leash [<dist>]] (\ag%.2f\ax) implicit on",leashLength);
	WriteChatf("/makecamp [radius <dist>] (\ag%.2f\ax)",campRadius);
	sprintf(Buffer,(mPause)?"\agon\ax":"\agoff\ax");
	WriteChatf("/makecamp [mpause] (%s) manual control pause or break", Buffer);
	sprintf(Buffer,(PauseLogic)?"\agon\ax":"\agoff\ax");
	WriteChatf("/makecamp [pause|unpause] (%s) MoveUtils", Buffer);
	sprintf(Buffer,(bMakeCampReturn)?"\agon\ax":"\agoff\ax");
	WriteChatf("/makecamp [return] (%s) immediatly to camp", Buffer);
	WriteChatColor("/makecamp [save|load] INI file");
}

void MakeCampCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if(stickOn)breakStick();
	if(bCircling) breakCircle();
	if(bMoveToOn) breakMoveTo();
	char currentArg[MAX_STRING];
	int argn=1;

	GetArg(currentArg,szLine,argn++);
	if( !*currentArg ) {
		if(!bMakeCamp) {
			bMakeCamp=true;
			anchorY=pChar->Y;
			anchorX=pChar->X;
			campRadius = (campRadius<10)?10:campRadius;
			leashLength = (leashLength>=campRadius)?leashLength:campRadius+10;
			mindelay = (mindelay < 125)?125:mindelay;
			maxdelay = (maxdelay <= mindelay)?mindelay+125:maxdelay;
		} else bMakeCamp = false;
	}


	while( *currentArg ) {

		if( !strncmp(currentArg,"on",3) ) {
			bMakeCamp=true;
			anchorY=pChar->Y;
			anchorX=pChar->X;
			campRadius = (campRadius<10)?10:campRadius;
			leashLength = (leashLength>=campRadius)?leashLength:campRadius+10;
			mindelay = (mindelay < 125)?125:mindelay;
			maxdelay = (maxdelay <= mindelay)?mindelay+125:maxdelay;
		} else if( !strncmp(currentArg,"off",4) ) {
			bMakeCamp = false;
			break;
		} else if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.') {
			bMakeCamp = true;
			anchorY=(float)atof(currentArg);
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.')
				anchorX=(float)atof(currentArg);
			else {
				bMakeCamp = false;
				MakeCampHelp();
				return;
		 }
		} else if( !strncmp(currentArg,"mindelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0])) 
				mindelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				MakeCampHelp();
				return;
			}
		} else if( !strncmp(currentArg,"maxdelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0])) 
				maxdelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				MakeCampHelp();
				return;
			}
		} else if( !strncmp(currentArg,"leash",6) ) {
			GetArg(currentArg,szLine,argn);
			if( isdigit(currentArg[0])) {
				leashLength=(float)atof(currentArg);
				argn++;
			} else {
				bLeash = bLeash ? false : true;
			}
		} else if( !strncmp(currentArg,"radius",7) ) {
			GetArg(currentArg,szLine,argn);
			if( isdigit(currentArg[0])) {
				campRadius=(float)atof(currentArg);
				argn++;
			} else {
				bMakeCamp = false;
				MakeCampHelp();
				return;
			}
		} else if( !strncmp(currentArg,"return",7) ) {
			bMakeCampReturn=true;
			break;
		} else if( !strncmp(currentArg,"save",5 ) ) {
			Save_INI();
		} else if( !strncmp(currentArg,"load",5) ) {
			Load_INI();
		} else if( !strncmp(currentArg,"pause",6) ) {
			ReleaseKeys();
			PauseLogic = true;
		} else if( !strncmp( currentArg,"unpause",8 ) ) {
			PauseLogic = false;
		} else if( !strncmp(currentArg,"mpause",7) ) {
			mPause = mPause?false:true;
		} else {
			MakeCampHelp();
			return;
		}
		GetArg(currentArg,szLine,argn++);
	}
}


void HandleMakeCamp() {
	PSPAWNINFO psTarget = (PSPAWNINFO)pTarget;
	PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
	PSPAWNINFO pLPlayer = (PSPAWNINFO) pLocalPlayer;
	SYSTEMTIME stCurr;
	GetSystemTime(&stCurr);
	int elapsedTime = millisDiff(stCurr,stReturnTime);
	float newHeading=0.0f;

	if (!pChSpawn || !pLPlayer || !GetCharInfo2() ) {
		WriteChatColor("Null pointer, breaking HandleMakeCamp");
		return;
	}
	if ( ( ((long) (pChSpawn->CastingData.SpellID)) >= 0 && !(IsBardClass()) ) || (pChSpawn->StandState != STANDSTATE_STAND && pChSpawn->StandState != STANDSTATE_DUCK)|| GetCharInfo()->Stunned) {
		if( !casting ) {
			casting = true;
			ReleaseKeys();
		}
	} else if( casting ) {
		casting = false;
	}

	if( (!PauseLogic && !casting) ) {
		if( (GetDistance(pChSpawn->X,pChSpawn->Y,anchorX,anchorY) > campRadius && !bMoveToOn && !pTarget && elapsedTime >= returnTime && returnTime != 0) || bMakeCampReturn ) {
			returnTime=0;
			LocY = (int)rand()%(int)(campRadius*2) + (anchorY-campRadius);
			LocX = (int)rand()%(int)(campRadius*2) + (anchorX-campRadius); 
			bMoveToOn = true;
			bMakeCampReturn=false;
		} else if(GetDistance(pChSpawn->X,pChSpawn->Y,anchorX,anchorY) > campRadius && !bMoveToOn && !pTarget && returnTime == 0) {
			GetSystemTime(&stReturnTime);
			returnTime = (int)rand()%(maxdelay-mindelay+1) + mindelay;
		}

		if( GetDistance(pChSpawn->X,pChSpawn->Y,anchorX,anchorY) > leashLength && bLeash && (stickOn || bCircling)) {
			if(stickOn){
				DoFwd(false);
				ReleaseKeys();
				stickOn=false;
				stickWasOn=true;
			}
			if(bCircling) {
				DoFwd(false);
				ReleaseKeys();
				bCircling=false;
				bCirclingWasOn=true;
			}
		} else if(bLeash && psTarget && ( stickWasOn || bCirclingWasOn) ) {
			if(GetDistance(psTarget->X,psTarget->Y,anchorX,anchorY) > leashLength && bLeash) {
				newHeading = (float) (atan2(psTarget->X - pChSpawn->X, psTarget->Y - pChSpawn->Y) * 256.0 / (float)PI);
				if( newHeading >= 512.0f) newHeading -= 512.0f;
				if( newHeading < 0.0f ) newHeading += 512.0f;
				pChSpawn->Heading = newHeading;
			} else if(stickWasOn) {
				DoFwd(true);
				stickOn=true;
				stickWasOn=false;
			} else if(bCirclingWasOn) {
				DoFwd(true);
				bCircling=true;
				bCirclingWasOn=false;
			}
		}
	}
}

//Stick Related Functionality
class MQ2StickType *pStickType = 0;

class MQ2StickType : public MQ2Type
{
public:
	enum StickMembers {
		Status=1,
		Active=2,
		Distance=3,
		MoveBehind=4,
		MovePause=5,
		MoveBack=6,
		Loose=7,
		Paused=8,
		Behind=9,
		Stopped=10,
		Pin=11,
		StickTarget=12,
	};

	MQ2StickType():MQ2Type("stick")
	{
		TypeMember(Status);
		TypeMember(Active);
		TypeMember(Distance);
		TypeMember(MoveBehind);
		TypeMember(MovePause);
		TypeMember(MoveBack);
		TypeMember(Loose);
		TypeMember(Paused);
		TypeMember(Behind);
		TypeMember(Stopped);
		TypeMember(Pin);
		TypeMember(StickTarget);
	}
	~MQ2StickType()
	{
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
	{
		PMQ2TYPEMEMBER pMember=MQ2StickType::FindMember(Member);
		if (!pMember)
			return false;
		switch((StickMembers)pMember->ID)
		{
		case Status:
			strcpy(DataTypeTemp,"OFF");
			if( stickOn ) {
				strcpy(DataTypeTemp,"ON");
		 }
			if( PauseLogic ) {
				strcpy(DataTypeTemp,"PAUSED");
		 }
			Dest.Ptr=DataTypeTemp;
			Dest.Type=pStringType;
			return true;
		case Active:
			Dest.DWord=stickOn;
			Dest.Type=pBoolType;
			return true;
		case Distance:
			Dest.Float=stickDist;
			Dest.Type=pFloatType;
			return true;
		case MoveBehind:
			Dest.DWord=moveBehind;
			Dest.Type=pBoolType;
			return true;
		case MovePause:
			Dest.DWord=mPause;
			Dest.Type=pBoolType;
			return true;
		case MoveBack:
			Dest.DWord=moveBack;
			Dest.Type=pBoolType;
			return true;
		case Loose:
			Dest.DWord=looseStick;
			Dest.Type=pBoolType;
			return true;
		case Paused:
			Dest.DWord=PauseLogic;
			Dest.Type=pBoolType;
			return true;
		case Behind:
			if (ppTarget && pTarget) {
				PSPAWNINFO psTarget = (PSPAWNINFO)pTarget;
				PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
				Dest.DWord=(GetDistance(pChSpawn,psTarget) > stickDist || fabs(angularDistance(psTarget->Heading,pChSpawn->Heading)) > 45.0 )?false:true;
			} else
				Dest.DWord=false;
			Dest.Type=pBoolType;
			return true;
		case Stopped:
			if( ppTarget && pTarget ) {
				PSPAWNINFO psTarget = stickHold?stickTarget:(PSPAWNINFO)pTarget;
				PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
				Dest.DWord=(GetDistance(pChSpawn,psTarget)<=stickDist)?true:false;
			} else
				Dest.DWord=false;
			Dest.Type=pBoolType;
			return true;
		case Pin:
			Dest.DWord=movePin;
			Dest.Type=pBoolType;
			return true;
		case StickTarget:
			if( (stickHold && (!stickTarget || !(stickTarget->SpawnID) || stickTarget_Type!=GetSpawnType(stickTarget))) || (!stickHold && ((!ppTarget) || (!pTarget))) ) {
				Dest.Int=0;
			} else Dest.Int=stickHold?stickTarget->SpawnID:((PSPAWNINFO)pTarget)->SpawnID;
			Dest.Type=pIntType;
	                return true;
		}
		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination)
	{
		strcpy(Destination,"OFF");
		if( stickOn ) {
			strcpy(Destination,"ON");
		}
		if( PauseLogic ) {
			strcpy(Destination,"PAUSED");
		}
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source)
	{
		return false;
	}
	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source)
	{
		return false;
	}
};

BOOL dataStick(PCHAR szName, MQ2TYPEVAR &Ret)
{
	Ret.DWord=1;
	Ret.Type=pStickType;
	return true;
}

void StickHelp()
{
	char Buffer[64];

	WriteChatColor(szVersion,CONCOLOR_YELLOW);
	//   WriteChatColor("Usage: /stick [on|hold|off|pause|unpause|reload|id <spawnid>] [<dist>] [behind|behindonce|pin] [mpause] [moveback] [loose] [-<dist>] [<perc>%] [uw]");
	sprintf(Buffer,(stickOn)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [on|off] (%s)",Buffer);
	sprintf(Buffer,(stickHold)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [hold] (%s) current target even if you lose/change target",Buffer);
	WriteChatf("/stick [<dist>] (\ag%.2f\ax) units of your target", stickDist);
	WriteChatf("/stick [[-]<dist>[%%]] (\ag%.2f\ax) - Substract or percentage modifier",stickDistMod);
	sprintf(Buffer,(moveBehind)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [behind] (%s) your target",Buffer);
	sprintf(Buffer,(moveBehindOnce)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [behindonce] (%s) your target once",Buffer);
	sprintf(Buffer,(movePin)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [pin] (%s) your target",Buffer);
	sprintf(Buffer,(moveNotFront)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [!front] (%s) not in front of your target",Buffer);
	sprintf(Buffer,(moveBack)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [moveback] (%s) to <dist> from target",Buffer);
	sprintf(Buffer,(looseStick)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [loose] (%s) to target",Buffer);
	sprintf(Buffer,(underwater)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [uw] (%s) look up/down at target",Buffer);
	WriteChatf("/stick [id <spawn>] (\ag%i\ax) to target with ID", stickTarget);
	sprintf(Buffer,(mPause)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [mpause] (%s) manual control pause toggle", Buffer);
	sprintf(Buffer,(PauseLogic)?"\agon\ax":"\agoff\ax");
	WriteChatf("/stick [pause|unpause] (%s) MoveUtils", Buffer);
	WriteChatColor("/stick [save|load] INI file");
	WriteChatf("/stick [mindelay <time in ms>] (\ag%d\ax) return to camp or resume after manual move", mindelay);
	WriteChatf("/stick [maxdelay <time in ms>] (\ag%d\ax) return to camp or resume after manual move", maxdelay);
}


void StickCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	char currentArg[MAX_STRING];
	int argn=1;
	int trand = rand()%100;
	turnDirection *= (trand>50)?1.0f:-1.0f;

	GetArg(currentArg,szLine,argn++);
	if( !strncmp(currentArg,"debug",6) ) {
		bDebug=true;
		return;
	}

	//no arguements
	if(bCircling) breakCircle();
	if(bMoveToOn) breakMoveTo();
	if( !*currentArg ) {
		if( pTarget ) {
			breakStick();
			stickOn=true;
			currentDist=GetDistance(pChar,(PSPAWNINFO) pTarget);
			StandUp();
		} else {
			WriteChatColor("You must specify something to stick to!");
		}
	}


	while( *currentArg ) {
		if( !strncmp(currentArg,"on",3) ) {
			if( pTarget ) {
				breakStick();
				stickOn=true;
				currentDist=GetDistance(pChar,(PSPAWNINFO) pTarget);
				StandUp();
			} else {
				WriteChatColor("You must specify something to stick to!");
			}
		} else if( strstr(currentArg,"%") ) {
			stickDistModP = (float)atof(currentArg) / 100.0f;
			if( setDist ) stickDist *= stickDistModP;
			stickOn = true;
		} else if( isdigit(currentArg[0]) || currentArg[0]=='.' ) {
			setDist = true;
			stickDist = (float)atof(currentArg) * stickDistModP + stickDistMod;
			stickOn = true;
		} else if( currentArg[0]=='-' ) {
			stickDistMod = (float)atof(currentArg);
			if( setDist ) stickDist += stickDistMod;
			stickOn = true;
		} else if( !strncmp(currentArg,"pause",6) ) {
			ReleaseKeys();
			PauseLogic = true;
		} else if( !strncmp( currentArg,"unpause",8 ) ) {
			PauseLogic = false;
		} else if( !strncmp(currentArg,"mpause",7) ) {
			mPause = mPause?false:true;
		} else if( !strncmp(currentArg,"moveback",9) ) {
			moveBack = true;
			stickOn = true;
		} else if( !strncmp(currentArg,"loose",6) ) {
			looseStick = true;
			stickOn = true;
		} else if( !strncmp(currentArg,"uw",3 ) ) {
			underwater=true;
			stickOn = true;
		} else if( !strncmp(currentArg,"save",5 ) ) {
			Save_INI();
		} else if( !strncmp(currentArg,"load",5) ) {
			Load_INI();
		} else if( !strncmp(currentArg,"off",4) ) {
			breakStick();
			break;
		} else if( !strncmp(currentArg,"hold",5) ) {
			if( ppTarget && pTarget ) {
				if (((PSPAWNINFO) pTarget)->SpawnID == ((PSPAWNINFO) pLocalPlayer)->SpawnID) {
					WriteChatColor("You cannot stick hold to yourself.");
					breakStick(true, true);
					return;
				}
//				breakStick();  Commented out because if you do something like /stick !front hold the !front parameter is reset due to this breakstick
				stickOn = true;
				stickHold = true;
				stickTarget = (PSPAWNINFO)pTarget;
				stickTarget_Type = stickTarget ? GetSpawnType(stickTarget) : NONE;
				currentDist=GetDistance(pChar,(PSPAWNINFO) stickTarget);
				StandUp();
			//				stickHold = stickHold && stickTarget && stickTarget->SpawnID;
			}
		} else if( !strncmp(currentArg,"id",3) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0]) ) {
//				breakStick();  Commented out because if you do something like /stick !front hold the !front parameter is reset due to this breakstick
				stickTarget = (PSPAWNINFO) GetSpawnByID(atoi(currentArg));
				if( stickTarget ) {
					if (((PSPAWNINFO) stickTarget)->SpawnID == ((PSPAWNINFO) pLocalPlayer)->SpawnID) {
						WriteChatColor("You cannot stick id to yourself.");
						breakStick(true, true);
						return;
					}
					stickOn = true;
					stickHold = true;
					stickTarget_Type = stickTarget ? GetSpawnType(stickTarget) : NONE;
					currentDist=GetDistance((PSPAWNINFO) pLocalPlayer,stickTarget);
					StandUp();
				}
			} else {
				WriteChatColor("When using ID the next parameter MUST be the spawn ID.");
				breakStick(true, true);
				return;
			}
		} else if( !strncmp(currentArg,"behind",7) ) {
			stickOn = true;
			moveBehind=true;
			moveBehindOnce=false;
			movePin=false;
			StandUp();
		} else if( !strncmp(currentArg,"behindonce",11) ) {
			stickOn = true;
			moveBehindOnce=true;
			moveBehind=false;
			movePin=false;
			StandUp();
		} else if( !strncmp(currentArg,"pin",3) ) {
			stickOn = true;
			movePin=true;
			moveBehind=false;
			moveBehindOnce=false;
			StandUp();
		} else if( !strncmp(currentArg,"!front",6) ) {
			stickOn = true;
			moveNotFront=true;
			moveBehind=false;
			moveBehindOnce=false;
			StandUp();
		} else if( !strncmp(currentArg,"mindelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0])) 
				mindelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				StickHelp();
				return;
			}
		} else if( !strncmp(currentArg,"maxdelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0])) 
				maxdelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				StickHelp();
				return;
			}
		} else if( !strncmp(currentArg,"help",5) ) {
			StickHelp();
		} else {
			breakStick();
			StickHelp();
			break;
		}
		GetArg(currentArg,szLine,argn++);
	}

	//Reno - call to make chat text work properly
	stickText();
}

void HandleStick() {
	if( (stickHold && (!stickTarget || !(stickTarget->SpawnID) || stickTarget_Type!=GetSpawnType(stickTarget))) || (!stickHold && ((!ppTarget) || (!pTarget))) ) {
		breakStick();
		return;
	}

	PSPAWNINFO psTarget = stickHold?stickTarget:(PSPAWNINFO)pTarget;
	PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
	PSPAWNINFO pLPlayer = (PSPAWNINFO) pLocalPlayer;
	if (!pChSpawn || !psTarget || !pLPlayer || !GetCharInfo2() ) {
		WriteChatColor("Null pointer, breaking stick");
		breakStick();
		return;
	}

	float newHeading=0.0f;
	float prevDist = currentDist;
	bool mounted;

	mounted = pChSpawn->Mount ? true : false;

	if( !setDist )
		stickDist = (psTarget->StandState?get_melee_range(pLocalPlayer,(EQPlayer *)psTarget):15.0f) * stickDistModP + stickDistMod;

	currentDist=GetDistance(pChSpawn,psTarget);
	if( breakOnWarpEnabled && (currentDist-prevDist) > breakDist ) {
		breakStick();
		return;
	}
	if ( ( ((long) (pChSpawn->CastingData.SpellID)) >= 0 && !(IsBardClass()) ) || (pChSpawn->StandState != STANDSTATE_STAND && pChSpawn->StandState != STANDSTATE_DUCK)|| GetCharInfo()->Stunned || psTarget->SpawnID == pChSpawn->SpawnID ) {
		if( !casting ) {
			casting = true;
			ReleaseKeys();
		}
	} else if( casting ) {
		casting = false;
	}

	if(bMakeCamp &&  bLeash) {
		if(GetDistance(pChSpawn->X,pChSpawn->Y,anchorX,anchorY) < GetDistance(psTarget->X, psTarget->Y, anchorX,anchorY) && 
			GetDistance(psTarget->X, psTarget->Y, anchorX,anchorY) > leashLength) {
			breakStick();
			if (stickVerbosity)
				WriteChatColor("MakeCamp breaking Stick because target outside of leash length.", CONCOLOR_YELLOW);
			DoFwd(false);
			return;
		}
	}

	if( (!PauseLogic && !casting) || !autoPauseEnabled ) {
		float pulseMoved=GetDistance(pChSpawn->X,pChSpawn->Y,prevX,prevY);
		if( pulseMoved < 5) pulseAvg = (pulseAvg + pulseMoved)/2;
		prevX=pChSpawn->X;
		prevY=pChSpawn->Y;
		prevZ=pChSpawn->Z;

		float SpeedMultiplier = *((float*) &(((PSPAWNINFO) pLocalPlayer)->SpeedMultiplier));
		if( stickhasmovedfwd && ((pulseAvg < (stuckDist + SpeedMultiplier) && (!pChSpawn->UnderWater || !mounted) ) ||
			(pulseAvg == 0 && pChSpawn->UnderWater) || (pulseAvg < (stuckDist + SpeedMultiplier)/3 && mounted )) &&
			((currentDist - stickDist) > 10.0) && stuckLogic && !GetCharInfo()->Stunned ) {
				stuck++;
				if(fmod((float)stuck,(float)stuckCheck) == 0.0f) {
					newHeading =(float) ( pChSpawn->Heading + turnDirection);
					if( newHeading >= 512.0f) newHeading -= 512.0f;
					if( newHeading < 0.0f ) newHeading += 512.0f;
					pChSpawn->Heading = newHeading;

					//check to see if we are heading directly away from our target if so then go back
					newHeading = (float) (atan2(psTarget->X - pChSpawn->X, psTarget->Y - pChSpawn->Y) * 256.0 / (float)PI);
					if( newHeading >= 512.0f) newHeading -= 512.0f;
					if( newHeading < 0.0f ) newHeading += 512.0f;

               newHeading += 256.0f;
					if( newHeading >= 512.0f) newHeading -= 512.0f;
					if( newHeading < 0.0f ) newHeading += 512.0f;
					if( pChSpawn->Heading > (newHeading - fabs(turnDirection/2)) &&  pChSpawn->Heading <	(newHeading + fabs(turnDirection/2)) ) {
                  newHeading -= 256.0f;
						if( newHeading >= 512.0f) newHeading -= 512.0f;
						if( newHeading < 0.0f ) newHeading += 512.0f;
						pChSpawn->Heading = newHeading;
						stuck = stuckCheck;
						turnDirection *= -1.0f;
			  }
				}
				stuckFree=0;
		} else if( stuck > 0 ) {
			if(fmod((float)stuck,(float)stuckCheck) == 0.0f) {
				newHeading =(float) ( pChSpawn->Heading - turnDirection );
				pChSpawn->Heading = newHeading;
			}
			stuck--;
			if(stuckFree++ > stuckCheck*3 ) {
				stuck=0;
			}
		} else {
			newHeading = (float) (atan2(psTarget->X - pChSpawn->X, psTarget->Y - pChSpawn->Y) * 256.0 / (float)PI);
			if( newHeading >= 512.0f) newHeading -= 512.0f;
			if( newHeading < 0.0f ) newHeading += 512.0f;
			if (stickhasmovedfwd && fabs(pChSpawn->Heading - newHeading) >= 200 && currentDist < 10) {
				DoBck(true);
				return;
			} else {
				DoBck(false);
			}
			if( looseStick ) {
				gFaceAngle = newHeading;
			} else {
				pChSpawn->Heading = newHeading;
			}

			//underwater = pChSpawn->UnderWater==5;
			if( underwater ) {
				double lookAngle = (float) atan2(psTarget->Z + psTarget->AvatarHeight*StateHeightMultiplier(psTarget->StandState) -
					pChSpawn->Z - pChSpawn->AvatarHeight*StateHeightMultiplier(pChSpawn->StandState),
					currentDist) * 256.0f / (float)PI;
				if ( looseStick ) {
					gLookAngle = lookAngle;
				} else {
					pChSpawn->CameraAngle = (FLOAT)lookAngle;
			 }
			}
			if ( currentDist > (stickDist * 3) ) {
				// too far away, dont do pin or behind
		 } else if( (moveBehind || moveBehindOnce) && pChSpawn->SpawnID!=pChSpawn->TargetOfTarget ) {
				float angDist = angularDistance(psTarget->Heading,pChSpawn->Heading);
				if( fabs(angDist) > 45.0 ) {
					if(angDist < 0.0) {
						// strafe left
						DoLft(true);
					} else {
						// strage right
						DoRgt(true);
					}
				} else {
					moveBehindOnce = false;
					DoLft(false);
					DoRgt(false);
			 }
		 } else if (movePin && pChSpawn->SpawnID!=pChSpawn->TargetOfTarget ) {
				FLOAT angDist = angularDistance(psTarget->Heading,pChSpawn->Heading);
				if((angDist > 0 && angDist <= 112) || angDist < -144) {
					DoLft(true);
				} else if ((angDist < 0 && angDist > -112) || angDist > 144) {
					DoRgt(true);
				} else {
					DoLft(false);
					DoRgt(false);
			 }
         } else if (moveNotFront && pChSpawn->SpawnID!=pChSpawn->TargetOfTarget ) {
				FLOAT angDist = angularDistance(psTarget->Heading,pChSpawn->Heading);
				if( fabs(angDist) > 135.0 ) {
					int trand = rand()%100;
					if (trand>85) {
						moveBehindOnce = true;
					} else if(angDist < 0.0) {
						//strafe left
						DoLft(true);
					} else {
						//strage right
						DoRgt(true);
					}
				} else {
					//				moveBehindOnce = false;
					DoLft(false);
					DoRgt(false);
			 }
			}
		}
		if(pChSpawn->StandState == STANDSTATE_DUCK && stuckLogic ) pChSpawn->StandState = STANDSTATE_STAND;
		if( currentDist > stickDist) {
			// if distance is less than 10 and target not fleeing, walk
			bool WalkMode=(fabs(angularDistance(pChSpawn->Heading,psTarget->Heading))>64 && (currentDist-stickDist)<=10);
			DoFwd(true,WalkMode);
		} 
		else if( moveBack && currentDist < (stickDist-5.0) ) {
			DoBck(true);
		} else {
			DoFwd(false);
			DoBck(false);
		}
	}
}
void stickText()
{
	char szTemp[MAX_STRING];

	if( stickVerbosity == 1 ) {
		if( PauseLogic ) {
			WriteChatColor("Stick paused.");
		} else if( stickOn ) {
			if( stickHold && stickTarget ) {
				sprintf(szTemp,"You are now sticking to %s.",stickTarget->DisplayedName);
			} else if( ppTarget && pTarget ) {
				sprintf(szTemp,"You are now sticking to %s.",((PSPAWNINFO)pTarget)->DisplayedName);
			} else {
				sprintf(szTemp,"Need a target for stick.");
		 }
			WriteChatColor(szTemp);
		} else{
			WriteChatColor("You are no longer sticking to anything.");
		}
	}
}

void breakStick(bool stopMoving, bool quite) {
	if(!bMakeCamp) {
		PauseLogic=false;
		mPause=false;
		bMoveToOn=false;
		bCircling=false;
	}

	pulseAvg=10.0f;;
	stickOn=false;
	stickWasOn=false;
	stickHold=false;
	setDist=false;
	moveBehind=false;
	prevMoveBehind=false;
	moveBehindOnce=false;
	movePin=false;
	moveNotFront=false;
	prevMovePin=false;
	moveBack=false;
	looseStick=false;
	underwater=false;
	stickTarget=NULL;
	stickDistMod=0.0f;
	stickDistModP=1.0f;
	stuck=0;
	stickhasmovedfwd=false;
	stickTarget_Type=NONE;
	if( stopMoving )
		ReleaseKeys();
	else {
		DoWalk(false);
		DoLft(false);
		DoRgt(false);
	}

	//Reno - To get proper break stick message
	if (!quite)
		stickText();
}
//MoveTo Related Functionality
class MQ2MoveToType *pMoveToType = 0;

class MQ2MoveToType : public MQ2Type
{
public:
	enum MoveToMembers {
		Moving=1,
		Stopped=2
	};

	MQ2MoveToType():MQ2Type("moveto")
	{
		TypeMember(Moving);
		TypeMember(Stopped);
	}

	~MQ2MoveToType()
	{
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
	{
		PMQ2TYPEMEMBER pMember=MQ2MoveToType::FindMember(Member);
		if (!pMember)
			return false;
		switch((MoveToMembers)pMember->ID)
		{
		case Moving:
			Dest.DWord=bMoveToOn;
			Dest.Type=pBoolType;
			return true;
		case Stopped:
			PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
			Dest.DWord=(GetDistance(pChSpawn->X,pChSpawn->Y,(float)LocX,(float)LocY)<=moveDist)?true:false;
			Dest.Type=pBoolType;
			return true;
		}
		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination)
	{
		if( bMoveToOn ) {
			strcpy(Destination,"ON");
		} else {
			strcpy(Destination,"OFF");
		}
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source)
	{
		return false;
	}
	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source)
	{
		return false;
	}
};

BOOL dataMoveTo(PCHAR szName, MQ2TYPEVAR &Ret)
{
	Ret.DWord=1;
	Ret.Type=pMoveToType;
	return true;
}

void MoveToHelp()
{
	char Buffer[64];

	WriteChatColor(szVersion,CONCOLOR_YELLOW);
	sprintf(Buffer,(bMoveToOn)?"\agon\ax":"\agoff\ax");
	WriteChatf("/moveto [loc Y X|off] (%s \ag%.2f\ax \ag%.2f\ax)",Buffer,LocY, LocX);
   sprintf(Buffer,(looseMoveto)?"\agon\ax":"\agoff\ax");
   WriteChatf("/moveto [loose] (%s)",Buffer);
	WriteChatf("/moveto [<dist>] (\ag%.2f\ax) area your anchor", moveDist);
	WriteChatf("/moveto [[-]<dist>] (\ag%.2f\ax) - Substract modifier",moveDistMod);
	sprintf(Buffer,(mPause)?"\agon\ax":"\agoff\ax");
	WriteChatf("/moveto [mpause] (%s) manual control pause toggle", Buffer);
	sprintf(Buffer,(PauseLogic)?"\agon\ax":"\agoff\ax");
	WriteChatf("/moveto [pause|unpause] (%s) MoveUtils", Buffer);
	WriteChatColor("/moveto ID [Spawn ID | If not numeric value then move to current target]");
	WriteChatf("/moveto [mindelay <time in ms>] (\ag%d\ax) return to camp or resume after manual move", mindelay);
	WriteChatf("/moveto [maxdelay <time in ms>] (\ag%d\ax) return to camp or resume after manual move", maxdelay);
	WriteChatColor("/moveto [save|load] INI file");
}

void MoveToCommand(PSPAWNINFO pChar, PCHAR szLine) {
	PSPAWNINFO movetoTarget;
	breakStick();
	breakCircle();
	char currentArg[MAX_STRING];
	int argn=1;
	PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
	GetArg(currentArg,szLine,argn++);
 
	while( *currentArg ) {
		if (!stricmp(currentArg,"help")) {
			MoveToHelp();
		} else if (!stricmp(currentArg,"loc")) {
			GetArg(currentArg,szLine,argn);
			if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.' ) {
				LocY = (float) atof(currentArg);
				argn++;
				GetArg(currentArg,szLine,argn);
				if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.') {
					LocX = (float) atof(currentArg);
					argn++;
					bMoveToOn=true;
					StandUp();
				} else {
					MoveToHelp();
					breakMoveTo();
					return;
				}
			} else {
				MoveToHelp();
				breakMoveTo();
				return;
			}
		} else if( isdigit(currentArg[0]) || currentArg[0]=='.') {
			moveDist = (float)atof(currentArg);
		} else if( currentArg[0]=='-' ) {
			moveDistMod = (float)atof(currentArg);
			moveDist += moveDistMod;
		} else if( !stricmp(currentArg,"pause") ) {
			ReleaseKeys();
			PauseLogic = true;
		} else if( !stricmp( currentArg,"unpause" ) ) {
			PauseLogic = false;
	  } else if( !strncmp( currentArg,"loose",6 ) ) {
		  looseMoveto=true;
		} else if( !stricmp(currentArg,"mpause") ) {
			mPause = mPause?false:true;
		} else if( !strncmp(currentArg,"save",5 ) ) {
			Save_INI();
		} else if( !strncmp(currentArg,"load",5) ) {
			Load_INI();
		} else if (!stricmp(currentArg,"id")) {
			GetArg(currentArg,szLine,argn);
			if( isdigit(currentArg[0]) ) {
				argn++;
				movetoTarget = (PSPAWNINFO) GetSpawnByID(atoi(currentArg));
				if( movetoTarget ) {
					if (((PSPAWNINFO) movetoTarget)->SpawnID == ((PSPAWNINFO) pLocalPlayer)->SpawnID) {
						WriteChatColor("You tried to move to yourself!!");
						breakMoveTo();
						return;
					}
					LocY = movetoTarget->Y;
					LocX = movetoTarget->X;
					bMoveToOn=true;
					StandUp();
				}
			} else if(pTarget) {
				movetoTarget = (PSPAWNINFO)pTarget;
				if( movetoTarget ) {
					if (movetoTarget->SpawnID == ((PSPAWNINFO) pLocalPlayer)->SpawnID) {
						WriteChatColor("You tried to move to yourself!!");
						breakMoveTo();
						return;
					}
					LocY = movetoTarget->Y;
					LocX = movetoTarget->X;
					bMoveToOn=true;
					StandUp();
				}
			} else {
				WriteChatColor("You specified to move to a location by ID but did not specify an ID and you don't have a target, You must specify a spawn ID or have a target.");                           
			}
		} else if (!stricmp(currentArg,"off")) {
			breakMoveTo();
			return;
		} else if( !strncmp(currentArg,"mindelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0])) 
				mindelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				MoveToHelp();
				return;
			}
		} else if( !strncmp(currentArg,"maxdelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0])) 
				maxdelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				MoveToHelp();
				return;
			}
		} else {
			MoveToHelp();
			breakMoveTo();
			return;
		}
		GetArg(currentArg,szLine,argn++);
	} 

	sprintf(szMsg, "Moving to loc %g %g", LocY, LocX);
	if (stickVerbosity && bMoveToOn) WriteChatColor(szMsg, CONCOLOR_YELLOW);
	else if (stickVerbosity) WriteChatColor("MoveTo off.", CONCOLOR_YELLOW);
}

void HandleMoveTo()
{
	PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
	PSPAWNINFO pLPlayer = (PSPAWNINFO) pLocalPlayer;
	if (!pChSpawn || !pLPlayer || !GetCharInfo2() ) {
		WriteChatColor("Null pointer, breaking MoveTo");
		breakMoveTo();
		return;
	}

	if ( ( ((long) (pChSpawn->CastingData.SpellID)) >= 0 && !(IsBardClass()) ) || (pChSpawn->StandState != STANDSTATE_STAND && pChSpawn->StandState != STANDSTATE_DUCK)|| GetCharInfo()->Stunned ) {
		if( !casting ) {
			casting = true;
			DoFwd(false);
			ReleaseKeys();
		}
	} else if( casting ) {
		casting = false;
	}

	if(bMakeCamp &&  bLeash) {
		if(GetDistance(pChSpawn->X,pChSpawn->Y,anchorX,anchorY) < GetDistance(LocX, LocY, anchorX,anchorY) && 
			GetDistance(LocX, LocY, anchorX,anchorY) > leashLength) {
			bMoveToOn=false;
			if (stickVerbosity)
				WriteChatColor("MakeCamp breaking MoveTo because location outside of leash length.", CONCOLOR_YELLOW);
			DoFwd(false);
			return;
		}
	}

	if( !casting || !autoPauseEnabled ) {
		float newHeading;
		float pulseMoved=GetDistance(pChSpawn->X,pChSpawn->Y,prevX,prevY);
		if( pulseMoved < 5) pulseAvg = (pulseAvg + pulseMoved)/2;
		prevX=pChSpawn->X;
		prevY=pChSpawn->Y;
		prevZ=pChSpawn->Z;

		float SpeedMultiplier = *((float*) &(((PSPAWNINFO) pLocalPlayer)->SpeedMultiplier));

		if( stickhasmovedfwd && ((pulseAvg < (stuckDist + SpeedMultiplier) && !pChSpawn->UnderWater) ||
         (pulseAvg == 0 && pChSpawn->UnderWater))&& GetDistance(pChSpawn->X,pChSpawn->Y,(float)LocX,(float)LocY) > moveDist && stuckLogic && !GetCharInfo()->Stunned ) {
				stuck++;
				if(fmod((float)stuck,(float)stuckCheck) == 0.0f) {
					newHeading =(float) ( pChSpawn->Heading + turnDirection);
					if( newHeading >= 512.0f) newHeading -= 512.0f;
					if( newHeading < 0.0f ) newHeading += 512.0f;
			   looseMoveto?gFaceAngle=newHeading:pChSpawn->Heading=newHeading;

					//check to see if we are heading directly away from our target if so then go back
					newHeading = (float) (atan2(LocX - pChSpawn->X, LocY - pChSpawn->Y) * 256.0 / (float)PI);
					if( newHeading >= 512.0f) newHeading -= 512.0f;
					if( newHeading < 0.0f ) newHeading += 512.0f;

               newHeading += 256.0f;
					if( newHeading >= 512.0f) newHeading -= 512.0f;
					if( newHeading < 0.0f ) newHeading += 512.0f;
					if( pChSpawn->Heading > (newHeading - fabs(turnDirection/2)) &&  pChSpawn->Heading <	(newHeading + fabs(turnDirection/2)) ) {
                  newHeading -= 256.0f;
						if( newHeading >= 512.0f) newHeading -= 512.0f;
						if( newHeading < 0.0f ) newHeading += 512.0f;
	   			  looseMoveto?gFaceAngle=newHeading:pChSpawn->Heading=newHeading;

						stuck = stuckCheck;
						turnDirection *= -1.0f;
			  }
				}
				stuckFree=0;
		} else if( stuck > 0 ) {
			if(fmod((float)stuck,(float)stuckCheck) == 0.0f) {
				newHeading =(float) ( pChSpawn->Heading - turnDirection );
			looseMoveto?gFaceAngle=newHeading:pChSpawn->Heading=newHeading;
			}
			stuck--;
			if(stuckFree++ > stuckCheck*3 ) {
				stuck=0;
			}
		} else {
			newHeading = (float) (atan2(LocX - pChSpawn->X, LocY - pChSpawn->Y) * 256.0 / (float)PI);
			if( newHeading >= 512.0f)
				newHeading -= 512.0f;
			if( newHeading < 0.0f )
				newHeading += 512.0f;
		 
			looseMoveto?gFaceAngle=newHeading:pChSpawn->Heading=newHeading;

		}
		if( GetDistance(pChSpawn->X,pChSpawn->Y,(float)LocX,(float)LocY) > moveDist) {
	
        if( ((fabs(pChSpawn->Heading - newHeading)/2.0f > GetDistance(pChSpawn->X,pChSpawn->Y,(float)LocX,(float)LocY)) || fabs(pChSpawn->Heading - newHeading) > 128.0f) && looseMoveto ) {
			DoFwd(false);
		} else {

			DoFwd(true);
    	}

		} else {
			bMoveToOn=false;
         // FOLLOWPATH 
         //if (stickVerbosity) 
         if ((stickVerbosity) && ((!bMoveToTrader) || fpdebug)) 
         // END FOLLOWPATH 
				WriteChatColor("Arrived at MoveTo location", CONCOLOR_YELLOW);
			DoFwd(false);
		}
	}
}

void breakMoveTo(bool stopMoving, bool quite) {
   //FOLLOWPATH 
   bMoveToTrader=false; 
   //END FOLLOWPATH 
	bMoveToOn=false;
	bCircling=false;
   looseMoveto=false;
	stuck=0;
	if( stopMoving )
		ReleaseKeys();
	else {
		DoWalk(false);
		DoLft(false);
		DoRgt(false);
	}
}

//Circle Related Functionality
void CircleHelp()
{
	char Buffer[64];

	// holy spam
	WriteChatColor(szVersion,CONCOLOR_YELLOW);
	WriteChatColor("Usage: /circle on|off|drunken <radius> [<y> <x>] [cw|ccw|clockwise|counterclockwise|reverse] [fowards|backwards]",USERCOLOR_DEFAULT);
	sprintf(Buffer,(bCircling)?"\agon\ax":"\agoff\ax");
	WriteChatf("/circle [on|off] (%s) set anchor use current radius.", Buffer);
	WriteChatf("/circle [on <radius>] (\ag%.2f\ax) set radius at current||existing loc", CircleRadius);
	WriteChatColor("  If you call '/circle on <radius>' while not circling, it will start with your current loc and specified radius.",USERCOLOR_DEFAULT); 
	WriteChatColor("  If you call '/circle on <radius>' while already circling, it will update with your new loc and radius.",USERCOLOR_DEFAULT); 
	WriteChatColor("  If you call '/circle on' while already circling, it will update with your new loc using original radius.",USERCOLOR_DEFAULT); 
	WriteChatColor("  If you call '/circle on' while radius is zero, it will not activate.",USERCOLOR_DEFAULT); 
	WriteChatf("/circle [<y> <x>] (\ag%.2f\ax \ag%.2f\ax) anchor location",LocY, LocX);
	WriteChatColor("  Y and X are optional, if not specified will use your currect loc.",USERCOLOR_DEFAULT);
	WriteChatColor("  Y and X are in the same order that /location prints them.",USERCOLOR_DEFAULT);
	sprintf(Buffer,(PauseLogic)?"\agon\ax":"\agoff\ax");
	WriteChatf("/circle [pause|unpause]	(%s) pause toggle", Buffer);
	sprintf(Buffer,(bDrunken)?"\agon\ax":"\agoff\ax");
	WriteChatf("/circle [drunken] (%s) toggle random turns", Buffer);
	WriteChatf("/circle [mindelay <time in ms>] (\ag%d\ax) return to camp or resume after manual move", mindelay);
	WriteChatf("/circle [maxdelay <time in ms>] (\ag%d\ax) return to camp or resume after manual move", maxdelay);
	WriteChatColor("/circle [cw|clockwise] - Makes you circle clockwise, (default)",USERCOLOR_DEFAULT); 
	WriteChatColor("/circle [ccw|counterclockwise|reverse] - Makes you circle counter-clockwise, default is clockwise circling",USERCOLOR_DEFAULT); 
	WriteChatColor("/circle [forwards] - Circle running forwards, (default)",USERCOLOR_DEFAULT); 
	WriteChatColor("/circle [backwards] - Circle running backwards, default is to run forwards",USERCOLOR_DEFAULT); 
	WriteChatColor("/circle [save|load] INI file");
}

void CircleCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	if(stickOn)breakStick();
	if(bMoveToOn)breakMoveTo();

	bRunNextCommand = TRUE;
	CHAR currentArg[MAX_STRING];
	CHAR szMsg[MAX_STRING]={0};
	int argn=1;

	GetArg(currentArg,szLine,argn++);
	if (!stricmp(currentArg,"help") || szLine[0]==0) {
		CircleHelp();
		return;
	}

	bool circleOn=bCircling;
	bool drunken=bDrunken;
	bool clockwise=bClockwise;
	bool backwards=bBackwards;
	double circleXLoc=0.0f;
	double circleYLoc=0.0f;
	double circleRadiusMod=0.0f;
	bool bcircleXLoc=false;
	bool bcircleYLoc=false;
	bool bcircleRadiusMod=false;
	//bDebug=true;

	while( *currentArg ) {
		if( !strncmp(currentArg,"pause",6) ) {
			ReleaseKeys();
			PauseLogic = true;
			sprintf(szMsg, " \ayCircling  : %s", PauseLogic?"\arPAUSED":"\agRESUMED"); 
			WriteChatf(szMsg);
			return;
		} else if( !strncmp( currentArg,"unpause",8 ) ) {
			PauseLogic = false;
			sprintf(szMsg, " \ayCircling  : %s", PauseLogic?"\arPAUSED":"\agRESUMED"); 
			WriteChatf(szMsg);
			return;
		} else if( !strncmp(currentArg,"save",5 ) ) {
			Save_INI();
			return;
		} else if( !strncmp(currentArg,"load",5) ) {
			Load_INI();
			return;
		} else if( !strncmp(currentArg,"on",3) ) {
			PauseLogic = false;
			circleOn = true;
		} else if( !strncmp(currentArg,"off",4) ) {
			stickhasmovedfwd = bCircling = circleOn = false;
		} else if (!strncmp(currentArg,"drunken",8)) {
			drunken = drunken?false:true;
		} else if( !strncmp(currentArg,"clockwise",10) || !strncmp(currentArg,"cw",3)) {
			clockwise = true;
		} else if( !strncmp(currentArg,"ccw",4) || !strncmp(currentArg,"counterclockwise",17) || !strncmp(currentArg,"reverse",8)) {
			clockwise = false;
		} else if( !strncmp(currentArg,"forwards",9) ) {
			backwards = false;
		} else if( !strncmp(currentArg,"backwards",10) ) {
			backwards = true;
		} else if( !strncmp(currentArg,"mindelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0]))
				mindelay=atoi(currentArg);
			else {
				bMakeCamp = false;
				CircleHelp();
				return;
			}
		} else if( !strncmp(currentArg,"maxdelay",9) ) {
			GetArg(currentArg,szLine,argn++);
			if( isdigit(currentArg[0]))
				maxdelay=(int)atoi(currentArg);
			else {
				bMakeCamp = false;
				CircleHelp();
				return;
			}
		} else if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.') {
			if( !bcircleRadiusMod ) {
				bcircleRadiusMod = true;
         			circleRadiusMod = atof(currentArg);
			} else if ( !bcircleYLoc ) {
				bcircleYLoc = true;
				circleYLoc = atof(currentArg);
			} else if ( !bcircleXLoc ) {
				bcircleXLoc = true;
				circleXLoc = atof(currentArg);
			} else {
				CircleHelp();
				return;
			}
		} else {
			CircleHelp();
			return;
		}
		GetArg(currentArg,szLine,argn++);
	}

	if( circleOn ) {
		if( bcircleRadiusMod ) CircleRadius = (float)circleRadiusMod;
		CircleY = (float)(bcircleYLoc ? circleYLoc : pChar->Y + CircleRadius * sin(pChar->Heading * PI / 256.0));
		CircleX = (float)(bcircleXLoc ? circleXLoc : pChar->X + CircleRadius * cos(pChar->Heading * PI / 256.0));
	} else {
		bDrunken = backwards = false;
		clockwise = true;
	}

	bCircling = (circleOn && CircleRadius)?true:false;
	bClockwise = clockwise;
	bBackwards = backwards;

	if( bCircling ) {
		sprintf(szMsg, " \ayCircling Radius %g, center %g %g%s%s : \agON", CircleRadius, CircleY, CircleX, bClockwise?"":", Reverse", bBackwards?", Backwards":""); 
		WriteChatf(szMsg);
	} else {
		sprintf(szMsg, " \ayCircling Radius %g, center %g %g%s%s : \arOFF", CircleRadius, CircleY, CircleX, bClockwise?"":", Reverse", bBackwards?", Backwards":""); 
		WriteChatf(szMsg);
	}
}

void HandleCircle() {
	static int counter = 0;
	double distance;
	double heading;
	PSPAWNINFO pChSpawn = (PSPAWNINFO) pCharSpawn;
	PSPAWNINFO pLPlayer = (PSPAWNINFO) pLocalPlayer;
	if (!pChSpawn || !pLPlayer || !GetCharInfo2() ) {
		WriteChatColor("Null pointer, breaking Circle");
		breakCircle();
		return;
	}

	if ( ( ((long) (pChSpawn->CastingData.SpellID)) >= 0 && !(IsBardClass()) ) || (pChSpawn->StandState != STANDSTATE_STAND && pChSpawn->StandState != STANDSTATE_DUCK)|| GetCharInfo()->Stunned ) {
		if( !casting ) {
			casting = true;
			DoFwd(false);
			ReleaseKeys();
		}
	} else if( casting ) {
		casting = false;
	}

	if (!GetCharInfo() || !bCircling) return;

	double X = pChSpawn->X - CircleX;
	double Y = pChSpawn->Y - CircleY;
	distance = sqrt(X*X + Y*Y);

	if( !casting || !autoPauseEnabled ) {
		if (distance>(CircleRadius*(2.0/3.0))) {
			float newHeading;
			float pulseMoved=GetDistance(pChSpawn->X,pChSpawn->Y,prevX,prevY);
			if( pulseMoved < 5) pulseAvg = (pulseAvg + pulseMoved)/2;
			prevX=pChSpawn->X;
			prevY=pChSpawn->Y;
			prevZ=pChSpawn->Z;

			float SpeedMultiplier = *((float*) &(((PSPAWNINFO) pLocalPlayer)->SpeedMultiplier));

			if( stickhasmovedfwd && ((pulseAvg < (stuckDist + SpeedMultiplier) && !pChSpawn->UnderWater) ||
				(pulseAvg == 0 && pChSpawn->UnderWater))&&
				GetDistance(pChSpawn->X,pChSpawn->Y,(float)LocX,(float)LocY) > moveDist && stuckLogic && !GetCharInfo()->Stunned) {
					stuck++;
					if(fmod((float)stuck,(float)stuckCheck) == 0.0f) {
						newHeading =(float) ( pChSpawn->Heading + turnDirection);
						if( newHeading >= 512.0f) newHeading -= 512.0f;
						if( newHeading < 0.0f ) newHeading += 512.0f;
						pChSpawn->Heading = newHeading;

						//check to see if we are heading directly away from our target if so then go back
						newHeading=(float) atan2(pChSpawn->Y - CircleY,  CircleX - pChSpawn->X) * 180.0f / (float)PI + 90.0f;
						newHeading += (float)(90.0f * (CircleRadius/distance));
						newHeading *= 512.0f/360.0f;
						if( newHeading >= 512.0f ) newHeading -= 512.0f;
						if( newHeading < 0.0f ) newHeading += 512.0f;

						newHeading += 256.0f;
						if( newHeading >= 512.0f) newHeading -= 512.0f;
						if( newHeading < 0.0f ) newHeading += 512.0f;
						if( pChSpawn->Heading > (newHeading - fabs(turnDirection/2)) &&  pChSpawn->Heading <  (newHeading + fabs(turnDirection/2)) ) {
							newHeading -= 256.0f;
							if( newHeading >= 512.0f) newHeading -= 512.0f;
							if( newHeading < 0.0f ) newHeading += 512.0f;
							pChSpawn->Heading = newHeading;
							stuck = stuckCheck;
							turnDirection *= -1.0f;
						}
					}
					stuckFree=0;

			} else if( stuck > 0 ) {
				if(fmod((float)stuck,(float)stuckCheck) == 0.0f) {
					newHeading =(float) ( pChSpawn->Heading - turnDirection );
					pChSpawn->Heading = newHeading;
				}
				stuck--;
				if(stuckFree++ > stuckCheck*3 ) {
					stuck=0;
				}
			} else {
				heading = (bClockwise!=bBackwards)?atan2(pChSpawn->Y - CircleY, CircleX - pChSpawn->X) * 180.0f / PI:
				atan2(CircleY - pChSpawn->Y, pChSpawn->X - CircleX) * 180.0f / PI;
				if (bClockwise) heading += 90.0f + 90.0f * (CircleRadius/distance);
				if (!bClockwise) heading -= 90.0f + 90.0f * (CircleRadius/distance);
				heading *= 512.0f/360.0f;
				if( heading >= 512.0f ) heading -= 512.0f;
				if( heading < 0.0f ) heading += 512.0f;
				if( bDrunken ) {
					gFaceAngle = (float)heading;
				} else {
					pChSpawn->Heading = (float)heading;
				}
			}
		}
		if (!bBackwards) DoFwd(true);
		if (bBackwards) DoBck(true);
	}
}


void breakCircle(bool stopMoving, bool quite) {
	bCircling=false;
	bCirclingWasOn=false;
	stuck=0;
	if( stopMoving )
		ReleaseKeys();
	else {
		DoWalk(false);
		DoLft(false);
		DoRgt(false);
	}
}

// Utility Functions
float angularDistance(float h1, float h2)
{
	if( h1 == h2 ) return 0.0;

	if( fabs(h1-h2) > 256.0 )
		*(h1<h2?&h1:&h2) += 512.0;

	return (fabs(h1-h2)>256.0)?(h2-h1):(h1-h2);
}

float getRand(float n)
{
	return (n * rand() / (RAND_MAX+1.0f));
}

int millisDiff(SYSTEMTIME &stCurr, SYSTEMTIME &stPrev)
{
	SYSTEMTIME stResult;
	FILETIME ftPrev, ftCurr, ftResult;
	ULARGE_INTEGER prev,curr,result;

	GetSystemTime(&stCurr);
	SystemTimeToFileTime(&stPrev,&ftPrev);
	SystemTimeToFileTime(&stCurr,&ftCurr);
	prev.HighPart = ftPrev.dwHighDateTime;
	prev.LowPart = ftPrev.dwLowDateTime;
	curr.HighPart = ftCurr.dwHighDateTime;
	curr.LowPart = ftCurr.dwLowDateTime;
	result.QuadPart = curr.QuadPart - prev.QuadPart;
	ftResult.dwHighDateTime = result.HighPart;
	ftResult.dwLowDateTime = result.LowPart;
	FileTimeToSystemTime(&ftResult,&stResult);

	return ((int)(stResult.wSecond * 1000 + stResult.wMilliseconds));
}

bool IsBardClass()
{
	if(strncmp(pEverQuest->GetClassDesc(GetCharInfo2()->Class & 0xff),"Bard",5))
		return false;
	else
		return true;
}

//INI Functions
void Save_INI(void) {
	char szTemp[MAX_STRING];

	sprintf(szTemp,"%s",autoPauseEnabled?"on":"off");
	WritePrivateProfileString("Defaults","AutoPause",szTemp,INIFileName);

	sprintf(szTemp,"%s",mPause?"on":"off");
	WritePrivateProfileString("Defaults","ManualPause",szTemp,INIFileName);

	sprintf(szTemp,"%s",breakOnWarpEnabled?"on":"off");
	WritePrivateProfileString("Defaults","BreakOnWarp",szTemp,INIFileName);

	sprintf(szTemp,"%.1f",breakDist);
	WritePrivateProfileString("Defaults","BreakDist",szTemp,INIFileName);

	sprintf(szTemp,"%s",breakOnGateEnabled?"on":"off");
	WritePrivateProfileString("Defaults","BreakOnGate",szTemp,INIFileName);

	sprintf(szTemp,"%d",stickVerbosity);
	WritePrivateProfileString("Defaults","Verbosity",szTemp,INIFileName);

	sprintf(szTemp,"%.1f",stuckDist);
	WritePrivateProfileString("Defaults","stuckDist",szTemp,INIFileName);

	sprintf(szTemp,"%.1f",turnDirection);
	WritePrivateProfileString("Defaults","turnDirection",szTemp,INIFileName);

	sprintf(szTemp,"%i",stuckCheck);
	WritePrivateProfileString("Defaults","stuckCheck",szTemp,INIFileName);

	sprintf(szTemp,"%s",stuckLogic?"on":"off");
	WritePrivateProfileString("Defaults","StuckLogic",szTemp,INIFileName);

	sprintf(szTemp,"%s",bLeash?"on":"off");
	GetPrivateProfileString("Defaults","UseLeash","on",szTemp,MAX_STRING,INIFileName);
	bLeash=(strncmp(szTemp,"on",3)==0);

	sprintf(szTemp,"%.1f",leashLength);
	WritePrivateProfileString("Defaults","leashLength",szTemp,INIFileName);

	sprintf(szTemp,"%.1f",campRadius);
	WritePrivateProfileString("Defaults","campRadius",szTemp,INIFileName);

	sprintf(szTemp,"%i",mindelay);
	WritePrivateProfileString("Defaults","MinDelay",szTemp,INIFileName);

	sprintf(szTemp,"%i",maxdelay);
	WritePrivateProfileString("Defaults","MaxDelay",szTemp,INIFileName);
}

void Load_INI(void)
{
	char szTemp[MAX_STRING], szTemp2[MAX_STRING];

	// Defaults
	GetPrivateProfileString("Defaults","AutoPause","on",szTemp,MAX_STRING,INIFileName);
	autoPauseEnabled=(strncmp(szTemp,"on",3)==0);

	GetPrivateProfileString("Defaults","ManualPause","on",szTemp,MAX_STRING,INIFileName);
	mPause=(strncmp(szTemp,"on",3)==0);

	GetPrivateProfileString("Defaults","BreakOnWarp","on",szTemp,MAX_STRING,INIFileName);
	breakOnWarpEnabled=(strncmp(szTemp,"on",3)==0);

	GetPrivateProfileString("Defaults","BreakDist","250.0",szTemp,MAX_STRING,INIFileName);
	breakDist = (float)atof(szTemp);

	GetPrivateProfileString("Defaults","BreakOnGate","on",szTemp,MAX_STRING,INIFileName);
	breakOnGateEnabled=(strncmp(szTemp,"on",3)==0);

	stickVerbosity=(short)GetPrivateProfileInt("Defaults","Verbosity",1,INIFileName);

	GetPrivateProfileString("Defaults","stuckDist","0.1",szTemp,MAX_STRING,INIFileName);
	stuckDist = (float)atof(szTemp);

	GetPrivateProfileString("Defaults","turnDirection","10.0",szTemp,MAX_STRING,INIFileName);
	turnDirection = (float)atof(szTemp);

	GetPrivateProfileString("Defaults","stuckCheck","5",szTemp,MAX_STRING,INIFileName);
	stuckCheck = (int)atoi(szTemp);

	GetPrivateProfileString("Defaults","StuckLogic","on",szTemp,MAX_STRING,INIFileName);
	stuckLogic=(strncmp(szTemp,"on",3)==0);

	GetPrivateProfileString("Defaults","UseLeash","on",szTemp,MAX_STRING,INIFileName);
	bLeash=(strncmp(szTemp,"on",3)==0);

	sprintf(szTemp2,"%.1f",leashLength);
	GetPrivateProfileString("Defaults","leashLength",szTemp2,szTemp,MAX_STRING,INIFileName);
	leashLength = (float)atof(szTemp);

	sprintf(szTemp2,"%.1f",campRadius);
	GetPrivateProfileString("Defaults","campRadius",szTemp2,szTemp,MAX_STRING,INIFileName);
	campRadius = (float)atof(szTemp);

	mindelay=GetPrivateProfileInt("Defaults","MinDelay",500,INIFileName);

	maxdelay=GetPrivateProfileInt("Defaults","MaxDelay",5000,INIFileName);

	// Character specific
	GetPrivateProfileString(GetCharInfo()->Name,"AutoPause",autoPauseEnabled?"on":"off",szTemp,MAX_STRING,INIFileName);
	autoPauseEnabled=(strncmp(szTemp,"on",3)==0);

	GetPrivateProfileString(GetCharInfo()->Name,"BreakOnWarp",breakOnWarpEnabled?"on":"off",szTemp,MAX_STRING,INIFileName);
	breakOnWarpEnabled=(strncmp(szTemp,"on",3)==0);

	sprintf(szTemp2,"%.1f",breakDist);
	GetPrivateProfileString(GetCharInfo()->Name,"BreakDist",szTemp2,szTemp,MAX_STRING,INIFileName);
	breakDist = (float)atof(szTemp);

	GetPrivateProfileString(GetCharInfo()->Name,"BreakOnGate",breakOnGateEnabled?"on":"off",szTemp,MAX_STRING,INIFileName);
	breakOnGateEnabled=(strncmp(szTemp,"on",3)==0);

	stickVerbosity=(short)GetPrivateProfileInt(GetCharInfo()->Name,"Verbosity",stickVerbosity,INIFileName);
}

//Movement Related Functions
void ReleaseKeys() {
	DoWalk(false);
	DoFwd(false);
	DoBck(false);
	DoRgt(false);
	DoLft(false);
}

void DoWalk(bool walk) {
	bool state_walking = (*EQADDR_RUNWALKSTATE) ? false : true;
	float SpeedMultiplier = *((float*) &(((PSPAWNINFO) pLocalPlayer)->SpeedMultiplier));
	if (SpeedMultiplier < 0)
		walk = false; // we're snared, dont go into walk mode no matter what
	if ( (walk && !state_walking) || (!walk && state_walking) ) {
		MQ2Globals::ExecuteCmd(FindMappableCommand("run_walk"),1,0);
		MQ2Globals::ExecuteCmd(FindMappableCommand("run_walk"),0,0);
	}
}

void DoFwd(bool hold, bool walk) {
	static bool held = false;
	if ( hold ) {
		stickhasmovedfwd = true;
		DoWalk(walk);
		DoBck(false);
		if (!held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("forward"),1,0);
		}
		held = true;
	} else {
		DoWalk(false);
		if (held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("forward"),1,0);
			MQ2Globals::ExecuteCmd(FindMappableCommand("forward"),0,0);
		}
		held = false;
	}
}

void DoBck(bool hold) {
	static bool held = false;
	if( hold ) {
		DoFwd(false);
		if (!held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("back"),1,0);
		}
		held = true;
	} else {
		if (held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("back"),1,0);
			MQ2Globals::ExecuteCmd(FindMappableCommand("back"),0,0);
		}
		held = false;
	}
}

void DoLft(bool hold) {
	static bool held = false;
	if( hold ) {
		DoRgt(false);
		if (!held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_left"),1,0);
		}
		held = true;
	} else {
		if (held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_left"),1,0);
			MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_left"),0,0);
		}
		held = false;
	}
}

void DoRgt(bool hold) {
	static bool held = false;
	if( hold ) {
		DoLft(false);
		if (!held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_right"),1,0);
		}
		held = true;
	} else {
		if (held) {
			MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_right"),1,0);
			MQ2Globals::ExecuteCmd(FindMappableCommand("strafe_right"),0,0);
		}
		held = false;
	}
}
//returnTime = (int)rand()%(maxdelay-mindelay+1) + mindelay;

//MQ2Plugin Taps
VOID DoUnstickBind(PCHAR Name, BOOL Down)
{
	if( ! Down ) {
		keysDown--;
		if(PauseLogic) {
			GetSystemTime(&stReturnTime);
			returnTime = (int)rand()%(maxdelay-mindelay+1) + mindelay;
		}
	} else {
		keysDown++;
		if(!PauseLogic && (stickOn || bMoveToOn || bCircling || bMakeCamp) ) {
			//			&& strncmp(Name,"UNSTICK_STRAFE_RGT",12) && strncmp(Name,"UNSTICK_STRAFE_LFT",19)
			if( mPause  ) {
				PauseLogic = true;
				DoFwd(false);
				if(bMakeCamp && bMoveToOn) breakMoveTo();
				//			} else if( mPause ) {
				//				prevMoveBehind = moveBehind || prevMoveBehind;
				//				prevMovePin = movePin || prevMovePin;
				//				moveBehind = false;
			} else {
				if(stickOn)breakStick();
				//				if(stickOn)breakStick((strncmp(Name,"UNSTICK_BCK",12) && strncmp(Name,"UNSTICK_FWD",12)));
				if(bCircling) breakCircle();
				if(bMoveToOn) breakMoveTo();
		 }
		}
	}
}

void CreateBinds(){
	if (MoveBindsLoaded)
		return;
	MoveBindsLoaded=true;
	AddMQ2KeyBind("UNSTICK_FWD",DoUnstickBind);
	AddMQ2KeyBind("UNSTICK_BCK",DoUnstickBind);
	AddMQ2KeyBind("UNSTICK_LFT",DoUnstickBind);
	AddMQ2KeyBind("UNSTICK_RGT",DoUnstickBind);
	AddMQ2KeyBind("UNSTICK_STRAFE_LFT",DoUnstickBind);
	AddMQ2KeyBind("UNSTICK_STRAFE_RGT",DoUnstickBind);
	SetMQ2KeyBind("UNSTICK_FWD",false,pKeypressHandler->NormalKey[FindMappableCommand("forward")]);
	SetMQ2KeyBind("UNSTICK_FWD",true,pKeypressHandler->AltKey[FindMappableCommand("forward")]);
	SetMQ2KeyBind("UNSTICK_BCK",false,pKeypressHandler->NormalKey[FindMappableCommand("back")]);
	SetMQ2KeyBind("UNSTICK_BCK",true,pKeypressHandler->AltKey[FindMappableCommand("back")]);
	SetMQ2KeyBind("UNSTICK_LFT",false,pKeypressHandler->NormalKey[FindMappableCommand("left")]);
	SetMQ2KeyBind("UNSTICK_LFT",true,pKeypressHandler->AltKey[FindMappableCommand("left")]);
	SetMQ2KeyBind("UNSTICK_RGT",false,pKeypressHandler->NormalKey[FindMappableCommand("right")]);
	SetMQ2KeyBind("UNSTICK_RGT",true,pKeypressHandler->AltKey[FindMappableCommand("right")]);
	SetMQ2KeyBind("UNSTICK_STRAFE_LFT",false,pKeypressHandler->NormalKey[FindMappableCommand("strafe_left")]);
	SetMQ2KeyBind("UNSTICK_STRAFE_LFT",true,pKeypressHandler->AltKey[FindMappableCommand("strafe_left")]);
	SetMQ2KeyBind("UNSTICK_STRAFE_RGT",false,pKeypressHandler->NormalKey[FindMappableCommand("strafe_right")]);
	SetMQ2KeyBind("UNSTICK_STRAFE_RGT",true,pKeypressHandler->AltKey[FindMappableCommand("strafe_right")]);
}

void DestroyBinds()
{
	if (!MoveBindsLoaded)
		return;
	RemoveMQ2KeyBind("UNSTICK_FWD");
	RemoveMQ2KeyBind("UNSTICK_BCK");
	RemoveMQ2KeyBind("UNSTICK_LFT");
	RemoveMQ2KeyBind("UNSTICK_RGT");
	RemoveMQ2KeyBind("UNSTICK_STRAFE_LFT");
	RemoveMQ2KeyBind("UNSTICK_STRAFE_RGT");
}

// Called once, when the plugin is to initialize
PLUGIN_API void InitializePlugin(void)
{
	DebugSpewAlways("Initializing MQ2MoveUtils");

	// Add commands, macro parameters, hooks, etc.
   // FOLLOWPATH 
   AddCommand("/followpath",InitializeFollowPath); 
   AddCommand("/fp",InitializeFollowPath); 
   // END FOLLOWPATH 
	AddCommand("/circle",CircleCommand,0,1,1);
	AddCommand("/stick",StickCommand);
	AddMQ2Data("Stick",dataStick);
	AddCommand("/moveto",MoveToCommand,0,1,1);
	AddMQ2Data("MoveTo",dataMoveTo);
	AddCommand("/makecamp",MakeCampCommand,0,1,1);
	AddMQ2Data("MakeCamp",dataMakeCamp);

	pStickType = new MQ2StickType;
	pMakeCampType = new MQ2MakeCampType;
	pMoveToType = new MQ2MoveToType;

	srand((unsigned int)time(NULL));
	if (gGameState==GAMESTATE_INGAME) {
		CreateBinds();
	}
	GetSystemTime(&stPrevCirc);
	GetSystemTime(&stPrevStick);
}

// Called once, when the plugin is to shutdown
PLUGIN_API void ShutdownPlugin(void)
{
	DebugSpewAlways("Shutting down MQ2MoveUtils");

	// Remove commands, macro parameters, hooks, etc.
	RemoveMQ2Data("Stick");
	RemoveCommand("/circle");
	RemoveCommand("/stick");
	RemoveMQ2Data("MoveTo");
	RemoveCommand("/moveto");
	RemoveMQ2Data("MakeCamp");
	RemoveCommand("/makecamp");
   // FOLLOWPATH 
   RemoveCommand("/followpath"); 
   // END FOLLOWPATH 

	delete pStickType;
	delete pMoveToType;
	delete pMakeCampType;

	DestroyBinds();
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	if( breakOnGateEnabled && (stickHold?(stickTarget!=NULL):(ppTarget && pTarget)) ) {
		char szTemp[MAX_STRING];

		sprintf(szTemp,"%s Gates.",stickHold?stickTarget->DisplayedName:((PSPAWNINFO)pTarget)->DisplayedName);
		if( ! strcmp(szTemp,Line) ) {
			DoFwd(false);
			stickOn = false;
		}
	}

	return 0;
}

PLUGIN_API void SetGameState(DWORD GameState)
{
	if (GameState==GAMESTATE_INGAME) {
		CreateBinds();
		Load_INI();
	} else {
		stickOn=false;
		PauseLogic=false;
		stickHold=false;
		setDist=false;
		stickTarget=NULL;
		bCircling=false;
		bMakeCamp=false;
		bMoveToOn=false;
	}
}

// Called after entering a new zone
PLUGIN_API void OnZoned(void)
{
	DoWalk(false);
	breakStick();
}

PLUGIN_API void OnPulse(void)
{
	SYSTEMTIME stCurr;
	GetSystemTime(&stCurr);

	int elapsedTime = millisDiff(stCurr,stReturnTime);

	if( bDebug ) DebugSpew();
	if( bCircling && !PauseLogic) {
		if( bDrunken ) {
			SYSTEMTIME stCurr;
			GetSystemTime(&stCurr);
			if( millisDiff(stCurr,stPrevCirc) > 900 + (int)getRand(600.0) ) {
				GetSystemTime(&stPrevCirc);
				HandleCircle();
		 }
		} else {
			HandleCircle();
		}
	}
	if( stickOn && !PauseLogic) {
		if( looseStick ) {
			if( millisDiff(stCurr,stPrevStick) > 100 + (int)getRand(200.0) ) {
				GetSystemTime(&stPrevStick);
				HandleStick();
		 }
		} else {
			HandleStick();
		};
	} else if( stickOn && PauseLogic ) {
		//WriteChatf("elapsedTime (\ag%d\ax) returnTime (\ag%d\ax)", elapsedTime, returnTime);
		if( keysDown == 0 && mPause && elapsedTime >= returnTime && PauseLogic) {
			DoFwd(false);
			PauseLogic = false;
			stuck=0;
			stickhasmovedfwd=false;
			moveBehind = prevMoveBehind;
			movePin = prevMovePin;
			pulseAvg=10.0f;;
		}
	}
	if( bMoveToOn  && !PauseLogic ) {
		HandleMoveTo();
	}
	if (bMakeCamp) {
		HandleMakeCamp();
	}
   // FOLLOWPATH 
   if (bMoveToTrader && !PauseLogic) { 
      HandleMoveToTrader(); 
   } 
   // END FOLLOWPATH 
}

PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	if (pSpawn == stickTarget)
		stickTarget = NULL;
}

// FOLLOWPATH 

#include <string> 
#include <sstream> 
#include "math.h" 
#include <windows.h> 
#include <winbase.h> 

// Global constants 
const int Ypos = 1; 
const int Xpos = 2; 
const int Zpos = 3; 
const int maxNodes = 100;      // Change to allow more nodes per zone 
const int maxConnectors = 10;   // Change to allow more connections from each node 

// Global variables 
string positions[maxNodes]; 
int PosArray[maxNodes][maxConnectors]; 
int zoneNodes; 
int MyNearestNode; 
int TraderNearestNode; 
int pi[maxNodes]; 
int currentNode; 
float TraderYPos; 
float TraderXPos; 
float TraderZPos; 

void MoveToTrader(int current) 
// Used to kick off the Follow Path routine 
{ 
   currentNode = current; 
   // Set variables for MoveTo routines 
   LocY = (float) PosArray[currentNode][Ypos]; 
   LocX = (float) PosArray[currentNode][Xpos]; 
   // Kick off move 
   bMoveToOn=true; 
   bMoveToTrader=true; 
   StandUp(); 
} 

void HandleMoveToTrader() 
// Called on each pulse - checks if we are still moving 
{ 
   if (!bMoveToOn) { 
      // We've stopped so check if we are at the last node 
      if (currentNode == TraderNearestNode) { 
         // We are so just move to desired loc and turn off 
         LocY = (float) TraderYPos; 
         LocX = (float) TraderXPos; 
         bMoveToTrader=false; 
         bMoveToOn=true; 
         StandUp(); 
      } else { 
         // Not there yet, so work out next node 
         if (fpdebug) WriteChatf("At node %i",currentNode); 
         int x; 
         int i; 
         x = TraderNearestNode; 
         i = 1; 
         // Cycle through previous list to determine where to go next 
         // Limit the number of cycles (i<=zoneNodes) to prevent a node error causing an infinite loop 
         while ((!(pi[x]==currentNode))&&(i<=zoneNodes)) { 
            x = pi[x]; 
            i++; 
         } 
         if (i>zoneNodes) { 
            // Encountered a problem (e.g. a node with no connections) 
            WriteChatf("Problem with node %i.  Please check that the connections are correct in the INI file.",currentNode); 
            bMoveToTrader=false; 
            bMoveToOn=false; 
            return; 
         } 
         if (fpdebug) WriteChatf("Going to node %i",x); 
         // Set up variables for MoveTo routine 
         LocY = (float) PosArray[x][Ypos]; 
         LocX = (float) PosArray[x][Xpos]; 
         // And go 
         currentNode = x; 
         bMoveToOn=true; 
         StandUp(); 
      } 
   } 
} 

string IntToString(int num) 
// Convert Integer to String for processing 
{ 
  ostringstream myStream; 
  myStream << num << flush; 
  return(myStream.str()); 
} 

int GetToken(string Input, int index) 
// Parses the Input and returns the token chosen by index 
// Assumes the delimiter to be ';' 
{ 
   int pos; 
   string x; 

   pos = Input.find(";"); 
   if (pos == string::npos) { 
      if (index <= 1) { 
         return atoi(Input.c_str()); 
      } else { 
         return NULL; 
      } 
   } 

   if (index <= 1) { 
      return atoi(Input.substr(0,pos).c_str()); 
   } else { 
      return GetToken(Input.substr(pos+1,Input.length()-pos),index-1); 
   } 
} 

bool Read_INI(VOID) 
// Reads INI file to populate for Zone 
{ 
   char szTemp[MAX_STRING]; 
   char szZone[64]; 
   char szPosition[64]; 
   int i; 

   PCHARINFO pChar = GetCharInfo(); 

   sprintf(szZone,"%s",GetFullZone(pChar->zoneId)); 

   i = 1; 
   zoneNodes = 0; 
   while ((i<=maxNodes)&&(zoneNodes==0)) { 
      strcpy(szPosition,IntToString(i).c_str()); 
      GetPrivateProfileString(szZone,szPosition,"NULL",szTemp,MAX_STRING,INIFileName); 
      positions[i] = szTemp; 
      if (strstr(szTemp,"NULL")) { 
         zoneNodes = i - 1; 
         i = maxNodes; 
      } 
      i++; 
   } 

   if (zoneNodes==0) { 
      // There were no entries for current zone 
      WriteChatf("Error - couldn't read any nodes for %s.",szZone); 
      return false; 
   } else { 
      // All nodes read in 
      if (fpdebug) WriteChatf("Read %i nodes for %s.",zoneNodes,szZone); 
      return true; 
   } 
} 

void InitializeFollowPath(PSPAWNINFO pChar, PCHAR szLine) 
// This is the main FollowPath routine.  It reads the INI and sets up the 
// path for the desired node.  Note that the INI is read for each /followpath 
// command invokation.  This allows it to be dynamically altered and is only 
// a very slight overhead.  Good for debugging. 
// 
// Usage: 
//   /followpath [SpawnID]         e.g. /followpath 1182 
//   /followpath [SpawnName]       e.g. /followpath Ajaxx or /followpath "Banker Denton" 
//   /followpath locY locX [locZ]  e.g. /followpath 1130 -832 or /followpath -82 -950 45 
//   /followpath debugon 
//   /followpath debugoff 
// 
{ 
   // Main local variable declaration 
    int i; 
    int j; 
   char allArg[MAX_STRING]; 
   char currentArg[MAX_STRING]; 
   char nextArg[MAX_STRING]; 
   bool showusage = false; 

   // Concatanate all arguments together. 
   // For use in Spawn names, e.g. Banker Denton 
   i = 1; 
   GetArg(currentArg,szLine,i); 
   strcpy(allArg,currentArg); 
   while (i<10 && *currentArg) { 
      i = i + 1; 
      GetArg(currentArg,szLine,i); 
      if (*currentArg) { 
         strcat(allArg," "); 
      } 
      strcat(allArg,currentArg); 
   } 
   if (fpdebug) WriteChatf("Args: '%s'",allArg); 

   // Read in argument and check if we are switching debug on or off 
   GetArg(currentArg,szLine,1); 
   if (strcmp(currentArg,"debugon")==0) { 
      WriteChatf("FollowPath debug is ON."); 
      fpdebug = true; 
      return; 
   } 
   if (strcmp(currentArg,"debugoff")==0) { 
      WriteChatf("FollowPath debug is OFF."); 
      fpdebug = false; 
      return; 
   } 

   // Read in second argument - this will either be blank (if using ID or name) or 
   // will contain the X axis loc 
   GetArg(nextArg,szLine,2); 

   // Read node information from INI 
   if (fpdebug) WriteChatf("Reading INI file."); 
   if (!Read_INI()) { 
      return; 
   } 

   if (fpdebug) WriteChatf("Finished reading INI file."); 

   // Split data in INI into actual locations and node connectors 
   if (fpdebug) WriteChatf("Initialising position array."); 
    for (i=1;i<=zoneNodes;i++) { 
      char * str = strdup(positions[i].c_str()); 
       PosArray[i][1] = atoi(strtok (str,";")); 
      for (j=2;j<maxConnectors;j++) { 
         PosArray[i][j] = atoi(strtok (NULL,";")); 
      } 
   } 

   // Local variables for pathing algorithm 
   int s = 1; 
   int u; 
   int v; 
   int d[maxNodes]; 
   string X; 
   string Q; 
   string _Q; 
   int myclosest_dist; 
   int targetclosest_dist; 
   double w; 
   int x1; 
   int y1; 
   bool IgnoreZ; 
   PSPAWNINFO pTargetSpawn; 

   IgnoreZ = false; 

   if (fpdebug) WriteChatf("Searching for target."); 
   // Checking for blank argument 
   if (!*allArg) { 
      WriteChatColor("You must specify a valid name, ID or location"); 
      showusage = true; 
   } 

   // Attempt to find target by assuming the parameter is an ID or name 
   PSPAWNINFO pLPlayer = (PSPAWNINFO) pLocalPlayer; 
   if (isdigit(currentArg[0]) && (strlen(nextArg)==0)) { 
      // It is numeric and there is no second parameter, so try to resolve to player ID 
      if (fpdebug) WriteChatf("Looking for target by ID."); 
      pTargetSpawn = (PSPAWNINFO) GetSpawnByID(atoi(currentArg)); 
   } else { 
      // Alphanumeric so try to find the name in the spawn list 
      // Check first if we are trying to go to our TARGET 
      if (strcmp(strupr(currentArg),"TARGET")==0) { 
         // Looking for target 
         if (fpdebug) WriteChatf("Trying to move to our current target."); 
         if (pTarget) { 
            //pTargetSpawn = (PSPAWNINFO) pTarget; 
         } else { 
            if (fpdebug) WriteChatf("No current target selected."); 
         } 
      } else { 
         if (fpdebug) WriteChatf("Looking for target by name."); 
         SEARCHSPAWN ssSpawn; 
         ClearSearchSpawn(&ssSpawn); 
         ParseSearchSpawn(allArg,&ssSpawn); 
         pTargetSpawn = (PSPAWNINFO) SearchThroughSpawns(&ssSpawn,(PSPAWNINFO)pCharSpawn); 
      } 
   } 

   if( pTargetSpawn) { 
      // Spawn located so check we are not trying to locate ourselves 
      if (((PSPAWNINFO) pTargetSpawn)->SpawnID == ((PSPAWNINFO) pLPlayer)->SpawnID) { 
         WriteChatColor("You tried to move to yourself!"); 
         showusage = true; 
      } else { 
         // Set up target variables for spawn 
         if (fpdebug) WriteChatf("Determing location for target."); 
         TraderYPos = pTargetSpawn->Y; 
         TraderXPos = pTargetSpawn->X; 
         TraderZPos = pTargetSpawn->Z; 
      } 
   } else if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.' ) { 
      // Seems to be co-ordinates so try to use these instead 
      if (fpdebug) WriteChatf("Reading location arguments."); 
      TraderYPos = (float) atof(currentArg); 
      GetArg(currentArg,szLine,2); 
      if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.') { 
         TraderXPos = (float) atof(currentArg); 
         GetArg(currentArg,szLine,3); 
         // Check if we got Z axis co-ordinates 
         if( isdigit(currentArg[0]) || currentArg[0] == '-' || currentArg[0] == '.') { 
            // Yes, so set the variable accordingly 
            TraderZPos = (float) atof(currentArg); 
         } else { 
            // No, so ignore Z axis where possible 
            TraderZPos = 0.0; 
            IgnoreZ = true; 
         } 
      } else { 
         // Didn't receive suitable parameters 
         WriteChatColor("You must specify a valid name, ID or location"); 
         showusage = true; 
      } 
   } else { 
      // Didn't receive suitable parameters 
      WriteChatColor("You must specify a valid name, ID or location"); 
      showusage = true; 
   } 

   // Check if we need to display command usage and exit 
   if (showusage) { 
      WriteChatColor("Usage:",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath or /fp with the following arguments",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath <SpawnID>                e.g. /followpath 1182",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath <SpawnName>              e.g. /followpath Ajaxx or /followpath Banker Denton",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath <locY> <locX> [<locZ>]   e.g. /followpath 1130 -832 or /followpath -82 -950 45",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath target",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath debugon",CONCOLOR_YELLOW); 
      WriteChatColor("   /followpath debugoff",CONCOLOR_YELLOW); 
      return; 
   } 

   if (fpdebug) WriteChatf("Setting up target location variables."); 

   // Set up spawn structure for target - used for line-of-sight checks 
   SPAWNINFO Temp=*GetCharInfo()->pSpawn; 
   Temp.Y=TraderYPos; 
   Temp.X=TraderXPos; 
   Temp.Z=TraderZPos; 

   if (fpdebug) WriteChatf("Determining closest nodes to current loc and target loc."); 

   // Determine nearest nodes from me and from target 
   MyNearestNode = 0; 
   TraderNearestNode = 0; 
   myclosest_dist = 99999; 
   targetclosest_dist = 99999; 

   for (i=1;i<=zoneNodes;i++) { 
      // Check line of sight from us to first node 
      if (CastRay(pLPlayer,(float)PosArray[i][Ypos],(float)PosArray[i][Xpos],(float)PosArray[i][Zpos])) { 
         // Line of sight OK, so check distances 
         y1 = (int)abs((float)PosArray[i][Ypos]-(pLPlayer->Y)); 
         y1 = y1*y1; 
         x1 = (int)abs((float)PosArray[i][Xpos]-(pLPlayer->X)); 
         x1 = x1*x1; 
         w = x1+y1; 
         w = sqrt(w); 
         if (w < myclosest_dist) { 
            // This is the closest so far 
            myclosest_dist = (int) w; 
            MyNearestNode = i; 
         } 
      } 
      if (IgnoreZ) { 
         // If we are ignoring Z, assume target is always at same level as node 
         Temp.Z = (float)PosArray[i][Zpos]; 
      } 
      // Check line of sight for target to last node 
      if (CastRay(&Temp,(float)PosArray[i][Ypos],(float)PosArray[i][Xpos],(float)PosArray[i][Zpos])) { 
         // Line of sight OK, so check distances 
         y1 = (int)abs((float)PosArray[i][Ypos]-(Temp.Y)); 
         y1 = y1*y1; 
         x1 = (int)abs((float)PosArray[i][Xpos]-(Temp.X)); 
         x1 = x1*x1; 
         w = x1+y1; 
         w = sqrt(w); 
         if (w < targetclosest_dist) { 
            // This is the closest so far 
            targetclosest_dist = (int)w; 
            TraderNearestNode = i; 
         } 
      } 
   } 

   if (fpdebug) WriteChatf("Current closest node is %i.",MyNearestNode); 
   if (fpdebug) WriteChatf("Target closest node is %i.",TraderNearestNode); 

   // Check if we managed to resolve nodes 
   if (TraderNearestNode==0 || MyNearestNode==0) { 
      // Nope, so show error 
      WriteChatf("Mysterious forces prevent you from following the path..."); 
      return; 
   } 

   if (fpdebug) WriteChatf("Determining shortest path."); 

   // Now determine shortest path. 
   // First initialise distances, and set up node list 
   Q = ""; 
   for (i=1;i<=zoneNodes;i++) { 
      d[i] = 99999; 
      Q = Q + IntToString(i); 
      if (i<zoneNodes) Q = Q + ";"; 
   } 


   // Set distance on nearest node to 0, as it is closest 
   d[MyNearestNode] = 0; 

   // Now build up list of nodes in best path 
   for (i=1;i<=zoneNodes;i++) { 
      // Cycle through node list and determine closest to current node 
      u = GetToken(Q,1); 
      for (j=1;j<=(zoneNodes+1-i);j++) { 
         if (d[GetToken(Q,j)] < d[u]) { 
            u = GetToken(Q,j); 
         } 
      } 
      // Remove this node from node list 
      _Q = Q; 
      Q = ""; 
      for (j=1;j<=(zoneNodes+1-i);j++) { 
         if (GetToken(_Q,j) != u) { 
            Q = Q + IntToString(GetToken(_Q,j)) + ";"; 
         } 
      } 
      // For each connected node, update their distance from original node 
      for (j=4;j<maxConnectors;j++) { //Ignore last entry 
         v = PosArray[u][j]; 
         if (v > 0) { 
            y1 = abs(PosArray[u][Ypos]-PosArray[v][Ypos]); 
            y1 = y1*y1; 
            x1 = abs(PosArray[u][Xpos]-PosArray[v][Xpos]); 
            x1 = x1*x1; 
            w = x1+y1; 
            w = sqrt(w); 
            if (d[v] > (d[u] + w)) { 
               d[v] = (int)w + d[u]; 
               pi[v] = u; 
            } 
         } 
      } 
   } 

   if (fpdebug) { 
      // If debug mode is on, show the determined path 
      WriteChatf("Node path would be:"); 
      i = TraderNearestNode; 
      j = 1; 
      Q = IntToString(i); 
      while ((i != MyNearestNode)&&(j < zoneNodes)) { 
         i = pi[i]; 
         Q = IntToString(i) + "->" + Q; 
         j++; 
      }; 
      WriteChatf("%s",Q.c_str()); 
      //return; 
   }; 

   // Start moving to the destination 
   MoveToTrader(MyNearestNode); 
} 

//END FOLLOWPATH 