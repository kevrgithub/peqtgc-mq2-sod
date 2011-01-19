// MQ2RenoDPS.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.



#include "../MQ2Plugin.h"
#include <time.h>


PreSetup("MQ2RenoDPS");


//globals are a no-no! Never do this!
bool bParse = false;
bool Attacking = false;
bool bShowDetails = false;
bool bShowNPC = false;
char TargetName[64] = "";
char TargetCleanName[64] = "";
char PetName[64] = "";
DWORD PetID = 0;
bool HavePet = false;
PSPAWNINFO pKillTarget;
time_t CombatStart;
unsigned int KickDmg, BashDmg, SlashDmg, BluntDmg, PunchDmg, PierceDmg, BackstabDmg, SpellDmg, HitDmg, DoTDmg, PetDmg;
unsigned int TotalDmg;
double HitCount, MissCount;
double DPS;
double HitRatio;
unsigned int TotalDmgIn;


//prototypes
void AddDamage(std::string);
unsigned int GetDamage(std::string);
void DoCalc();
void CleanUp();
void AddNPCDamage(std::string);

//testing
void PrintShit(const char * incoming) {
	char szTemp[MAX_STRING];

	sprintf(szTemp, "MESSAGE: %s.", incoming);

	WriteChatColor(szTemp);
}

//testing
void PrintShit(double incoming) {
	char szTemp[MAX_STRING];

	sprintf(szTemp,"Damage Per Second: %f.", incoming);

	WriteChatColor(szTemp);
}


/**
Shows the stats for the last fight to the chat window
*/
void ShowStats() {
	char szTemp[MAX_STRING];

	sprintf(szTemp,"Statistics for %s.", TargetCleanName);
	WriteChatColor(szTemp);

	sprintf(szTemp,"Damage Per Second: \ar%.2f\ax.", DPS);
	WriteChatColor(szTemp);

	sprintf(szTemp,"Total Damage: \ar%u\ax.", TotalDmg);
	WriteChatColor(szTemp);

	sprintf(szTemp,"Hit Ratio: \ay%.2f\ax %.", HitRatio);
	WriteChatColor(szTemp);

	if(bShowDetails) {
		if(HitCount > 0 || MissCount > 0) {
			sprintf(szTemp,"Hits: \ay%u\ax.", (int) HitCount);
			WriteChatColor(szTemp);
			sprintf(szTemp,"Misses: \ay%u\ax.", (int) MissCount);
			WriteChatColor(szTemp);
		}  
		if(KickDmg > 0) {
			sprintf(szTemp,"Total Kick Damage: \ar%u\ax.", KickDmg);
			WriteChatColor(szTemp);
		} 
		if(BashDmg > 0) {
			sprintf(szTemp,"Total Bash Damage: \ar%u\ax.", BashDmg);
			WriteChatColor(szTemp);
		} 
		if(SlashDmg > 0) {
			sprintf(szTemp,"Total Slash Damage: \ar%u\ax.", SlashDmg);
			WriteChatColor(szTemp);
		} 
		if(BluntDmg > 0) {
			sprintf(szTemp,"Total Blunt Damage: \ar%u\ax.", BluntDmg);
			WriteChatColor(szTemp);
		} 
		if(PierceDmg > 0) {
			sprintf(szTemp,"Total Pierce Damage: \ar%u\ax.", PierceDmg);
			WriteChatColor(szTemp);
		} 
		if(PunchDmg > 0) {
			sprintf(szTemp,"Total Punch Damage: \ar%u\ax.", PunchDmg);
			WriteChatColor(szTemp);
		} 
		if(BackstabDmg > 0) {
			sprintf(szTemp,"Total Backstab Damage: \ar%u\ax.", BackstabDmg);
			WriteChatColor(szTemp);
		}
		if(SpellDmg > 0) {
			sprintf(szTemp,"All Spell Direct Damage: \ar%u\ax.", SpellDmg);
			WriteChatColor(szTemp);
		}
		if(DoTDmg > 0) {
			sprintf(szTemp,"All Spell DoT Damage: \ar%u\ax.", DoTDmg);
			WriteChatColor(szTemp);
		}
		if(PetDmg > 0) {
			sprintf(szTemp,"Pet: \ar%u\ax.", PetDmg);
			WriteChatColor(szTemp);
		}
		if(HitDmg > 0) {
			sprintf(szTemp,"All Other Damage: \ar%u\ax.", HitDmg);
			WriteChatColor(szTemp);
		}
	}

	//NPC stats
	if(bShowNPC) {
		if(TotalDmgIn > 0) {
			sprintf(szTemp,"Incoming Damage: \ar%u\ax.", TotalDmgIn);
			WriteChatColor(szTemp);
		}
	}
}

