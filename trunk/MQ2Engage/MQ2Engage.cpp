// MQ2Engage.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.



#include "../MQ2Plugin.h"
#include <string>
#include <sstream>

PreSetup("MQ2Engage");

// BEGIN VARIABLES \\

//constants for slash command comparisons
#define ON "on"
#define OFF "off"
#define STICK "stick"
#define AUTOATTACK "autoattack"
#define RANGE "range"
#define BEHIND "behind"
#define FRONT "front"
#define PIN "pin"

//tracks current combat status
bool IsEngaged = false;

//current combat target
PSPAWNINFO pCurTar;

//max range to assist
int iAssistRange = 200;

//is it enraged?
bool IsEnraged = false;

//stick type
CHAR * szStickType = "behind";

//stick distance
int iStickDist = 12;

//do i stick?
bool bDoStick = false;

//do i use autoattack?
bool bDoAutoAttack = false;

//have I attacked yet?
bool bHaveAttacked = false;

//have I stuck yet?
bool bHaveStuck = false;

//attack commands
CHAR szAttackCmd[MAX_STRING] = {0};
#define ATTACKON "/attack on"
#define ATTACKOFF "/attack off"
#define AUTOFIRE "/autofire"
#define STICKOFF "/stick off"

//class for the TLO, Combat
class MQ2CombatType *pCombatType = 0;
// END VARIABLES \\

// BEGIN PROTOTYPES \\

VOID HandleTargeting(PSPAWNINFO);
VOID DoStick(std::string);
VOID DoStick(PSPAWNINFO, std::string);
FLOAT GetDistanceBetween(PSPAWNINFO, PSPAWNINFO);
bool StringIsInt(std::string);
int StringToInt(std::string);
VOID EngageCommand(PSPAWNINFO, PCHAR);
VOID SetupClassDefaults();
VOID DisengageCommand(PSPAWNINFO, PCHAR);
VOID CombatCommand(PSPAWNINFO, PCHAR);
bool IsEngageTargetAlive();
bool IsTargetInRange();
VOID EnrageOn();
VOID EnrageOff();
// END PROTOTYPES \\

// BEGIN FUNCTION DEFINITIONS \\

/*
Returns the distance between two spawns
*/
FLOAT GetDistanceBetween(PSPAWNINFO pOne, PSPAWNINFO pTwo) {
	FLOAT xDif = abs(pOne->X - pTwo->X);
	FLOAT yDif = abs(pOne->Y - pTwo->Y);
	FLOAT zDif = abs(pOne->Z - pTwo->Z);
	return sqrt(pow(xDif, 2) + pow(yDif, 2) + pow(zDif, 2));
}

bool StringIsInt(std::string value) {
	std::istringstream iss(value);
	int integer = 0;

	if(!(iss >> integer))
		return false;
	else
		return true;
}

BOOL AmInCombat() {
	return *EQADDR_ATTACK;
}

int StringToInt(std::string value) {
	std::istringstream iss(value);
	int integer = 0;

	if(!(iss >> integer))
		return false;
	else
		return integer;
}

/*
Engages the current target.
*/
VOID EngageCommand(PSPAWNINFO pChar, PCHAR szLine) {
	//if autoassisting, we need to check the assist for a target
	if (IsEngaged) 
		DisengageCommand(pChar, szLine);

	CHAR szArg[MAX_STRING] = {0};

	GetArg(szArg, szLine, 1);

	if (strlen(szArg) < 1)
		return;

	//better be an integer.....
	if (StringIsInt(szArg)) {
		pCurTar = (PSPAWNINFO)GetSpawnByID(StringToInt(szArg));

		//is it in range?
		if (GetDistanceBetween(pChar, pCurTar) < iAssistRange)
			HandleTargeting(pCurTar);
		else
			return;
	} else {
		return;
	}

	IsEngaged = true;
}

VOID SetupClassDefaults() {
	DWORD myClass = GetCharInfo2()->Class;
	if(!(myClass == Mage || myClass == Cleric || myClass == Druid || myClass == Shaman || myClass == Mage || myClass == Necromancer || myClass == Wizard || myClass == Enchanter)) {
		bDoStick = true;
		bDoAutoAttack = true;
		strcpy(szAttackCmd, ATTACKON);
	}
}

VOID Clear() {
	IsEngaged = false;
	IsEnraged = false;
	bHaveAttacked = false;
	bHaveStuck = false;
}

/*
Disengages from the current combat.
*/
VOID DisengageCommand(PSPAWNINFO pChar, PCHAR szLine) {
	Clear();

	//turn off attack
	EzCommand(ATTACKOFF);
}

