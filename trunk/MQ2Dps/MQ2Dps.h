// MQ2Dps.h part of MQ2Dps plugin for MQ2
// Author: Shire

#include "time.h"

void Event_Dot (char* line);
void Event_Melee (char* line);
void Event_OtherMeleeDmg(char* line);
void Event_Cast (char* line);
void Event_MobDead (char* line);
void AddToList (char* name);
void WriteStats(long i, bool pet);
int IdInList (char* name);
void ResetGV (void);
void Sort (int opt);
void Usage(void);

int IntArrayMax(int a[], int size);
void WriteWindowINI(PCSIDLWND pWindow);
void ReadWindowINI(PCSIDLWND pWindow);

// Functions
char* GetPetNameFromCharName(char* Name);
PSPAWNINFO GetPetFromCharName(char* Name);
PSPAWNINFO GetCharFromPetName(char* Name);
PSPAWNINFO GetMaster(DWORD PLAYERID);
bool IsNpc(char* Name);

// Commands
void CmdDps(PSPAWNINFO pChar, PCHAR szLine);
// DataType
int _paramDps (int id);
BOOL dataDps(PCHAR szName, MQ2TYPEVAR &Ret);

// Defines
#define MAX_CHARS 2048 // Edit this if there is more than 50 people.
#define debug 0
#define ver "v0.9.95"

#define NO_SORT      0
#define MELEE_DMG   1
#define CAST_DMG   2
#define TOTAL_DMG   3
#define DPS         4

// Global variables
bool fight = 0;
char _name[MAX_CHARS][MAX_STRING];
long _meleedmg[MAX_CHARS];
long _castdmg[MAX_CHARS];
time_t _timeI[MAX_CHARS];
time_t _timeF[MAX_CHARS];
bool _fight[MAX_CHARS];
bool _isnpc [MAX_CHARS];
bool _written[MAX_CHARS];

long _mobtotaldmg = 0;
long _mobtotalcast = 0;

bool _ON = 1;
int _playerid = -1;
int _petid = -1;
bool _logonshowwindow = 1;
int _sort;
bool _petowner;
bool _absdps;
bool _focusdps;
char strFocusName[MAX_CHARS];