PSPAWNINFO GetPet() {
	return (PSPAWNINFO)GetSpawnByID(GetCharInfo()->pSpawn->PetID);
}

/**
Starts and stops the parser
attack - true - Turn parsing on if not already on
attack - false - Turn parsing off if it is on.
*/
void SetAttacking(bool attack) {
	if(attack && !Attacking) {
		Attacking = true;

		time(&CombatStart);

		pKillTarget = ((PSPAWNINFO) pTarget);
		sprintf(TargetName, "%s", pKillTarget->Name);
		sprintf(TargetCleanName, "%s", pKillTarget->DisplayedName);

		CleanUp();
	}

	if(!attack && Attacking) {
		Attacking = false;
		DoCalc();
		CleanUp();
	}

}

/**
Clean up variables after a fight
*/
void CleanUp() {
	KickDmg = 0, BashDmg = 0, SlashDmg = 0, BluntDmg = 0, PunchDmg = 0, PierceDmg = 0, BackstabDmg = 0, SpellDmg = 0, HitDmg = 0, DoTDmg = 0, PetDmg = 0;
	TotalDmg = 0;
	DPS = 0;
	HitCount = 0;
	MissCount = 0;
	HitRatio = 0;
	TotalDmgIn = 0;
}

/**
Parses a string of text that is incoming
*/
void ParseText(std::string line) {
	if(line == "Auto attack is on.")
		SetAttacking(true);
	else if(line == "Auto attack is off.")
		SetAttacking(false);
	else if(line == "Auto fire on.")
		SetAttacking(true);
	else if(line == "Auto fire off.")
		SetAttacking(false);
	else if(! (line.find(TargetCleanName) == std::string.npos) )
		AddDamage(line);
}

/**
Will add damage.
*/
void AddDamage(std::string data) {
	//Duh! Why are you here? Go away!
	if(!Attacking)
		return;
	
	//NPC is hitting me, Don't add it
	//NOTE: NOT working
	/**
	if(! (data.find("YOU") == std::string.npos) || ( (data.find(TargetCleanName) == 0) && ( (data.find("damage from your") == std::string.npos) || !(data.find("by non-melee") == std::string.npos) ) ) ) {
		AddNPCDamage(data);
		return;
	}
	*/
	unsigned int damage = GetDamage(data);

	//Keep out Finishing Moves
	if(damage == 32000)
		return;

	//was it my pet?
	if(!(data.find(GetPet()->DisplayedName) == std::string.npos)) {
		PetDmg += damage;
		return;
	}

	//duh, we missed
	if( (data.find("misses") != std::string.npos) || (data.find("missed") != std::string.npos) ) {
		++MissCount;
		return;
	}

	//TODO: need to add bite dmg for pets and illusions
	//      do i add pet's as a seperate entity?
	if(! (data.find("non-melee") == std::string.npos) ) {
		SpellDmg += damage;
		return;
	} else if(! (data.find("hit") == std::string.npos) ) {
		HitDmg += damage;
	} else if(! (data.find("kick") == std::string.npos) ) {
		KickDmg += damage;
	} else if(! (data.find("bash") == std::string.npos) ) {
		BashDmg += damage;
	} else if(! (data.find("slash") == std::string.npos) ) {
		SlashDmg += damage;
	} else if(! (data.find("crush") == std::string.npos) ) {
		BluntDmg += damage;
	} else if(! (data.find("punch") == std::string.npos) ) {
		PunchDmg += damage;
	} else if(! (data.find("pierce") == std::string.npos) ) {
		PierceDmg += damage;
	} else if(! (data.find("backstab") == std::string.npos) ) { 
		BackstabDmg += damage;	
	} else if(! (data.find("damage from your") == std::string.npos) ) { 
		DoTDmg += damage;	
	}

	//Keep a running hit count total
	if(damage > 0)
		++HitCount;
	
}

/**
Get's the damage from a line of text and makes it an integer
*/
unsigned int GetDamage(std::string data) {
	unsigned int damage = 0;
	std::string temp;
	for(int i = 0; i < data.size(); ++i) {
		if(isdigit(data[i])) {
			temp.append(1, data[i]);
		}
	}

	damage = atoi(temp.c_str());
	return damage;
}