VOID CombatCommand(PSPAWNINFO pChar, PCHAR szLine) {
	CHAR szArg[MAX_STRING] = {0};
	CHAR szTemp[MAX_STRING] = {0};

	GetArg(szArg, szLine, 1);

	if (stricmp(szArg, RANGE) == 0) {
		GetArg(szArg, szLine, 2);
		if (StringIsInt(szArg)) {
			iAssistRange = StringToInt(szArg);
		} 

		sprintf(szTemp, "Assist range is: %d units", iAssistRange);
	} else if (stricmp(szArg, AUTOATTACK) == 0) {
		GetArg(szArg, szLine, 2);
		if (stricmp(szArg, ON) == 0) {
			bDoAutoAttack = true;
		} else if (stricmp(szArg, OFF) == 0) {
			bDoAutoAttack = false;
		}

		sprintf(szTemp, "Autoattack is now: %s", (bDoAutoAttack)?(ON):(OFF));
	} else if (stricmp(szArg, STICK) == 0) {
		GetArg(szArg, szLine, 2);
		if (stricmp(szArg, ON) == 0) {
			bDoStick = true;
		} else if (stricmp(szArg, OFF) == 0) {
			bDoStick = false;
		} else if (stricmp(szArg, BEHIND) == 0) {
			szStickType = BEHIND;
		} else if (stricmp(szArg, PIN) == 0) {
			szStickType = PIN;
		} else if (stricmp(szArg, FRONT) == 0) {
			szStickType = FRONT;
		}

		sprintf(szTemp, "Stick[%s] :: Type[%s]", (bDoStick)?(ON):(OFF), szStickType);
	}

	if (strlen(szTemp) > 0) WriteChatColor(szTemp);
}

/*
This handles sticking to the current target.
*/
VOID DoStick(std::string command) {
	char stick[MAX_STRING] = {0};
	strcpy(stick, command.c_str());
	EzCommand(STICKOFF);
	EzCommand(stick);
}

/*
Targets a specific spawn by name
*/
VOID HandleTargeting(PSPAWNINFO pSpawn) {
	char temp[MAX_STRING] = {0};
	std::string target;
	target += "/target ";
	target += pSpawn->Name;
	strcpy(temp, target.c_str());
	EzCommand(temp);
}

/*
This handles sticking to a particular spawn. Currently no LoS checking or anything
*/
VOID DoStick(PSPAWNINFO pSpawn, std::string command) {
	HandleTargeting(pSpawn);

	char stick[MAX_STRING] = {0};
	strcpy(stick, command.c_str());
	EzCommand(stick);
}

/*
Checks to see if the current combat target is alive.
*/
bool IsEngageTargetAlive() {
	if(GetSpawnType(pCurTar) == CORPSE)
		return false;

	if(!GetSpawnByID(pCurTar->SpawnID))
		return false;

	return true;
}

/*
Verifies that a target is in range
*/
bool IsTargetInRange() {
	return GetDistanceBetween(((PSPAWNINFO)GetCharInfo()->pSpawn), pCurTar) < iAssistRange;
}

VOID EnrageOn() {
	IsEnraged = true;
	WriteChatColor("Here i am");
	EzCommand(ATTACKOFF);
}

VOID EnrageOff() {
	IsEnraged = false;

	if (bDoAutoAttack) 
		bHaveAttacked = false;
}

// END FUNCTION DEFINITIONS \\

// CLASS DEFINITION \\

class MQ2CombatType : public MQ2Type {
private:

public:
	enum CombatMembers {
		Engaged=1,
		Target=2,
		Range=3,
		Enraged=4,
		AutoStick=5,
		AutoAttack=6,
		StickType=7
	};

	MQ2CombatType():MQ2Type("Combat") {
		TypeMember(Engaged);
		TypeMember(Target);
		TypeMember(Range);
		TypeMember(Enraged);
		TypeMember(AutoStick);
		TypeMember(AutoAttack);
		TypeMember(StickType);
	}

