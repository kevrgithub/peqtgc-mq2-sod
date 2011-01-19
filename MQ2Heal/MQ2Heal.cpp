// MQ2Heal.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.



#include "../MQ2Plugin.h"

PreSetup("MQ2Heal");

typedef LONG (__cdecl *fNetBot)(VOID);

class MQ2HealType *pHealType = 0;

BOOL IsCorpse(PSPAWNINFO pSpawn) {
	std::string temp = pSpawn->Name; 
	//WriteChatColor(pSpawn->Name);
	if (temp.find("'s corpse") != std::string::npos)
		return true;

	return false;
}

INT CountGroupMembersInRange(INT MaxRange) {
	INT count = 0;
	PCHARINFO pChar = GetCharInfo();
	if (!pChar->pGroupInfo) 
		return false;
	for (int i = 1; i < 6; i++) {
		if (pChar->pGroupInfo->pMember[i]) {
			PSPAWNINFO pTemp = pChar->pGroupInfo->pMember[i]->pSpawn;
			if (!pTemp)
				continue;
			if (GetDistance((PSPAWNINFO)pChar, pTemp) < MaxRange && !IsCorpse(pTemp) && pTemp->Type != PET)
				++count;
		}
	}

	return count;
}

INT CountGroupMembersBelow(INT MaxHPToCount) {
	INT count = 0;
	PCHARINFO pChar = GetCharInfo();
	if (!pChar->pGroupInfo) 
		return false;
	for (int i = 1; i < 6; i++) {
		if (pChar->pGroupInfo->pMember[i]) {
			PSPAWNINFO pTemp = pChar->pGroupInfo->pMember[i]->pSpawn;
			if (!pTemp)
				continue;
			if (pTemp->HPCurrent < MaxHPToCount && !IsCorpse(pTemp) && pTemp->Type != PET) {
				++count;
			}
		}
	}
	
	
	if (((PSPAWNINFO)pCharSpawn)->HPCurrent < MaxHPToCount)
		++count;

	return count;
}

INT CountGroupMembersAbove(INT MinHPToCount) {
	INT count = 0;
	PCHARINFO pChar = GetCharInfo();
	if (!pChar->pGroupInfo) 
		return false;
	for (int i = 1; i < 6; i++) {
		if (pChar->pGroupInfo->pMember[i]) {
			PSPAWNINFO pTemp = pChar->pGroupInfo->pMember[i]->pSpawn;
			if (!pTemp)
				continue;
			if (pTemp->HPCurrent > MinHPToCount && !IsCorpse(pTemp) && pTemp->Type != PET)
				++count;
		}
	}

	if (((PSPAWNINFO)pChar)->HPCurrent > MinHPToCount)
		++count;

	return count;
}

PSPAWNINFO FindWorstInjuredInGroup() {
	int lowHP = 100;
	PSPAWNINFO pGroupMember = 0;
	PCHARINFO pChar = GetCharInfo();
	if (!pChar->pGroupInfo) 
		return false;
	for (int i = 1; i < 6; i++) {
		if (pChar->pGroupInfo->pMember[i]) {
			PSPAWNINFO pTemp = pChar->pGroupInfo->pMember[i]->pSpawn;
			if (!pTemp)
				continue;
			if (pTemp->HPCurrent < lowHP && !IsCorpse(pTemp) && pTemp->Type != PET) {
				pGroupMember = pTemp;
				lowHP = pTemp->HPCurrent;
			}
		}
	}

	if (GetCharInfo()->pSpawn->HPCurrent < lowHP)
		pGroupMember = ((PSPAWNINFO)pChar);

	return pGroupMember;
}

PSPAWNINFO FindWorstInjuredInNetBots() {
	fNetBot findf = NULL;
	PMQPLUGIN pFind = pPlugins;
	while (pFind && stricmp(pFind->szFilename, "mq2netbots")) pFind = pFind->pNext;
	if (pFind) findf=(fNetBot)GetProcAddress(pFind->hModule, "GetLowestHPBot");
	if (findf == NULL)
		return 0;

	return ( (PSPAWNINFO)GetSpawnByID(findf()) );
}

//BEGIN CLASS DEFINITION\\

class MQ2HealType : public MQ2Type {
private:

public:
	enum HealMembers {
		WorstGroup=1,
		WorstNetBots=2,
		InjuredCount=3,
		UninjuredCount=4,
		InRangeCount=5
	};

	MQ2HealType():MQ2Type("Heal") {
		TypeMember(WorstGroup);
		TypeMember(WorstNetBots);
		TypeMember(InjuredCount);
		TypeMember(UninjuredCount);
		TypeMember(InRangeCount);
	}