/**
Performs all final DPS calculations
*/
void DoCalc() {
	time_t end = time(NULL);
	double TotalTime = difftime(end, CombatStart);
	
	TotalDmg = KickDmg + BashDmg + SlashDmg + BluntDmg + PunchDmg + PierceDmg + BackstabDmg + SpellDmg + HitDmg + DoTDmg + PetDmg;

	DPS = TotalDmg / TotalTime;
	
	HitRatio = (HitCount / (HitCount + MissCount)) * 100;

	ShowStats();
}

void AddNPCDamage(std::string data) {
	TotalDmgIn += GetDamage(data);
}

/**
Handles all the commands for /dps
Params:
	on - Turns DPS parsing on.
	off - Turns DPS parsing off.
	details - Toggles showing details on/off
	npc - Toggles showing NPC damage statistics
*/
void DPSCommand(PSPAWNINFO pChar, PCHAR szLine) {
	CHAR currentArg[MAX_STRING];
	int argn=1;
	GetArg(currentArg,szLine,argn++);

	if (!stricmp(currentArg,"help") || szLine[0]==0) {
		//CommandHelp();
		return;
	}

	/* /dps on */
	if (!stricmp(currentArg,"on") || szLine[0]==0) {
		bParse = true;
		WriteChatColor("DPS Parsing \arON\ax");
		return;
	}

	/* /dps off */
	if (!stricmp(currentArg,"off") || szLine[0]==0) {
		bParse = false;
		WriteChatColor("DPS Parsing \arOFF\ax");
		return;
	}

	/* /dps details */
	//if details view is currently on
	if ((!stricmp(currentArg,"details") || szLine[0]==0) && bShowDetails) {
		bShowDetails = false;
		WriteChatColor("Detail View \arOFF\ax");
		return;
	}

	/* /dps details */
	//if details view is currently off
	if ((!stricmp(currentArg,"details") || szLine[0]==0) && !bShowDetails) {
		bShowDetails = true;
		WriteChatColor("Details View \arON\ax");
		return;
	}

	/* /dps npc  */
	//if npc view is currently on
	if ((!stricmp(currentArg,"npc") || szLine[0]==0) && !bShowNPC) {
		bShowNPC = true;
		WriteChatColor("Details View \arOFF\ax");
		return;
	}

	/* /dps npc  */
	//if npc view is currently off
	if ((!stricmp(currentArg,"npc") || szLine[0]==0) && bShowNPC) {
		bShowNPC = false;
		WriteChatColor("Details View \arON\ax");
		return;
	}
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
    DebugSpewAlways("Initializing MQ2RenoDPS");
	
	AddCommand("/dps", DPSCommand);
    //Add commands, MQ2Data items, hooks, etc.
    //AddCommand("/mycommand",MyCommand);
    //AddXMLFile("MQUI_MyXMLFile.xml");
    //bmMyBenchmark=AddMQ2Benchmark("My Benchmark Name");
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
    DebugSpewAlways("Shutting down MQ2RenoDPS");
	
	RemoveCommand("/dps");
    //Remove commands, MQ2Data items, hooks, etc.
    //RemoveMQ2Benchmark(bmMyBenchmark);
    //RemoveCommand("/mycommand");
    //RemoveXMLFile("MQUI_MyXMLFile.xml");
}

// Called after entering a new zone
PLUGIN_API VOID OnZoned(VOID)
{
	if(bParse && Attacking) {
		Attacking = false;
		CleanUp();
	}
    DebugSpewAlways("MQ2RenoDPS::OnZoned()");
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	if(Attacking && pKillTarget->Type == SPAWN_CORPSE) {
		Attacking = false;
		DoCalc();
		CleanUp();
	}
    // DONT leave in this debugspew, even if you leave in all the others
    //DebugSpewAlways("MQ2RenoDPS::OnPulse()");
}

// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
// but after MQ filters and chat events are taken care of.
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	if(bParse)
		ParseText(Line);

    DebugSpewAlways("MQ2RenoDPS::OnIncomingChat(%s)",Line);
    return 0;
}

// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	if(Attacking && pSpawn->SpawnID == pKillTarget->SpawnID) {
		Attacking = false;
		DoCalc();
		CleanUp();
	}
}