	~MQ2CombatType() {}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2CombatType::FindMember(Member);
		if (!pMember)
			return false;
		switch((CombatMembers)pMember->ID) {
			case Engaged:
				Dest.DWord = IsEngaged;
				Dest.Type = pBoolType;

				return true;
			case Target:
				if (IsEngaged) {
					Dest.Type = pSpawnType;
					Dest.Ptr = GetSpawnByID(pCurTar->SpawnID);
				} else {
					return false;
				}				
				return true;
			case Range:
				Dest.Int = iAssistRange;
				Dest.Type = pIntType;
				return true;
			case Enraged:
				Dest.Type = pBoolType;
				Dest.DWord = IsEnraged;
				return true;
			case AutoStick:
				Dest.Type = pBoolType;
				Dest.DWord = bDoStick;
				return true;
			case AutoAttack:
				Dest.Type = pBoolType;
				Dest.DWord = bDoAutoAttack;
				return true;
			case StickType:
				Dest.Type = pStringType;
				strcpy(DataTypeTemp, szStickType);
				Dest.Ptr = DataTypeTemp;
				return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		if (IsEngaged)
			strcpy(Destination, "TRUE");
		else 
			strcpy(Destination, "FALSE");

		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

BOOL dataCombat(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pCombatType;

	return true;
}
// END CLASS DEFINITION \\

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2Engage");

	AddCommand("/engage", EngageCommand);
	AddCommand("/disengage", DisengageCommand);
	AddCommand("/combat", CombatCommand);

	AddMQ2Data("Combat", dataCombat);
	pCombatType = new MQ2CombatType;


	// Add commands, MQ2Data items, hooks, etc.
	// AddCommand("/mycommand",MyCommand);
	// AddXMLFile("MQUI_MyXMLFile.xml");
	// bmMyBenchmark=AddMQ2Benchmark("My Benchmark Name");
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Engage");

	RemoveCommand("/engage");
	RemoveCommand("/disengage");
	RemoveCommand("/combat");

	RemoveMQ2Data("Combat");
	delete pCombatType;

	// Remove commands, MQ2Data items, hooks, etc.
	// RemoveMQ2Benchmark(bmMyBenchmark);
	// RemoveCommand("/mycommand");
	// RemoveXMLFile("MQUI_MyXMLFile.xml");
}

// Called after entering a new zone
PLUGIN_API VOID OnZoned(VOID)
{
	DebugSpewAlways("MQ2Engage::OnZoned()");
	if(IsEngaged) {
		Clear();
	}
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
	DebugSpewAlways("MQ2Engage::SetGameState()");
	if (GameState != GAMESTATE_INGAME && IsEngaged) {
		Clear();
	} else if (GameState == GAMESTATE_INGAME) {
		SetupClassDefaults();
	}
	// create custom windows if theyre not set up, etc
}


// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	if(IsEngaged && pTarget) {
		//need to verify spawn is still alive
		if(!IsEngageTargetAlive() || !IsTargetInRange()) {
			Clear();
			return;
		} 

		//stick to target
		if (bDoStick && ((PSPAWNINFO)pTarget)->SpawnID == pCurTar->SpawnID && !bHaveStuck) {
			char temp[MAX_STRING] = {0};
			sprintf(temp, "/stick %s %d", szStickType, iStickDist);
			DoStick(temp);
			bHaveStuck = true;
		}

		//turn attack on
		if (bDoAutoAttack && !AmInCombat() && ((PSPAWNINFO)pTarget)->SpawnID == pCurTar->SpawnID && !bHaveAttacked) {
			WriteChatColor(szAttackCmd);
			WriteChatColor(((PSPAWNINFO)pTarget)->Name);
			EzCommand(szAttackCmd);
			bHaveAttacked = true;
		}
		//need to do special attacks based on class...what is the easiest way..?
	}

	// DONT leave in this debugspew, even if you leave in all the others
//	DebugSpewAlways("MQ2Engage::OnPulse()");
}

// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
// but after MQ filters and chat events are taken care of.
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	//check for enrage here? if it is enraged, why not have it turn off 
	//attack if the mob is not facing away from us? This could save a whole 
	//lot of headaches
	if (IsEngaged && !IsEnraged) {
		std::string sToFind, sLine;
		sLine = Line;
		sToFind += pCurTar->DisplayedName;
		sToFind += " has become enraged";
		if (sLine.find(sToFind) != std::string::npos) {
			EnrageOn();
		}
	} else if (IsEngaged && IsEnraged) {
		std::string sToFind, sLine;
		sLine = Line;
		sToFind += pCurTar->DisplayedName;
		sToFind += " is no longer enraged";
		if (sLine.find(sToFind) != std::string::npos) {
			EnrageOff();
		}
	}

	DebugSpewAlways("MQ2Engage::OnIncomingChat(%s)",Line);
	return 0;
}

// This is called each time a spawn is added to a zone (inserted into EQ's list of spawns),
// or for each existing spawn when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn)
{
	DebugSpewAlways("MQ2Engage::OnAddSpawn(%s)",pNewSpawn->Name);
	if (IsEngaged) {
		if (pNewSpawn->SpawnID == pCurTar->SpawnID)
			Clear();
	}
}

// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	DebugSpewAlways("MQ2Engage::OnRemoveSpawn(%s)",pSpawn->Name);
	if(IsEngaged && pSpawn->SpawnID == pCurTar->SpawnID) {
		Clear();
	}
}