	~MQ2HealType() {}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2HealType::FindMember(Member);
		if (!pMember)
			return false;
		PSPAWNINFO pSpawn = 0;
		int limit = 0;
		switch((HealMembers)pMember->ID) {
			case WorstGroup:
				pSpawn = FindWorstInjuredInGroup();
				if (pSpawn) {
					Dest.Type = pSpawnType;
					Dest.Ptr = pSpawn;
				} else {
					return false;
				}	
				return true;
			case WorstNetBots:
				pSpawn = FindWorstInjuredInNetBots();
				if (pSpawn) {
					Dest.Type = pSpawnType;
					Dest.Ptr = pSpawn;
				} else {
					return false;
				}				
				return true;
			case InjuredCount:
				limit = 100;
				if (IsNumber(Index))
					limit = atoi(Index);

				Dest.Type = pIntType;
				Dest.Int = CountGroupMembersBelow(limit);
				return true;
			case UninjuredCount:
				limit = 99;
				if (IsNumber(Index))
					limit = atoi(Index);

				Dest.Type = pIntType;
				Dest.Int = CountGroupMembersAbove(limit);
				return true;
			case InRangeCount:
				if (IsNumber(Index)) {
					Dest.Type = pIntType;
					Dest.Int = CountGroupMembersInRange(atoi(Index));
					return true;
				}
				return false;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

BOOL dataHeal(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pHealType;

	return true;
}

// END CLASS DEFINITION \\
// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2Heal");

	AddMQ2Data("Heal", dataHeal);
	pHealType = new MQ2HealType;	
	// Add commands, MQ2Data items, hooks, etc.
	// AddCommand("/mycommand",MyCommand);
	// AddXMLFile("MQUI_MyXMLFile.xml");
	// bmMyBenchmark=AddMQ2Benchmark("My Benchmark Name");
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Heal");
	RemoveMQ2Data("Heal");
	delete pHealType;
	// Remove commands, MQ2Data items, hooks, etc.
	// RemoveMQ2Benchmark(bmMyBenchmark);
	// RemoveCommand("/mycommand");
	// RemoveXMLFile("MQUI_MyXMLFile.xml");
}

// Called after entering a new zone
PLUGIN_API VOID OnZoned(VOID)
{
	DebugSpewAlways("MQ2Heal::OnZoned()");
}

// Called once directly before shutdown of the new ui system, and also
// every time the game calls CDisplay::CleanGameUI()
PLUGIN_API VOID OnCleanUI(VOID)
{
	DebugSpewAlways("MQ2Heal::OnCleanUI()");
	// destroy custom windows, etc
}

// Called once directly after the game ui is reloaded, after issuing /loadskin
PLUGIN_API VOID OnReloadUI(VOID)
{
	DebugSpewAlways("MQ2Heal::OnReloadUI()");
	// recreate custom windows, etc
}

// Called every frame that the "HUD" is drawn -- e.g. net status / packet loss bar
PLUGIN_API VOID OnDrawHUD(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
//	DebugSpewAlways("MQ2Heal::OnDrawHUD()");
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
	DebugSpewAlways("MQ2Heal::SetGameState()");
	// if (GameState==GAMESTATE_INGAME)
	// create custom windows if theyre not set up, etc
}


// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
//	DebugSpewAlways("MQ2Heal::OnPulse()");
}

// This is called every time WriteChatColor is called by MQ2Main or any plugin,
// IGNORING FILTERS, IF YOU NEED THEM MAKE SURE TO IMPLEMENT THEM. IF YOU DONT
// CALL CEverQuest::dsp_chat MAKE SURE TO IMPLEMENT EVENTS HERE (for chat plugins)
PLUGIN_API DWORD OnWriteChatColor(PCHAR Line, DWORD Color, DWORD Filter)
{
	DebugSpewAlways("MQ2Heal::OnWriteChatColor(%s)",Line);
	return 0;
}

// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
// but after MQ filters and chat events are taken care of.
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	DebugSpewAlways("MQ2Heal::OnIncomingChat(%s)",Line);
	return 0;
}

// This is called each time a spawn is added to a zone (inserted into EQ's list of spawns),
// or for each existing spawn when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn)
{
	DebugSpewAlways("MQ2Heal::OnAddSpawn(%s)",pNewSpawn->Name);
}

// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	DebugSpewAlways("MQ2Heal::OnRemoveSpawn(%s)",pSpawn->Name);
}

// This is called each time a ground item is added to a zone
// or for each existing ground item when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddGroundItem(PGROUNDITEM pNewGroundItem)
{
	DebugSpewAlways("MQ2Heal::OnAddGroundItem(%d)",pNewGroundItem->DropID);
}

// This is called each time a ground item is removed from a zone
// It is NOT called for each existing ground item when a plugin shuts down.
PLUGIN_API VOID OnRemoveGroundItem(PGROUNDITEM pGroundItem)
{
	DebugSpewAlways("MQ2Heal::OnRemoveGroundItem(%d)",pGroundItem->DropID);
}
