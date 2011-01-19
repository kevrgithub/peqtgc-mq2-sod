// MQ2Camp.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.



#include "../MQ2Plugin.h"


PreSetup("MQ2Camp");

int xLoc = 0;
int yLoc = 0;
int maxDis = 35;

void MakeCamp() {
	xLoc = ((PSPAWNINFO) pLocalPlayer)->X;
	yLoc = ((PSPAWNINFO) pLocalPlayer)->Y;
}

void MoveToCamp() {
	int xCur = ((PSPAWNINFO) pLocalPlayer)->X;
	int yCur = ((PSPAWNINFO) pLocalPlayer)->Y;
	if (GetDistance(xCur, yCur, xLoc, yLoc) >= 35) {

	} else if (GetDistance(xCur, yCur, xLoc, yLoc) <= 35) {

	}
}

//BEGIN MOVE FUNCTIONS\\
//stolen from MQ2MoveUtils
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
//END MOVE FUNCTIONS\\

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2Camp");

	// Add commands, MQ2Data items, hooks, etc.
	// AddCommand("/mycommand",MyCommand);
	// AddXMLFile("MQUI_MyXMLFile.xml");
	// bmMyBenchmark=AddMQ2Benchmark("My Benchmark Name");
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Camp");

	// Remove commands, MQ2Data items, hooks, etc.
	// RemoveMQ2Benchmark(bmMyBenchmark);
	// RemoveCommand("/mycommand");
	// RemoveXMLFile("MQUI_MyXMLFile.xml");
}

// Called after entering a new zone
PLUGIN_API VOID OnZoned(VOID)
{
	DebugSpewAlways("MQ2Camp::OnZoned()");
}

// Called once directly before shutdown of the new ui system, and also
// every time the game calls CDisplay::CleanGameUI()
PLUGIN_API VOID OnCleanUI(VOID)
{
	DebugSpewAlways("MQ2Camp::OnCleanUI()");
	// destroy custom windows, etc
}

// Called once directly after the game ui is reloaded, after issuing /loadskin
PLUGIN_API VOID OnReloadUI(VOID)
{
	DebugSpewAlways("MQ2Camp::OnReloadUI()");
	// recreate custom windows, etc
}

// Called every frame that the "HUD" is drawn -- e.g. net status / packet loss bar
PLUGIN_API VOID OnDrawHUD(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
//	DebugSpewAlways("MQ2Camp::OnDrawHUD()");
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
	DebugSpewAlways("MQ2Camp::SetGameState()");
	// if (GameState==GAMESTATE_INGAME)
	// create custom windows if theyre not set up, etc
}


// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
//	DebugSpewAlways("MQ2Camp::OnPulse()");
}

// This is called every time WriteChatColor is called by MQ2Main or any plugin,
// IGNORING FILTERS, IF YOU NEED THEM MAKE SURE TO IMPLEMENT THEM. IF YOU DONT
// CALL CEverQuest::dsp_chat MAKE SURE TO IMPLEMENT EVENTS HERE (for chat plugins)
PLUGIN_API DWORD OnWriteChatColor(PCHAR Line, DWORD Color, DWORD Filter)
{
	DebugSpewAlways("MQ2Camp::OnWriteChatColor(%s)",Line);
	return 0;
}

// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
// but after MQ filters and chat events are taken care of.
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	DebugSpewAlways("MQ2Camp::OnIncomingChat(%s)",Line);
	return 0;
}

// This is called each time a spawn is added to a zone (inserted into EQ's list of spawns),
// or for each existing spawn when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn)
{
	DebugSpewAlways("MQ2Camp::OnAddSpawn(%s)",pNewSpawn->Name);
}

// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	DebugSpewAlways("MQ2Camp::OnRemoveSpawn(%s)",pSpawn->Name);
}

// This is called each time a ground item is added to a zone
// or for each existing ground item when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddGroundItem(PGROUNDITEM pNewGroundItem)
{
	DebugSpewAlways("MQ2Camp::OnAddGroundItem(%d)",pNewGroundItem->DropID);
}

// This is called each time a ground item is removed from a zone
// It is NOT called for each existing ground item when a plugin shuts down.
PLUGIN_API VOID OnRemoveGroundItem(PGROUNDITEM pGroundItem)
{
	DebugSpewAlways("MQ2Camp::OnRemoveGroundItem(%d)",pGroundItem->DropID);
}
