//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2Casttimer.cpp
// Author: A_Enchanter_00
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//

#define       DEBUGGING         0

#ifndef PLUGIN_API
  #include "../MQ2Plugin.h"
  #include "../Blech/Blech.h"
  PreSetup("MQ2CastTimer");
  PLUGIN_VERSION(8.1116);
#endif

#ifndef   NUM_INV_SLOTS
  #define NUM_INV_SLOTS 31
#endif    NUM_INV_SLOTS

#ifndef   BAG_SLOT_START
  #define BAG_SLOT_START 23
#endif    BAG_SLOT_START
 
#define       DELAY_CAST    12000
#define       DELAY_MEMO     6000
#define       DELAY_STOP     3000
 
#define       CAST_SUCCESS      0
#define       CAST_INTERRUPTED  1
#define       CAST_RESIST       2
#define       CAST_COLLAPSE     3
#define       CAST_RECOVER      4
#define       CAST_FIZZLE       5
#define       CAST_STANDING     6
#define       CAST_STUNNED      7
#define       CAST_INVISIBLE    8
#define       CAST_NOTREADY     9
#define       CAST_OUTOFMANA   10
#define       CAST_OUTOFRANGE  11
#define       CAST_NOTARGET    12
#define       CAST_CANNOTSEE   13
#define       CAST_COMPONENTS  14
#define       CAST_OUTDOORS    15
#define       CAST_TAKEHOLD    16
#define       CAST_IMMUNE      17
#define       CAST_DISTRACTED  18
#define       CAST_ABORTED     19
#define       CAST_UNKNOWN     20
 
#define       FLAG_COMPLETE     0 
#define       FLAG_REQUEST     -1
#define       FLAG_PROGRESS1   -2 
#define       FLAG_PROGRESS2   -3 
#define       FLAG_PROGRESS3   -4 
#define       FLAG_PROGRESS4   -5 
 
#define       DONE_COMPLETE    -3
#define       DONE_ABORTED     -2 
#define       DONE_PROGRESS    -1 
#define       DONE_SUCCESS      0
 
#define       TYPE_SPELL        1
#define       TYPE_ALT          2
#define       TYPE_ITEM         3
 
#define       RECAST_DEAD       2
#define       RECAST_LAND       1
#define       RECAST_ZERO       0
 
#define       NOID             -1
 
long          CastingC=NOID;           // Casting Current ID
long          CastingO=NOID;              // Casting OnTarget

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
//MQ2CastTimer
//Updates by A_Enchanter_00 Date: 20060725
//Original constructed from MQ2Timer converted to add on to MQ2Cast

#include <vector>
#include <list>

#define MQ2BLECH 1              // must set to 0 if using vc6.0, But i recoment using vc 2005 express whitch is free

#define SPELLDURATSION 4
#define MAXSPELLLEVEL MAX_PC_LEVEL
#define EVENTSLENGTH 75
#define SPAWNSEARCHLOOP 100
#define TARGET_SEARCH_ZRADIUS 40.0f
#define TARGET_SEARCH_RADIUS 200.0f
#define SPELLTIMER_SEARCH_ZRADIUS 100.0f
#define SPELLTIMER_SEARCH_RADIUS 200.0f
#define CASTERANY 0
#define CASTERME 1
#define CASTEROTHER 2
#define CASTEREVENT 3

VOID MQTimerCommand(PSPAWNINFO pChar, PCHAR szLine);
VOID CreateTimerWindow();
VOID DestroyTimerWindow();
VOID LoadCostumEvents();
VOID ParseSpellsOnce();
PCONTENTS ItemWithSpell(long SpellID);
/*
        CastTimer Function's Defenision
*/
VOID ClearAll();
VOID ClearTimer(long SpawnID,eSpawnType SpawnType);
VOID AddSpell(int TargetID,int SpellID,int TickE);
VOID AddSpawnTimer(int SpawnID);

bool VerifyTarget(long SpawnID,PSPELL pSpell);
VOID AddSpellTimer(int SpawnID,PSPELL pSpell,int TickE,bool ignoreMod = false);
int CalDuratsion(PSPELL pSpell,bool ignoreMod = false);
//PSPELL BestDuratsionFocus(PSPELL pSpell);
float BestDuratsionFocus(PSPELL pSpell);

void __stdcall TimerCustomEvent(unsigned int ID, void *pData, PBLECHVALUE pValues);

CHAR Buffer[MAX_STRING] = {0};                                  // Buffer for String manipulatsion

bool isParsed=false;                                                    // "END" the plugin when a error hapen's

long OldSpawnID=0;                                                              // Me Old Spawn ID

time_t lastRun = 0;
long lastTargetID = 0;                                                  // ID of curent Last Target
long currentTargetID = 0;                                               // ID of curent Target
long TimerCount = 0;                                                    // Timer ID Count

Blech                   CASTONANOTHER('#');                     // BTree List for OnChat CASTONANOTHER
Blech                   CASTWORNOFF('#');                       // BTree List for OnChat CASTWORNOFF
Blech                   CUSTOMEVENT('#');                       // BTree List for OnChat CUSTOMEVENT
Blech                   SUCCESS('#');                           // BTree List for OnChat Message on SUCCESS Detection

bool BardClass() {
  return (strncmp(pEverQuest->GetClassDesc(GetCharInfo2()->Class & 0xFF),"Bard",5))?false:true;
}
 
long CastingLeft() {
  long CL=0;
  if(pCastingWnd && (PCSIDLWND)pCastingWnd->Show) {
    CL=GetCharInfo()->pSpawn->CastingData.SpellETA - GetCharInfo()->pSpawn->TimeStamp;
    if(CL<1) CL=1;
  }
  return CL;
}
 
void Execute(PCHAR zFormat, ...) {
  char zOutput[MAX_STRING]={0}; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList); if(!zOutput[0]) return;
  DoCommand(GetCharInfo()->pSpawn,zOutput);
}
 
long GEMID(DWORD ID) {
  for(int GEM=0; GEM < NUM_SPELL_GEMS; GEM++) if(GetCharInfo2()->MemorizedSpells[GEM] == ID) return GEM;
  return NOID;
}
 
bool GEMReady(DWORD ID) {
  if(pCastSpellWnd && ID < NUM_SPELL_GEMS)
    if((long)((PEQCASTSPELLWINDOW)pCastSpellWnd)->SpellSlots[ID]->spellicon!=NOID)
      if(BardClass() || (long)((PEQCASTSPELLWINDOW)pCastSpellWnd)->SpellSlots[ID]->spellstate!=1)
        return true;
  return false;
}

long SpellTimer(long Type, void *Data) {
  int Ready;
  switch(Type) {
    case TYPE_SPELL:
      if(GEMReady(GEMID(((PSPELL)Data)->ID))) return 0;
      return (long)((PSPELL)Data)->FizzleTime;
    case TYPE_ALT:
      if(pAltAdvManager->GetCalculatedTimer(pPCData,(PALTABILITY)Data)>0) {
        pAltAdvManager->IsAbilityReady(pPCData,(PALTABILITY)Data,&Ready);
        return (Ready<1)?0:Ready*1000;
      }
      return 999999;
    case TYPE_ITEM:
      return GetItemTimer((PCONTENTS)Data)*1000;
  }
  return 999999;
}

// Timer Structure
struct TimerSingle {
        long TimerID;
        PSPELL pSpell;
        long EventID;
        int CastE;
        long SpawnID;
        time_t timestamp;
        int Caster;
        bool AutoRecast;
        char SpawnName[64];
        time_t TimeOut;
        WORD SpawnWatch;
        float SpawnWatchX;
        float SpawnWatchY;
        float SpawnWatchZ;
} pTimerList;

// Spell Event Structure
struct SpellEvent {
        PSPELL pHSpell;                                                         // Higest spell ID
        vector<PSPELL> pSpells;                                         // All Matching spell ID'S
        char Event[EVENTSLENGTH];
} pSpellEventList;

struct EventTimer {
        char Name[MAX_STRING];
//      int IsSpell;
        char Event[MAX_STRING];
        int Duration;
        int SpawnType;
} pEventList;

list<TimerSingle>    TimerList;          // timer list
vector<SpellEvent>  SpellEventList;
vector<EventTimer>  EventList;
vector<unsigned long>  BleachList;
vector<long> XmlList;

VOID ClearOne(list<TimerSingle>::iterator &CurList);
VOID DefaultTimer(TimerSingle &MyData);
VOID SetTimerData(TimerSingle &MyData,list<TimerSingle>::iterator &CurList);
long getSpellTimer(long SpawnID, int Caster,vector<PSPELL> SpellSearch);
eSpawnType SpawnType(PSPAWNINFO tSpawn);
VOID RestackTimer(list<TimerSingle>::iterator &CurList,TimerSingle MyData,int Remove);

class CCastTimerWnd : public CCustomWnd {
public:
        CTabWnd *TabPages;
        CListWnd *TimerListBox;
        CButtonWnd *StopButton;
        CButtonWnd *WatchSpawnButton;
        CComboWnd *SelectActorListLeft;
        CComboWnd *SelectActorListRight;

        CCheckBoxWnd *TargetCheckBox;
        CCheckBoxWnd *NpcCheckBox;
        CCheckBoxWnd *PcCheckBox;
        CCheckBoxWnd *PetCheckBox;
        CCheckBoxWnd *MercCheckBox;
        CCheckBoxWnd *EventCheckBox;
        CCheckBoxWnd *MySpellCheckBox;
        CCheckBoxWnd *OtherSpellCheckBox;
        CCheckBoxWnd *ManualRecastCheckBox;
        CCheckBoxWnd *InGroupCheckBox;

        unsigned int TagsSize;          // used for Width of Tags Columns
        unsigned int NameSize;          // used for Width of Name Columns
        char DefaultGem[5];                     // DefaulyGem for Recasting
        int AutoRecast;
        int UpdateState;
                //  0=thread terminate
                //  1=update
                //  2=no update(sleep)
        char szSettingINISection[MAX_STRING];

        CCastTimerWnd():CCustomWnd("CastTimerWnd") {
                TabPages=(CTabWnd*)GetChildItem("CTW_Pages");
                TimerListBox=(CListWnd*)GetChildItem("CTW_TimerList");
                StopButton=(CButtonWnd*)GetChildItem("CTW_StopButton");
                WatchSpawnButton=(CButtonWnd*)GetChildItem("CTW_WatchSpawnButton");
                SelectActorListLeft=(CComboWnd*)GetChildItem("CTW_SelectActorBoxLeft");
                SelectActorListRight=(CComboWnd*)GetChildItem("CTW_SelectActorBoxRight");
                TargetCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowTargetCheckbox");
                NpcCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowNPCCheckbox");
                PcCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowPCCheckbox");
                PetCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowPETCheckbox");
                MercCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowMERCCheckbox");
                EventCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowEVENTCheckbox");
                MySpellCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowMYCheckbox");
                OtherSpellCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_ShowOTHERCheckbox");
                ManualRecastCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_MRECASTCheckbox");
                InGroupCheckBox=(CCheckBoxWnd*)GetChildItem("CTW_InGROUPCheckbox");

                // Add Target/TargetNAB/Recast to SelectActorListLeft combo box
                SelectActorListLeft->InsertChoice("Target");
                SelectActorListLeft->InsertChoice("TargetNAB");
                SelectActorListLeft->InsertChoice("Recast");
                SelectActorListLeft->InsertChoice("Clear");
                SelectActorListLeft->InsertChoice("Add: Recast");
                SelectActorListLeft->InsertChoice("Remove: Recast");
                SelectActorListLeft->InsertChoice("Toggle: Recast");
                // Add Target/TargetNAB/Recast to SelectActorListRight combo box
                SelectActorListRight->InsertChoice("Target");
                SelectActorListRight->InsertChoice("TargetNAB");
                SelectActorListRight->InsertChoice("Recast");
                SelectActorListRight->InsertChoice("Clear");
                SelectActorListRight->InsertChoice("Add: Recast");
                SelectActorListRight->InsertChoice("Remove: Recast");
                SelectActorListRight->InsertChoice("Toggle: Recast");

                SetWndNotification(CCastTimerWnd);
                BitOff(WindowStyle,CWS_CLOSE);
                sprintf(szSettingINISection,"Settings.%s.%s",EQADDR_SERVERNAME,GetCharInfo()->pSpawn->Name);
                ReadWindowINI();
                CloseOnESC = false; // this disables the close on escape
        }

        ~CCastTimerWnd() {
//              WriteWindowINI();
        }
        /*
        // Window Notifyer, to interact with xml window
        */
        int WndNotification(CXWnd *pWnd, unsigned int Message, void *unknown) {
                if (pWnd==(CXWnd*)WatchSpawnButton) {
                        if (Message==XWM_LCLICK) {
                                int cTarget=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;
                                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(cTarget)) {
                                        if (pSpawn->Type == SPAWN_NPC) AddSpawnTimer(pSpawn->SpawnID);
                                }
                        }
                        else DebugSpew("WatchSpawnButton message %Xh / %d",Message,Message);
                } else if (pWnd==(CXWnd*)StopButton) {
                        if (Message==XWM_LCLICK) {
                                if (UpdateState==1) {
                                        SetCXStr(&StopButton->WindowText,"Update");
                                        SetCXStr(&StopButton->Tooltip,"Click to Update");
                                        UpdateState=2;
                                }
                                else {
                                        SetCXStr(&StopButton->WindowText,"Pause");
                                        SetCXStr(&StopButton->Tooltip,"Click to Pause");
                                        UpdateState=1;
                                }
                        }
                        else DebugSpew("StopButton message %Xh / %d",Message,Message);
                }
                else if (pWnd==(CXWnd*)TimerListBox) {
                        if (Message==XWM_LCLICK) {
                                if(SelectActorListLeft->GetCurChoice() == 3) ClearTimerByListID(TimerListBox->GetCurSel());
                                else TimerByLoc(TimerListBox->GetCurSel(),SelectActorListLeft->GetCurChoice());
                        }
                        else if (Message==XWM_RCLICK) {
                                if(SelectActorListRight->GetCurChoice() == 3) ClearTimerByListID(TimerListBox->GetCurSel());
                                else TimerByLoc(TimerListBox->GetCurSel(),SelectActorListRight->GetCurChoice());                                
                        }
                        TimerListBox->ClearAllSel();
                }
                else if (pWnd==0) {
                        if (Message==XWM_CLOSE) {
                                Show=1;
                                return 1;
                        }
                }
                return CSidlScreenWnd::WndNotification(pWnd,Message,unknown);
        }
        /*
        //      Clear's Xml Window list and updates the window with updated timer's ( 1/sec )
        //      Can show by Target/NPC/PC/PET/MERC/ALL and can sort timers by low or no sorting
        */
        VOID UpdateList(int myTarget,bool force = false) {
                if (!force && UpdateState != 1) return;

                time_t now = time(NULL);
                int DisplayTimerCount = 0;
                eSpawnType pSpawnType;
                ClearList();
                list<TimerSingle>::iterator CurList=TimerList.begin();
                list<TimerSingle>::iterator EndList=TimerList.end(); 

                int TotalCount=0;

                while(CurList!=EndList) {
                        if ( CurList->SpawnWatch ) {
                                WriteSpawnWatchToXml(CurList, DisplayTimerCount);
                        } else {
                                if (CurList->SpawnID==NOID) break;
                                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID)) {
                                        //pSpawnType = GetSpawnType(pSpawn); // Changed to SpawnType() to make npc pet's same as NPC so that pet filter is only for PC pet's
                                        pSpawnType = SpawnType(pSpawn);
                                        if(CurList->EventID != NOID || CurList->Caster == CASTEREVENT) {
                                                if(EventCheckBox->Checked)
                                                        WriteEventToXml(CurList->EventID,pSpawn,difftime(CurList->timestamp, now), DisplayTimerCount,CurList->TimerID);
                                        }
                                        else if(CurList->pSpell && CurList->pSpell->ID) {                                               
                                                if((MySpellCheckBox->Checked && CurList->Caster == CASTERME) || (OtherSpellCheckBox->Checked && CurList->Caster == CASTEROTHER)) {
                                                        if((TargetCheckBox->Checked && pSpawn->SpawnID == myTarget) || (NpcCheckBox->Checked && pSpawnType == NPC) || (PetCheckBox->Checked && pSpawnType == PET)) {
                                                                WriteSpellToXml(CurList->pSpell,pSpawn,CurList->CastE, difftime(CurList->timestamp, now),DisplayTimerCount,CurList->TimerID,CurList->AutoRecast);
                                                        } else if ( InGroupCheckBox->Checked && ( pSpawnType == PC || pSpawnType == MERCENARY ) ) {
                                                                if (IsGroupMember(pSpawn) || pSpawn->SpawnID == GetCharInfo()->pSpawn->SpawnID) WriteSpellToXml(CurList->pSpell,pSpawn,CurList->CastE, difftime(CurList->timestamp, now),DisplayTimerCount,CurList->TimerID,CurList->AutoRecast);
                                                        } else if ( PcCheckBox->Checked && pSpawnType == PC ) {
                                                                WriteSpellToXml(CurList->pSpell,pSpawn,CurList->CastE, difftime(CurList->timestamp, now),DisplayTimerCount,CurList->TimerID,CurList->AutoRecast);
                                                        } else if ( MercCheckBox->Checked && pSpawnType == MERCENARY ) {
                                                                WriteSpellToXml(CurList->pSpell,pSpawn,CurList->CastE, difftime(CurList->timestamp, now),DisplayTimerCount,CurList->TimerID,CurList->AutoRecast);
                                                        }
                                                }
                                        }
                                        TotalCount++;
                                }
                        }
                        CurList++;
                }               
                sprintf(Buffer, "Timers: Total: %d TotalCount: %d Showing: %d",TimerList.size(),TotalCount, DisplayTimerCount);
                SetCXStr(&this->WindowText,Buffer);
        }

        /*
        //      Insert Time 00:00:00, Tags, Spell Name, Spawn Name in to line in the
        //      Xml Window List.
        */
        VOID WriteSpellToXml(PSPELL pSpell,PSPAWNINFO pSpawn,int CastE, double timeremain, int &DisplayTimerCount, long TimerID, bool AutoRecast) {
                XmlList.push_back(TimerID);
                long CurID=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;
                DWORD myColor;
                double timeMult = 0;
                int i = 0;

                int remain_sec = (int) timeremain;
                int remain_hou = int((remain_sec/(60 * 60)));
                int remain_min = int((remain_sec%(60 * 60)/60));
                remain_sec = int((remain_sec%60));

                char Addons[30] = {0};
                char TimeLeft[20] = {0};

                if(pSpawn->SpawnID==CurID) strcat(Addons,"T");

                if(AutoRecast) strcat(Addons,"AR");

                if( GetCharInfo()->pSpawn->GroupAssistNPC[0] && GetCharInfo()->pSpawn->GroupAssistNPC[0]==pSpawn->SpawnID) strcat(Addons,"GA");
                else {
                        for(i=0;i<3;i++) {
                                if(GetCharInfo()->pSpawn->RaidAssistNPC[i] && GetCharInfo()->pSpawn->RaidAssistNPC[i]==pSpawn->SpawnID) {
                                        sprintf(Buffer, "RA%d", i+1);
                                        strcat(Addons,Buffer);
                                        break;
                                }
                        }
                }

                for(i=0;i<3;i++) {
                        if( GetCharInfo()->pSpawn->RaidMarkNPC[i] && GetCharInfo()->pSpawn->RaidMarkNPC[i]==pSpawn->SpawnID) {
                                sprintf(Buffer, "R%d", i+1);
                                strcat(Addons,Buffer);
                                break;
                        } else if( GetCharInfo()->pSpawn->GroupMarkNPC[i] && GetCharInfo()->pSpawn->GroupMarkNPC[i]==pSpawn->SpawnID) {
                                sprintf(Buffer, "G%d", i+1);
                                strcat(Addons,Buffer);
                                break;
                        }
                }

                sprintf(TimeLeft, "%02d:%02d:%02d", remain_hou, remain_min, remain_sec);

                if(CastE==CAST_SUCCESS) {                                       // CAST_SUCCESS
                        timeMult = ((GetSpellDuration(pSpell,(PSPAWNINFO)pCharSpawn)*6)/16);
                        if (!timeMult) timeMult = 5;
                        if (timeremain < 6) myColor = 0xFFFF0000;       //red
                        else if (timeremain<timeMult*4) myColor = 0xFFFFFF00;   //yellow
                        else if (timeremain<timeMult*8) myColor = 0xFF00FFFF;   //teal
                        else myColor = 0xFF00FF00;      //green
                }
                else if(CastE==CAST_IMMUNE) {                           // CAST_IMMUNE
                        myColor = 0xFFFF0000;
                        strcat(Addons,"I");
                }
                else if(CastE==CAST_TAKEHOLD) {                         // CAST_TAKEHOLD
                        myColor = 0xFFFFFF00;                   
                        strcat(Addons,"N");
                }
                TimerListBox->AddString(&CXStr(""), myColor, 0, 0 );

                if (TagsSize < strlen(Addons)*6) {
                        TagsSize = strlen(Addons)*6;
                        if (TagsSize && TagsSize < 30) TimerListBox->SetColumnWidth(1,30);
                        else if (TagsSize) TimerListBox->SetColumnWidth(1,TagsSize);
                }
                if (NameSize < strlen(pSpell->Name)*6) {
                        NameSize = strlen(pSpell->Name)*6;
                        if (NameSize && NameSize < 60) TimerListBox->SetColumnWidth(2,60);
                        else if (NameSize) TimerListBox->SetColumnWidth(2,NameSize);
                }

                TimerListBox->SetItemText(DisplayTimerCount,0, &CXStr(TimeLeft));
                TimerListBox->SetItemColor(DisplayTimerCount,0,myColor);

                TimerListBox->SetItemText(DisplayTimerCount,1, &CXStr(Addons));
                TimerListBox->SetItemColor(DisplayTimerCount,1,myColor);

                TimerListBox->SetItemText(DisplayTimerCount,2, &CXStr(pSpell->Name));
                TimerListBox->SetItemColor(DisplayTimerCount,2,myColor);

                TimerListBox->SetItemText(DisplayTimerCount,3, &CXStr(pSpawn->Name));
                TimerListBox->SetItemColor(DisplayTimerCount,3,myColor);
                DisplayTimerCount++;
        }

        /*
        //      Insert Time 00:00:00 Event Name
        //      Xml Window List.
        */
        VOID WriteEventToXml(int EventNum,PSPAWNINFO pSpawn, double timeremain, int &DisplayTimerCount, long TimerID) {
                XmlList.push_back(TimerID);
                int remain_sec = (int) timeremain;
                int remain_hou = int((remain_sec/(60 * 60)));
                int remain_min = int((remain_sec%(60 * 60)/60));
                remain_sec = int((remain_sec%60));

                char Addons[30] = {0};
                strcat(Addons,"E");
                char TimeLeft[20] = {0};
                sprintf(TimeLeft, "%02d:%02d:%02d", remain_hou, remain_min, remain_sec);

                TimerListBox->AddString(&CXStr(""), 0xFFFF0000, 0, 0 );

                TimerListBox->SetItemText(DisplayTimerCount,0, &CXStr(TimeLeft));
                TimerListBox->SetItemColor(DisplayTimerCount,0,0xFFFF0000);

                if (TagsSize < strlen(Addons)*6) {
                        TagsSize = strlen(Addons)*6;
                        if (TagsSize && TagsSize < 30) TimerListBox->SetColumnWidth(1,30);
                        else if (TagsSize) TimerListBox->SetColumnWidth(1,TagsSize);
                }
                if (NameSize < strlen(EventList[EventNum].Name)*6) {
                        NameSize = strlen(EventList[EventNum].Name)*6;
                        if (NameSize && NameSize < 60) TimerListBox->SetColumnWidth(2,60);
                        else if (NameSize) TimerListBox->SetColumnWidth(2,NameSize);
                }
                TimerListBox->SetItemText(DisplayTimerCount,1, &CXStr(Addons));
                TimerListBox->SetItemColor(DisplayTimerCount,1,0xFFFF0000);

                TimerListBox->SetItemText(DisplayTimerCount,2, &CXStr(EventList[EventNum].Name));
                TimerListBox->SetItemColor(DisplayTimerCount,2,0xFFFF0000);

                if(pSpawn->SpawnID==GetCharInfo()->pSpawn->SpawnID) TimerListBox->SetItemText(DisplayTimerCount,3, &CXStr(""));
                else TimerListBox->SetItemText(DisplayTimerCount,3, &CXStr(pSpawn->Name));
                TimerListBox->SetItemColor(DisplayTimerCount,3,0xFFFF0000);
                DisplayTimerCount++;
        }

        /*
        //      Insert Time 00:00:00 Event Name
        //      Xml Window List.
        */
        VOID WriteSpawnWatchToXml(list<TimerSingle>::iterator &CurList, int &DisplayTimerCount) {
                XmlList.push_back(CurList->TimerID);
                time_t now = time(NULL);
                DWORD myColor = 0;
                long CurID=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;

                int remain_sec = 0;
                if ( CurList->timestamp > now ) {
                        remain_sec = (int) difftime(CurList->timestamp, now);

                        if (remain_sec < 60) myColor = 0xFFFF0000;      //red
                        else if (remain_sec<300) myColor = 0xFFFFFF00;  //yellow
                        else if (remain_sec<600) myColor = 0xFF00FFFF;  //teal
                        else myColor = 0xFF00FF00;      //green
                } else {
                        remain_sec = (int) difftime(now, CurList->timestamp);
                        myColor = 0xFFFF0000;
                }


                int remain_hou = int((remain_sec/(60 * 60)));
                int remain_min = int((remain_sec%(60 * 60)/60));
                remain_sec = int((remain_sec%60));

                char Addons[30] = {0};
                char TimeLeft[20] = {0};
                sprintf(TimeLeft, "%02d:%02d:%02d", remain_hou, remain_min, remain_sec);
                TimerListBox->AddString(&CXStr(""), 0xFFFF0000, 0, 0 );

                PSPAWNINFO pSpawn;
                if ( CurList->SpawnWatch == GetCharInfo()->zoneId ) pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID);

                int i = 0;

                if ( pSpawn ) {
                        if( pSpawn->SpawnID==CurID) strcat(Addons,"T");

                        if( GetCharInfo()->pSpawn->GroupAssistNPC[0] && GetCharInfo()->pSpawn->GroupAssistNPC[0]==pSpawn->SpawnID) strcat(Addons,"GA");
                        else {
                                for(i=0;i<3;i++) {
                                        if(GetCharInfo()->pSpawn->RaidAssistNPC[i] && GetCharInfo()->pSpawn->RaidAssistNPC[i]==pSpawn->SpawnID) {
                                                sprintf(Buffer, "RA%d", i+1);
                                                strcat(Addons,Buffer);
                                                break;
                                        }
                                }
                        }

                        for(i=0;i<3;i++) {
                                if( GetCharInfo()->pSpawn->RaidMarkNPC[i] && GetCharInfo()->pSpawn->RaidMarkNPC[i]==pSpawn->SpawnID) {
                                        sprintf(Buffer, "R%d", i+1);
                                        strcat(Addons,Buffer);
                                        break;
                                } else if( GetCharInfo()->pSpawn->GroupMarkNPC[i] && GetCharInfo()->pSpawn->GroupMarkNPC[i]==pSpawn->SpawnID) {
                                        sprintf(Buffer, "G%d", i+1);
                                        strcat(Addons,Buffer);
                                        break;
                                }
                        }
                }

                char SpawnTimerTitle[164] = {0};

                if ( CurList->SpawnName[0] ) sprintf(SpawnTimerTitle, "Spawn timer: %d, %s.", CurList->TimerID ,CurList->SpawnName);
                else sprintf(SpawnTimerTitle, "Spawn timer: %d (X:%.2f Y:%.2f Z:%.2f).",CurList->TimerID,CurList->SpawnWatchX,CurList->SpawnWatchY,CurList->SpawnWatchZ);

                if( pSpawn && pSpawn->Type != SPAWN_CORPSE ) {
                        TimerListBox->SetItemText(DisplayTimerCount, 0, &CXStr("00:00:00"));
                        TimerListBox->SetItemColor(DisplayTimerCount, 0, 0xFFFF0000);

                        strcat(Addons,"WA");
                        TimerListBox->SetItemText(DisplayTimerCount, 1, &CXStr(Addons));
                        TimerListBox->SetItemColor(DisplayTimerCount,1,  0xFFFF0000);

                        TimerListBox->SetItemText(DisplayTimerCount, 2, &CXStr(SpawnTimerTitle));
                        TimerListBox->SetItemColor(DisplayTimerCount, 2, 0xFFFF0000);

                        TimerListBox->SetItemText(DisplayTimerCount, 3, &CXStr(pSpawn->Name));
                        TimerListBox->SetItemColor(DisplayTimerCount, 3, 0xFFFF0000);
                } else {
                        TimerListBox->SetItemText(DisplayTimerCount, 0, &CXStr(TimeLeft));
                        TimerListBox->SetItemColor(DisplayTimerCount, 0, myColor);

                        strcat(Addons,"WD");
                        TimerListBox->SetItemText(DisplayTimerCount, 1, &CXStr(Addons));
                        TimerListBox->SetItemColor(DisplayTimerCount, 1, myColor);

                        TimerListBox->SetItemText(DisplayTimerCount,2, &CXStr(SpawnTimerTitle));
                        TimerListBox->SetItemColor(DisplayTimerCount, 2,myColor);

                        TimerListBox->SetItemText(DisplayTimerCount, 3, &CXStr("DEAD"));
                        TimerListBox->SetItemColor(DisplayTimerCount, 3, myColor);
                }

                if (TagsSize < strlen(Addons)*6) {
                        TagsSize = strlen(Addons)*6;
                        if (TagsSize && TagsSize < 30) TimerListBox->SetColumnWidth(1,30);
                        else if (TagsSize) TimerListBox->SetColumnWidth(1,TagsSize);
                }
                if (NameSize < strlen(SpawnTimerTitle)*6) {
                        NameSize = strlen(SpawnTimerTitle)*6;
                        if (NameSize && NameSize < 60) TimerListBox->SetColumnWidth(2,60);
                        else if (NameSize) TimerListBox->SetColumnWidth(2,NameSize);
                }
                DisplayTimerCount++;
        }

        /*
        // Clear Xml Window ( list )
        */
        VOID ClearList() {
                TagsSize = 0;
                NameSize = 0;
                TimerListBox->DeleteAll();
                TimerListBox->SetColumnWidth(1,0);
                TimerListBox->SetColumnWidth(2,60);
                XmlList.clear();
        }

        /*
        // Clear timer by list posission
        */
        VOID ClearTimerByListID(int ID) {
                if(!XmlList.size() || ID >= (long)XmlList.size()) return;
                long cTargetID=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;
                list<TimerSingle>::iterator CurList=TimerList.begin();
                list<TimerSingle>::iterator EndList=TimerList.end(); 
                while(CurList!=EndList) {
                        if (CurList->TimerID == XmlList[ID]) {
                                ClearOne(CurList);
                                UpdateList(cTargetID,true);
                                break;
                        }
                        CurList++;
                }
                
        }

        /*
        //      Target the closest NPC that has NOT been affected by this spell, other than <<exclude>>
        //      - good for picking out mobs needing to be mezzed in a pack
        //      - suggestion: assist the MT before calling this to target only the mobs that need mezzing
        //      If it is a Beneficial spell it target's nearest PC/MERC without the spell on it ( left click )
        */
        VOID TimerByLoc(int ID, int Act) {
                if(!XmlList.size() || ID >= (long)XmlList.size()) return;
                PSPAWNINFO sSpawn;
                PSPAWNINFO pSpawn;
                eSpawnType tmpSpawnType = NPC;
                bool flag = false;
                CHAR szLine[MAX_STRING] = {0};

        
                list<TimerSingle>::iterator CurList=TimerList.begin();
                list<TimerSingle>::iterator EndList=TimerList.end(); 

                list<TimerSingle>::iterator CurList2=TimerList.begin();
                list<TimerSingle>::iterator EndList2=TimerList.end(); 

                while(CurList!=EndList) {
                        if (CurList->SpawnID && CurList->TimerID == XmlList[ID] && ( CurList->EventID == NOID || CurList->SpawnID != GetCharInfo()->pSpawn->SpawnID )) {
                                if(Act == 0 || CurList->EventID != NOID)
                                        *(PSPAWNINFO*)ppTarget = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID);
                                else if(Act == 1 && CurList->pSpell && CurList->pSpell->ID) {
                                        if(CurList->pSpell->TargetType == 6 || CurList->pSpell->TargetType == 14) return;

                                        pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID);
                                        if(!pSpawn->SpawnID) return;

                                        switch(GetSpawnType(pSpawn)) {
                                                case PC: // PC
                                                        tmpSpawnType = PC;
                                                        break;
                                                case NPC: // NPC
                                                        tmpSpawnType = NPC;
                                                        break;
                                                case MERCENARY: // MERCENARY (as PC, then check merc after)
                                                        tmpSpawnType = PC;
                                                        break;
                                                default: // Unknown
                                                        return;
                                        }

                                        SEARCHSPAWN ssSpawn;
                                        ClearSearchSpawn(&ssSpawn);             
                                        ssSpawn.ZRadius=TARGET_SEARCH_ZRADIUS;
                                        ssSpawn.FRadius=TARGET_SEARCH_RADIUS;
                                        ssSpawn.bLoS=true;
                                        ssSpawn.SpawnType = tmpSpawnType;
                                        for (int i=0; i<SPAWNSEARCHLOOP; i++) {
                                                sSpawn = NthNearestSpawn(&ssSpawn,i,(PSPAWNINFO)pCharSpawn);
                                                if ( (sSpawn && sSpawn->SpawnID) && (!(CurList->SpawnID == sSpawn->SpawnID)) ) {
                                                        flag = false;
                                                        while(CurList2!=EndList2) {
                                                                if (CurList2->SpawnID && CurList2->SpawnID == sSpawn->SpawnID) {
                                                                        if (CurList2->pSpell && CurList2->pSpell->ID == CurList->pSpell->ID) {
                                                                                flag = true;
                                                                                break;
                                                                        }
                                                                }
                                                                CurList2++;
                                                        }
                                                        if (!flag) {
                                                                *(PSPAWNINFO*)ppTarget = (PSPAWNINFO)GetSpawnByID(sSpawn->SpawnID);
                                                                return;
                                                        }
                                                }
                                        }
                                        if ( tmpSpawnType == PC ) {
                                                SEARCHSPAWN ssSpawn;
                                                ClearSearchSpawn(&ssSpawn);             
                                                ssSpawn.ZRadius=TARGET_SEARCH_ZRADIUS;
                                                ssSpawn.FRadius=TARGET_SEARCH_RADIUS;
                                                ssSpawn.bLoS=true;
                                                ssSpawn.SpawnType = MERCENARY;
                                                for (int i=0; i<SPAWNSEARCHLOOP; i++) {
                                                        sSpawn = NthNearestSpawn(&ssSpawn,i,(PSPAWNINFO)pCharSpawn);
                                                        if ( (sSpawn && sSpawn->SpawnID) && (!(CurList->SpawnID == sSpawn->SpawnID)) ) {
                                                                flag = false;
                                                                while(CurList2!=EndList2) {
                                                                        if (CurList2->SpawnID && CurList2->SpawnID == sSpawn->SpawnID) {
                                                                                if (CurList2->pSpell && CurList2->pSpell->ID == CurList->pSpell->ID) {
                                                                                        flag = true;
                                                                                        break;
                                                                                }
                                                                        }
                                                                        CurList2++;
                                                                }
                                                                if (!flag) {
                                                                        *(PSPAWNINFO*)ppTarget = (PSPAWNINFO)GetSpawnByID(sSpawn->SpawnID);
                                                                        return;
                                                                }
                                                        }
                                                }
                                        }
                                }
                                else if(Act == 2 && CurList->pSpell && CurList->pSpell->ID && CurList->SpawnID != NOID) {
                                        if (CurList->Caster == CASTERME) {
                                                *(PSPAWNINFO*)ppTarget = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID);
                                                if ( PCONTENTS pItem = ItemWithSpell(CurList->pSpell->ID)) {
                                                        if(!SpellTimer(3,pItem)) {
                                                                sprintf(Buffer,"\"%s\" item -targetid|%d",pItem->Item->Name,CurList->SpawnID);
                                                                CastCommand((PSPAWNINFO)pCharSpawn, Buffer);
                                                                return;
                                                        }
                                                }
                                                sprintf(Buffer,"\"%s\" %s -targetid|%d",CurList->pSpell->Name,DefaultGem,CurList->SpawnID);
                                                CastCommand((PSPAWNINFO)pCharSpawn, Buffer);
                                        }
                                }
                                else if(Act == 4 && CurList->pSpell && CurList->pSpell->ID && CurList->SpawnID != NOID) {
                                        if (CurList->Caster == CASTERME) CurList->AutoRecast = true;
                                }
                                else if(Act == 5 && CurList->pSpell && CurList->pSpell->ID && CurList->SpawnID != NOID) {
                                        if (CurList->Caster == CASTERME) CurList->AutoRecast = false;
                                }
                                else if(Act == 6 && CurList->pSpell && CurList->pSpell->ID && CurList->SpawnID != NOID) {
                                        if (CurList->Caster == CASTERME) {
                                                if(CurList->AutoRecast) CurList->AutoRecast = false;
                                                else CurList->AutoRecast = true;
                                        }
                                }
                                return;
                        }
                        CurList++;
                }
        }

        VOID CastCommand(PSPAWNINFO pChar, PCHAR Cmd) {
          typedef VOID (__cdecl *CastCommandCALL) (PSPAWNINFO,PCHAR);
          PMQPLUGIN pLook=pPlugins;
          while(pLook && stricmp(pLook->szFilename,"MQ2Cast")) pLook=pLook->pNext;
          if(pLook && pLook->fpVersion>1.1100)
                  if(CastCommandCALL Request=(CastCommandCALL)GetProcAddress(pLook->hModule,"CastCommand"))
                          Request(pChar, Cmd);
        }

        /*
        //      Get Xml Window Setting's to Inifile MQ2Cast.ini
        //      In the Section [Settings.<ServerName>.<PlayerName>]
        */
        VOID ReadWindowINI() {
                #if DEBUGGING
                        DebugSpewAlways("MQ2CastTimer::ReadWindowINI()");
                #endif

                int Temp;
                this->Location.top      = GetPrivateProfileInt(szSettingINISection,"ChatTop",      300,INIFileName);
                this->Location.bottom   = GetPrivateProfileInt(szSettingINISection,"ChatBottom",      475,INIFileName);
                this->Location.left      = GetPrivateProfileInt(szSettingINISection,"ChatLeft",      150,INIFileName);
                this->Location.right    = GetPrivateProfileInt(szSettingINISection,"ChatRight",      500,INIFileName);
                this->Locked             = GetPrivateProfileInt(szSettingINISection,"Locked",         0,INIFileName);
                this->Fades             = GetPrivateProfileInt(szSettingINISection,"Fades",         1,INIFileName);
                this->TimeMouseOver       = GetPrivateProfileInt(szSettingINISection,"Delay",         2000,INIFileName);
                this->FadeDuration       = GetPrivateProfileInt(szSettingINISection,"Duration",      500,INIFileName);
                this->Alpha            = GetPrivateProfileInt(szSettingINISection,"Alpha",         255,INIFileName);
                this->FadeToAlpha      = GetPrivateProfileInt(szSettingINISection,"FadeToAlpha",   255,INIFileName);
                this->BGType            = GetPrivateProfileInt(szSettingINISection,"BGType",         1,INIFileName);
                this->BGColor.R         = GetPrivateProfileInt(szSettingINISection,"BGTint.red",      255,INIFileName);
                this->BGColor.G         = GetPrivateProfileInt(szSettingINISection,"BGTint.green",   255,INIFileName);
                this->BGColor.B         = GetPrivateProfileInt(szSettingINISection,"BGTint.blue",   255,INIFileName);

                this->TargetCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowTarget",   1,INIFileName);
                this->NpcCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowNpc",   1,INIFileName);
                this->PcCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowPC",   1,INIFileName);
                this->MercCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowMERC",   1,INIFileName);
                this->InGroupCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowGroupPCOnly",   0,INIFileName);
                this->PetCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowPet",   1,INIFileName);
                this->EventCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowEvent",   1,INIFileName);
                this->MySpellCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowMySpells",   1,INIFileName);
                this->OtherSpellCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ShowOtherSpells",   1,INIFileName);
                this->ManualRecastCheckBox->Checked = 0x00000001 & GetPrivateProfileInt(szSettingINISection,"ManualRecastSpells",   0,INIFileName);

                GetPrivateProfileString(szSettingINISection,"DefaultGem","gem1",this->DefaultGem,MAX_STRING,INIFileName);
                Temp = GetPrivateProfileInt(szSettingINISection,"ActorModeLeft",   0,INIFileName);
                this->SelectActorListLeft->SetChoice(Temp);
                Temp = GetPrivateProfileInt(szSettingINISection,"ActorModeRight",   0,INIFileName);
                this->SelectActorListRight->SetChoice(Temp);
                UpdateState = GetPrivateProfileInt(szSettingINISection,"UpdateState",   1,INIFileName);

                AutoRecast = GetPrivateProfileInt(szSettingINISection,"AutoRecast",   20,INIFileName);

                if (!UpdateState) {
                        this->Show = false;
                        SetCXStr(&StopButton->WindowText,"Pause");
                        SetCXStr(&StopButton->Tooltip,"Click to Pause");
                }
                else if(UpdateState == 1) {
                        SetCXStr(&StopButton->WindowText,"Pause");
                        SetCXStr(&StopButton->Tooltip,"Click to Pause");
                }
                else if (UpdateState == 2) {
                        SetCXStr(&StopButton->WindowText,"Update");
                        SetCXStr(&StopButton->Tooltip,"Click to Update");
                }
                this->TabPages->UpdatePage();
        }
        /*
        //      Saves Xml Window Setting's to Inifile MQ2Cast.ini
        //      In the Section [Settings.<ServerName>.<PlayerName>]
        */
        VOID WriteWindowINI() {
                #if DEBUGGING
                        DebugSpewAlways("MQ2CastTimer::WriteWindowINI()");
                #endif          
                if (this->Minimized) {
                        WritePrivateProfileString(szSettingINISection,"ChatTop",      itoa(this->OldLocation.top,      Buffer,10),INIFileName);
                        WritePrivateProfileString(szSettingINISection,"ChatBottom",   itoa(this->OldLocation.bottom,   Buffer,10),INIFileName);
                        WritePrivateProfileString(szSettingINISection,"ChatLeft",   itoa(this->OldLocation.left,      Buffer,10),INIFileName);
                        WritePrivateProfileString(szSettingINISection,"ChatRight",   itoa(this->OldLocation.right,   Buffer,10),INIFileName);
                }
                else {
                        WritePrivateProfileString(szSettingINISection,"ChatTop",      itoa(this->Location.top,         Buffer,10),INIFileName);
                        WritePrivateProfileString(szSettingINISection,"ChatBottom",   itoa(this->Location.bottom,      Buffer,10),INIFileName);
                        WritePrivateProfileString(szSettingINISection,"ChatLeft",   itoa(this->Location.left,      Buffer,10),INIFileName);
                        WritePrivateProfileString(szSettingINISection,"ChatRight",   itoa(this->Location.right,      Buffer,10),INIFileName);
                }
                WritePrivateProfileString(szSettingINISection,"Locked",      itoa(this->Locked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"Fades",      itoa(this->Fades,            Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"Delay",      itoa(this->MouseOver,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"Duration",   itoa(this->FadeDuration,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"Alpha",      itoa(this->Alpha,            Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"FadeToAlpha",   itoa(this->FadeToAlpha,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"BGType",      itoa(this->BGType,            Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"BGTint.red",   itoa(this->BGColor.R,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"BGTint.green",   itoa(this->BGColor.G,      Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"BGTint.blue",   itoa(this->BGColor.B,         Buffer,10),INIFileName);

                WritePrivateProfileString(szSettingINISection,"ShowTarget",   itoa(this->TargetCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowNpc",   itoa(this->NpcCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowPC",   itoa(this->PcCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowMERC",   itoa(this->MercCheckBox->Checked,         Buffer,10),INIFileName);

                WritePrivateProfileString(szSettingINISection,"ShowGroupPCOnly",   itoa(this->InGroupCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowPet",   itoa(this->PetCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowEvent",   itoa(this->EventCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowMySpells",   itoa(this->MySpellCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ShowOtherSpells",   itoa(this->OtherSpellCheckBox->Checked,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ManualRecastSpells",   itoa(this->ManualRecastCheckBox->Checked,         Buffer,10),INIFileName);

                WritePrivateProfileString(szSettingINISection,"DefaultGem", this->DefaultGem,INIFileName);
                WritePrivateProfileString(szSettingINISection,"AutoRecast",   itoa((int)this->AutoRecast,         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ActorModeLeft",   itoa((int)this->SelectActorListLeft->GetCurChoice(),         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"ActorModeRight",   itoa((int)this->SelectActorListRight->GetCurChoice(),         Buffer,10),INIFileName);
                WritePrivateProfileString(szSettingINISection,"UpdateState",   itoa((int)this->UpdateState,         Buffer,10),INIFileName);
        }
};

CCastTimerWnd *CastTimerWnd=0;

PCONTENTS ItemWithSpell(long SpellID) {
        for(int iSlot=0; iSlot < NUM_INV_SLOTS; iSlot++)        {
                if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
                        if(SpellID == cSlot->Item->Clicky.SpellID) {
                                return cSlot;
                        } else if (cSlot->Item->Type == ITEMTYPE_PACK) {
                                for(int iPack=0; iPack < cSlot->Item->Slots; iPack++) {
                                        if(PCONTENTS cPack=cSlot->Contents[iPack])
                                                if(SpellID == cPack->Item->Clicky.SpellID)
                                                        return cPack;
                                }
                        }
                }
        }
        return NULL;
}

PCONTENTS ItemSlots(long SlotID) {
        long InvSlot=NOID;
        long SubSlot=NOID;
        if(!(SlotID < NUM_INV_SLOTS)) {
                  InvSlot=23+(SlotID-262)/10;
                  SubSlot=(SlotID-1)%10;
        } else InvSlot=SlotID;
        if(InvSlot < NUM_INV_SLOTS) {
                  if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[InvSlot]) {
                                if(SubSlot < 0) return cSlot;
                                if(PCONTENTS cPack=cSlot->Contents[SubSlot]) return cPack;
                  }
        }
        return NULL;
}
   
PITEMINFO ItemInfos(PCONTENTS Contents) {
        return (Contents)?Contents->Item:NULL;
}

PITEMINFO ItemBySlot(long SlotID) {
        return ItemInfos(ItemSlots(SlotID));
} 

eSpawnType SpawnType(PSPAWNINFO tSpawn) {
        if(!tSpawn) return NONE;
        eSpawnType eST=GetSpawnType(tSpawn);
        if(eST!=PET) return eST;
        PSPAWNINFO mST=(PSPAWNINFO)GetSpawnByID(tSpawn->MasterID);
        return (!mST || GetSpawnType(mST)!=PC)?NPC:PET;
}

/*
// Clear all timer's from list
*/
VOID ClearAll() {
        TimerCount = 0;
        TimerList.clear();
        XmlList.clear();
}

/*
// Clear one timer's from list
*/
VOID ClearOne(list<TimerSingle>::iterator &CurList) {
        list<TimerSingle>::iterator PosList;
        PosList=CurList;
        CurList++;
        TimerList.erase(PosList);
}

/*
// UpdateBuffs from Target
*/
VOID UpdateBuffsTarget( ) {
        static unsigned long TargetChangeTime = clock();
        static long LastTargetID =NOID; 

        int i;
        int buffID = 0;
        time_t now = time(NULL);
        time_t thisBuffTimestamp = time(NULL);

        long TargetID =(pTarget)?((PSPAWNINFO)pTarget)->SpawnID:NOID;

        if (TargetID == NOID) {
                LastTargetID = NOID;
                return;
        }
        PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(TargetID);

        if ( GetSpawnType(pSpawn) != PC && GetSpawnType(pSpawn) != MERCENARY ) {
                if ( !GetCharInfo()->ActiveAbilities.InspectBuffs ) return;

                int Gcount=0;
                if(!GetCharInfo()->pGroupInfo) return;
                if ( GetCharInfo()->pGroupInfo ) {
                        for(i=1; i<6; i++) if ( GetCharInfo()->pGroupInfo->pMember[i] ) Gcount++;
                }
                if ( Gcount <= 1 ) return;
        }

        if ( LastTargetID != TargetID ) {
                LastTargetID = TargetID;
                TargetChangeTime = clock();
                return;
        } else if ( TargetChangeTime+200+pConnection->Last > (unsigned long)clock() ) {
                for(i = 0; i < 0x55; i++) {
                        buffID = ((PCTARGETWND)pTargetWnd)->BuffSpellID[i];
                        if(buffID != 0xffffffff) {
                                if (PSPELL pSpell = GetSpellByID(buffID)) TargetChangeTime=0;
                        }
                }
                if ( TargetChangeTime ) return;
        }

        list<TimerSingle>::iterator CurList=TimerList.begin();
        list<TimerSingle>::iterator EndList=TimerList.end();

        bool spellNotFouned=false;
        //update spell timers on target and removing spells no longer on target.
        CurList=TimerList.begin();
        while(CurList!=EndList) {
                spellNotFouned=false;
                
                if ( CurList->SpawnID!=NOID && CurList->SpawnID == TargetID && CurList->pSpell && CurList->pSpell->ID  && CurList->pSpell->DurationWindow==0 && CurList->CastE==CAST_SUCCESS ) {
                        if ( CurList->AutoRecast ) {
                                spellNotFouned=false;
                        } else {
                                spellNotFouned=true;
                        }
                        for(i = 0; i < 0x55; i++) {
                           buffID = ((PCTARGETWND)pTargetWnd)->BuffSpellID[i];
                                if(buffID != 0xffffffff) {
                                        if (PSPELL pSpell = GetSpellByID(buffID)) {
                                                if ( CurList->pSpell->ID == pSpell->ID ) {
                                                        thisBuffTimestamp = now+(((PCTARGETWND)pTargetWnd)->BuffTimer[i] / 1000);
                                                        if ( CurList->timestamp != thisBuffTimestamp ) {
                                                                TimerSingle MyData;
                                                                DefaultTimer(MyData);
                                                                MyData.TimerID = CurList->TimerID;
                                                                MyData.pSpell = CurList->pSpell;
                                                                MyData.CastE = CAST_SUCCESS;
                                                                MyData.SpawnID = CurList->SpawnID;
                                                                MyData.timestamp = thisBuffTimestamp;
                                                                MyData.Caster = CurList->Caster;
                                                                MyData.AutoRecast = CurList->AutoRecast;
                                                                RestackTimer(CurList,MyData,1);
                                                        }
                                                        spellNotFouned=false;
                                                        break;
                                                }
                                        }
                                }
                        }
                }
                if (spellNotFouned) ClearOne(CurList);
                else CurList++;
        }

        //add new spells to target will be added as spells others have casted unless it is spell you can cast on them.
        for(i = 0; i < 0x55; i++) {
                spellNotFouned=true;
                buffID = ((PCTARGETWND)pTargetWnd)->BuffSpellID[i];
                thisBuffTimestamp = now+(((PCTARGETWND)pTargetWnd)->BuffTimer[i] / 1000);
                if(buffID != 0xffffffff) {
                        if (PSPELL pSpell = GetSpellByID(buffID)) {
                                if ( pSpell->DurationWindow == 0 ) {
                                        CurList=TimerList.begin();
                                        while(CurList!=EndList) {
                                                if ( CurList->SpawnID!=NOID && CurList->SpawnID == TargetID && CurList->pSpell && CurList->pSpell->ID == pSpell->ID ) {
                                                        spellNotFouned=false;
                                                        break;
                                                }
                                                CurList++;
                                        }
                                        if (spellNotFouned) {
                                                TimerSingle MyData;
                                                DefaultTimer(MyData);
                                                TimerCount++;
                                                MyData.TimerID = TimerCount;
                                                MyData.pSpell = pSpell;
                                                MyData.CastE = CAST_SUCCESS;
                                                MyData.SpawnID = TargetID;
                                                MyData.timestamp = thisBuffTimestamp;
                                                MyData.Caster = CASTEROTHER;
                                                for (DWORD N = 0 ; N < NUM_BOOK_SLOTS ; N++) {
                                                        if (PSPELL pTempSpell=GetSpellByID(GetCharInfo2()->SpellBook[N])) {
                                                                if (pTempSpell->ID == pSpell->ID) {
                                                                        MyData.Caster = CASTERME;
                                                                        break;
                                                                }
                                                        }
                                                }
                                                if ( TargetID == GetCharInfo()->pSpawn->SpawnID && MyData.Caster != CASTERME ) {
                                                        if ( PCONTENTS pItem = ItemWithSpell(pSpell->ID) ) MyData.Caster = CASTERME;
                                                }
                                                RestackTimer(CurList,MyData,0);
                                        }
                                }
                        }
                }
        }
}

/*
 * Set default timer data
 */
VOID DefaultTimer(TimerSingle &MyData) { 
        MyData.TimerID = NOID;
        MyData.pSpell = NULL;
        MyData.EventID = NOID;
        MyData.CastE = CAST_SUCCESS;
        MyData.SpawnID = NOID;
        MyData.timestamp = 0;
        MyData.Caster = CASTERME;
        MyData.AutoRecast = false;

        char emptyName[64] = {NULL};
        strcpy(MyData.SpawnName,emptyName);

        MyData.TimeOut = 0;

        MyData.SpawnWatch = 0;
        MyData.SpawnWatchX = 0;
        MyData.SpawnWatchY = 0;
        MyData.SpawnWatchZ = 0;
}

/*
 * Set timer data from CurList
 */
VOID SetTimerData(TimerSingle &MyData,list<TimerSingle>::iterator &CurList) {
        MyData.TimerID = CurList->TimerID;
        MyData.pSpell = CurList->pSpell;
        MyData.EventID = CurList->EventID;
        MyData.CastE = CurList->CastE;
        MyData.SpawnID = CurList->SpawnID;
        MyData.timestamp = CurList->timestamp;
        MyData.Caster = CurList->Caster;
        MyData.AutoRecast = CurList->AutoRecast;
        strcpy(MyData.SpawnName,CurList->SpawnName);
        MyData.TimeOut = CurList->TimeOut;
        MyData.SpawnWatch = CurList->SpawnWatch;
        MyData.SpawnWatchX = CurList->SpawnWatchX;
        MyData.SpawnWatchY = CurList->SpawnWatchY;
        MyData.SpawnWatchZ = CurList->SpawnWatchZ;
}

/*
// Restack Timer
*/
VOID RestackTimer(list<TimerSingle>::iterator &CurList,TimerSingle MyData,int Remove) {
        if (Remove) ClearOne(CurList);

        if (Remove > -1) {
                CurList=TimerList.begin();
                list<TimerSingle>::iterator EndList=TimerList.end();
                while(CurList!=EndList && MyData.timestamp>CurList->timestamp ) CurList++;
                TimerList.insert(CurList,MyData);
        } else {
                TimerList.push_back(MyData);
        }
        if (Remove==1) CurList=TimerList.begin();
}


/*
// Clear timer by ....
*/
VOID ClearTimer(long SpawnID,eSpawnType SpawnType) {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        list<TimerSingle>::iterator CurList=TimerList.begin();
        list<TimerSingle>::iterator EndList=TimerList.end();
        time_t now = time(NULL);
        TimerSingle MyData;     
        int Remove = 0;
        while(CurList!=EndList) {
                Remove = 0;
                if (CurList->SpawnWatch) {
                        if ( CurList->SpawnWatch == GetCharInfo()->zoneId ) {
                                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID)) {
                                        if ( pSpawn->Type == SPAWN_CORPSE ) {
                                                DefaultTimer(MyData);
                                                SetTimerData(MyData, CurList);

                                                if ( MyData.TimeOut ) MyData.timestamp = now + MyData.TimeOut;
                                                else MyData.timestamp = now;
                                                MyData.SpawnID = NOID;
                                                Remove = 2;
                                        }
                                }
                        }
                } else {
                        if ( CurList->SpawnID != NOID ) {
                                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID)) {
                                        if ((pSpawn->Type == SPAWN_CORPSE) || SpawnType != NOID && GetSpawnType(pSpawn) == SpawnType) Remove = 1;
                                }
                        }

                        if ( CurList->SpawnName[0] || CurList->TimeOut != 0 ) {
                                if (difftime(CurList->TimeOut, now)<0) Remove = 1;
                        } else if (difftime(CurList->timestamp, now)<0 && (!CurList->AutoRecast || CurList->Caster != CASTERME)) {
                                if ( ((CastTimerWnd && !CastTimerWnd->ManualRecastCheckBox->Checked) || !CastTimerWnd || CurList->Caster != CASTERME) ) Remove = 1;
                        }
                        else if(CurList->SpawnID == SpawnID && SpawnID != NOID) Remove = 1;
                }
                if (Remove == 2) RestackTimer(CurList,MyData,-1);
                else if(Remove) ClearOne(CurList);
                else CurList++;
        }
}

/*
//      Add spell self casted spells spell list including instant cast
//      only usede from /casting
*/
VOID AddSpell(int TargetID,int SpellID,int TickE) {
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::AddSpell()");
        #endif
        if (PSPELL pSpell = (PSPELL)GetSpellByID(SpellID)) {
                if (!GetSpellDuration(pSpell,(PSPAWNINFO)pCharSpawn) ) return;
                switch(pSpell->TargetType) {
                        case  3: // Group v1
                        case  6: // Self
                        case 41: // Group v2
                                AddSpellTimer(GetCharInfo()->pSpawn->SpawnID,pSpell,TickE);
                                break;
                        case 14: // Pet
                                AddSpellTimer(GetCharInfo()->pSpawn->PetID,pSpell,TickE);
                                break;
                        case  5: // Single
                        case  8: // Targeted AE
                                if(VerifyTarget(TargetID,pSpell)) AddSpellTimer(TargetID,pSpell,TickE);
                                break;
                        default: break; // Unknown
                }
                if (CastTimerWnd) CastTimerWnd->UpdateList(TargetID);
        }
}

/*
//      Verity Target is correct for spell when the spell is casted
//      from MQ2Cast using /casting
case 2: // Beneficial(Group) , case 1: // Beneficial , case 0: // Detrimental , default: // Unknown
*/
bool VerifyTarget(long SpawnID,PSPELL pSpell) {
        if (PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(SpawnID)) {
                if(pSpawn->Type == SPAWN_CORPSE) return false;
                switch(SpawnType(pSpawn)) {
                        case NPC: // NPC
                                switch(pSpell->SpellType) {
                                        case 2: // Beneficial(Group)
                                        case 1: // Beneficial                           
                                                return false;
                                        case 0: // Detrimental
                                        default: // Unknown
                                                return true;
                                }
                                break;
                        case PET: // PET
                                switch(pSpell->SpellType) {
                                        case 2: // Beneficial(Group)
                                        case 1: // Beneficial
                                                return true;
                                        case 0: // Detrimental
                                                return false;
                                        default: // Unknown
                                                return true;
                                }
                                break;
                        case NONE: // NONE
                                switch(pSpell->SpellType) {
                                        case 2: // Beneficial(Group)
                                        case 1: // Beneficial
                                                return true;
                                        case 0: // Detrimental
                                                return false;
                                        default: // Unknown
                                                return true;
                                }
                                break;
                        default: // Unknown / PC / MERCENARY
                                switch(GetSpawnType(pSpawn)) {
                                        case PC: // PC
                                                switch(pSpell->SpellType) {
                                                        case 2: // Beneficial(Group)
                                                        case 1: // Beneficial
                                                                return true;
                                                        case 0: // Detrimental
                                                                return false;
                                                        default: // Unknown
                                                                return true;
                                                }
                                                break;
                                        case MERCENARY: // MERCENARY
                                                switch(pSpell->SpellType) {
                                                        case 2: // Beneficial(Group)
                                                        case 1: // Beneficial
                                                                return true;
                                                        case 0: // Detrimental
                                                                return false;
                                                        default: // Unknown
                                                                return true;
                                                }
                                                break;
                                        default: // Unknown
                                                return true;
                                }
                                break;
                }
                return true;
        }
        return false;
}

/*
//      Add spell timer to spell timer list
*/
VOID AddSpellTimer(int SpawnID,PSPELL pSpell,int TickE,bool ignoreMod) {
        if (!SpawnID || SpawnID == NOID || !pSpell->ID || !gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::AddSpellTimer()::Spell: %s To %d",pSpell->Name,SpawnID);
                if(ignoreMod) WriteChatf("NOT MINE");
                else WriteChatf("MINE");
        #endif

        time_t now = time(NULL);
    list<TimerSingle>::iterator CurList=TimerList.begin();
    list<TimerSingle>::iterator EndList=TimerList.end();
        switch(TickE) {
                case CAST_SUCCESS: // CAST_SUCCESS
                        now = now+CalDuratsion(pSpell,ignoreMod);
                        break;
                case CAST_IMMUNE: // CAST_IMMUNE
                        now = now+100000;
                        break;
                case CAST_TAKEHOLD: // CAST_TAKEHOLD
                        now = now+CalDuratsion(pSpell,ignoreMod);
                        break;
                default: // Unknown
                        return;
        }
        bool AutoRecast = false;

        bool Remove = false;
        while(CurList!=EndList) {
                if(CurList->SpawnID && CurList->SpawnID == SpawnID && CurList->pSpell && CurList->pSpell->ID == pSpell->ID) {
                        if( (ignoreMod && CurList->Caster == CASTEROTHER) || !ignoreMod ) {
                                if((now-CurList->timestamp) <= 1) TickE = CurList->CastE;
                                AutoRecast = CurList->AutoRecast;
                        }
                        Remove = true;
                        break;
                }
                else CurList++;
        }

        TimerSingle MyData;
        DefaultTimer(MyData);
        TimerCount++;
        MyData.TimerID = TimerCount;
        MyData.pSpell = pSpell;
        MyData.CastE = TickE;
        MyData.SpawnID = SpawnID;
        MyData.AutoRecast = AutoRecast;
        MyData.timestamp = now;
        if(ignoreMod) MyData.Caster = CASTEROTHER;

        RestackTimer(CurList,MyData,Remove);
}

/*
//      Add event timer to spell timer list
*/
VOID AddEventTimer(int EventNum,int SpawnID) {
        time_t now = time(NULL);
        now = now+EventList[EventNum].Duration;
    list<TimerSingle>::iterator CurList=TimerList.begin();
    list<TimerSingle>::iterator EndList=TimerList.end();
        TimerSingle MyData;
        DefaultTimer(MyData);

        while(CurList!=EndList) {
                if (CurList->SpawnID && CurList->EventID == EventNum && CurList->SpawnID == SpawnID) {
                        ClearOne(CurList);
                        break;
                }
                else CurList++;
        }
        TimerCount++;
        MyData.TimerID = TimerCount;
        MyData.EventID = EventNum;
        MyData.SpawnID = SpawnID;
        MyData.timestamp = now;
        MyData.Caster = CASTEREVENT;

        RestackTimer(CurList,MyData,0);
}

/*
//      Add event timer to spell timer list
*/
VOID AddSpawnTimer(int SpawnID) {
        if (!SpawnID) return;
        if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(SpawnID)) {
                list<TimerSingle>::iterator CurList=TimerList.begin();
            list<TimerSingle>::iterator EndList=TimerList.end();
                time_t now = time(NULL);

                while(CurList!=EndList) {
                        if( CurList->SpawnWatch && CurList->SpawnWatch == GetCharInfo()->zoneId ) {
                                if ( ( CurList->SpawnID!=NOID && CurList->SpawnID == SpawnID ) || ( pSpawn->X == CurList->SpawnWatchX && pSpawn->Y == CurList->SpawnWatchY && pSpawn->Z == CurList->SpawnWatchZ ) ) return;
                        }
                        CurList++;
                }
                CurList=TimerList.begin();

                TimerSingle MyData;
                DefaultTimer(MyData);

                TimerCount++;
                MyData.TimerID = TimerCount;
                MyData.SpawnID = pSpawn->SpawnID;
                MyData.timestamp = 0;

                if ( IsNamed(pSpawn) ) strcpy(MyData.SpawnName,pSpawn->Name);

                MyData.SpawnWatchX = pSpawn->X;
                MyData.SpawnWatchY = pSpawn->Y;
                MyData.SpawnWatchZ = pSpawn->Z;

                MyData.SpawnWatch = GetCharInfo()->zoneId;

                RestackTimer(CurList,MyData,0);
        }
}

/*
//      Calculate Spell duratsion
*/
int CalDuratsion(PSPELL pSpell,bool ignoreMod) {
        if(ignoreMod || BardClass()) return (GetSpellDuration(pSpell,GetCharInfo()->pSpawn)*6); 
        float dMod = 0;
        int dModTick = 0;
        int Dur = 0;                    // Duratsion
        DWORD MinDur = 0;               // Minimun Level

        dMod = BestDuratsionFocus(pSpell);
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::ReadCastTimerINI()::Focus spell: dMod %d",dMod);
        #endif
        if ( !GetCharInfo2()->Shrouded ) {
                        switch(pSpell->SpellType) {
                                case 0:  //Detrimental
                                        for (int i=0; i<=11; i++) { 
                                                switch(pSpell->Attrib[i]) {
                                                        case 31: //Mesmerize(c/level) 
                                                                if (PlayerHasAAAbility(GetAAIndexByName("Mesmerization Mastery"))) {
                                                                        for (unsigned int j=0; j < AA_CHAR_MAX_REAL; j++) {
                                                                                if (pPCData->GetAltAbilityIndex(j)) {
                                                                                        if ( pPCData->GetAltAbilityIndex(j) == GetAAIndexByName("Mesmerization Mastery")) {
                                                                                                switch(GetCharInfo2()->AAList[j].PointsSpent) {
                                                                                                        case 12: // level 1
                                                                                                                dModTick = dModTick + 1;
                                                                                                                break;
                                                                                                        case 24: // level 2
                                                                                                                dModTick = dModTick + 2;
                                                                                                                break;
                                                                                                        default: // Unknown
                                                                                                                break;
                                                                                                }
                                                                                                break; 
                                                                                        }
                                                                                }
                                                                        } 
                                                                }
                                                                break;
                                                        default: //undefined effect 
                                                                break; 
                                                }
                                        }
                                        break;
                                case 1:  //Beneficial(Group)
                                case 2:  //Beneficial
                                        if(PlayerHasAAAbility(GetAAIndexByName("Spell Casting Reinforcement Mastery"))) dMod = dMod+50;
                                        else if (PlayerHasAAAbility(GetAAIndexByName("Spell Casting Reinforcement"))) {
                                                for (unsigned int j=0; j < AA_CHAR_MAX_REAL; j++) {
                                                        if (pPCData->GetAltAbilityIndex(j)) {
                                                                if ( pPCData->GetAltAbilityIndex(j) == GetAAIndexByName("Spell Casting Reinforcement")) {
                                                                        switch(GetCharInfo2()->AAList[j].PointsSpent) {
                                                                                case 2: // level 1
                                                                                        dMod = dMod+5;
                                                                                        break;
                                                                                case 6: // level 2
                                                                                        dMod = dMod+15;
                                                                                        break;
                                                                                case 12: // level 3
                                                                                        dMod = dMod+30;
                                                                                        break;
                                                                                default: // Unknown
                                                                                        break;
                                                                        }
                                                                        break; 
                                                                }
                                                        }
                                                } 
                                        }
                                        break;
                                default: // Unknown
                                        break;
                        }

                if( pSpell->SpellType == 1 || pSpell->SpellType == 2 ) {

                }
        }
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::ReadCastTimerINI()::dMod after AA: dMod %d",dMod);
        #endif
        Dur = (GetSpellDuration(pSpell,GetCharInfo()->pSpawn)*6);
        dMod = ((Dur*dMod)/100);
        Dur = Dur+(int)dMod+(dModTick*6);
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::ReadCastTimerINI()::Finished: Dur = %d  dMod = %d",Dur,dMod);
        #endif
        return Dur;
}
/*
//      Find the Best Worn Focus Item by SpellType
*/
//PSPELL BestDuratsionFocus(PSPELL pSpell) {
float BestDuratsionFocus(PSPELL pSpell) {
        int FocusLevel = 0;
        int FocusLevelDecay = 0;

        float BestFocusMod = 0;
        float FocusMod = 0;

        DWORD MinDur = 0;

        bool correctType = false;
        bool correctFocus = false;
        PSPELL rSpell=NULL;
        for(int Scan=0; Scan<BAG_SLOT_START; Scan++) {
                if(PCONTENTS pItem=GetCharInfo2()->InventoryArray[Scan]) {
                        if(PSPELL fSpell=GetSpellByID(pItem->Item->Focus.SpellID)) {
                                correctType = false;
                                FocusMod = 0;
                                correctFocus = true;

                                for(int i=0; i<12; i++) {
                                        switch(fSpell->Attrib[i]) {
                                                case 128: // spell duration
                                                        FocusMod = (float)fSpell->Base[i];
                                                        break;
                                                case 134: // limit max level
                                                                FocusLevel = fSpell->Base[i];
                                                                FocusLevelDecay = fSpell->Base2[i];
                                                        break;
                                                case 138: // limit spelltype
                                                        if(fSpell->Base[i] == pSpell->SpellType)
                                                                correctType = true;
                                                        break;
                                                case 139: // limit spell this affects
                                                        if(PSPELL lSpell=GetSpellByID(abs(fSpell->Base[i]))) {
                                                                if(fSpell->Base[i]>0 && pSpell->ID != lSpell->ID) correctFocus = false;
                                                                if(fSpell->Base[i]<0 && pSpell->ID == lSpell->ID) correctFocus = false;
                                                        }
                                                        break;
                                                case 140: //limit min duration of spells this affects (base= #ticks)
                                                        MinDur = fSpell->Base[i];
                                                        break;
                                        }
                                }
                                if(MinDur && MinDur >= GetSpellDuration(pSpell,(PSPAWNINFO)pCharSpawn)) FocusMod = 0;
                                if ( FocusMod && correctType && correctFocus ) {
                                        if (pSpell->Level[GetCharInfo()->pSpawn->Class-1] > FocusLevel) FocusMod = FocusMod-((((pSpell->Level[((PSPAWNINFO)pCharSpawn)->Class-1]-FocusLevel)*FocusLevelDecay)*FocusMod)/100); 

                                        if( FocusMod > BestFocusMod ) {
                                                BestFocusMod = FocusMod;
                                                rSpell = fSpell;
                                        }
                                }
                        }
                        for(int iPack=0; iPack < 4; iPack++) {
                                if(PCONTENTS cPack=pItem->Contents[iPack]) {
                                        if(PSPELL fSpell=GetSpellByID(cPack->Item->Focus.SpellID)) {
                                                correctType = false;
                                                FocusMod = 0;
                                                correctFocus = true;

                                                for(int i=0; i<12; i++) {
                                                        switch(fSpell->Attrib[i]) {
                                                                case 128: // spell duration
                                                                        FocusMod = (float)fSpell->Base[i];
                                                                        break;
                                                                case 134: // limit max level
                                                                                FocusLevel = fSpell->Base[i];
                                                                                FocusLevelDecay = fSpell->Base2[i];
                                                                        break;
                                                                case 138: // limit spelltype
                                                                        if(fSpell->Base[i] == pSpell->SpellType)
                                                                                correctType = true;
                                                                        break;
                                                                case 139: // limit spell this affects
                                                                        if(PSPELL lSpell=GetSpellByID(abs(fSpell->Base[i]))) {
                                                                                if(fSpell->Base[i]>0 && pSpell->ID != lSpell->ID) correctFocus = false;
                                                                                if(fSpell->Base[i]<0 && pSpell->ID == lSpell->ID) correctFocus = false;
                                                                        }
                                                                        break;
                                                                case 140: //limit min duration of spells this affects (base= #ticks)
                                                                        MinDur = fSpell->Base[i];
                                                                        break;
                                                        }
                                                }
                                                if(MinDur && MinDur >= GetSpellDuration(pSpell,(PSPAWNINFO)pCharSpawn)) FocusMod = 0;
                                                if ( FocusMod && correctType && correctFocus ) {
                                                        if (pSpell->Level[GetCharInfo()->pSpawn->Class-1] > FocusLevel) FocusMod = FocusMod-((((pSpell->Level[((PSPAWNINFO)pCharSpawn)->Class-1]-FocusLevel)*FocusLevelDecay)*FocusMod)/100); 

                                                        if( FocusMod > BestFocusMod ) {
                                                                BestFocusMod = FocusMod;
                                                                rSpell = fSpell;
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
        if(BestFocusMod<0) BestFocusMod = 0;
        return BestFocusMod;
//      return rSpell;
}
/*
// Read Costom events from ini file
*/
VOID LoadCostumEvents() {
        #if DEBUGGING
                DebugSpewAlways("[MQ2CastTimer::LoadCostumEvents");
        #endif  
        CHAR szSection[MAX_STRING] = {0};
        CHAR szTemp[MAX_STRING] = {0};
        int prev=0;

        EventTimer MyData;
        //load timer array
        GetPrivateProfileString(NULL,NULL,NULL,Buffer,MAX_STRING,INIFileName);
        for (int i=0; ((Buffer[i] != 0) || (Buffer[i+1] != 0)); i++) {
                szSection[i-prev] = Buffer[i];
                if ( (Buffer[i]==0 ) && ( !strstr(szSection,"Settings") ) ) {
                        strcpy(MyData.Name, szSection);
//                      GetPrivateProfileString(szSection,"isspell","0",szTemp,MAX_STRING,INIFileName);
//                      MyData.IsSpell = atoi(Buffer);
                        GetPrivateProfileString(szSection,"duration","3",szTemp,MAX_STRING,INIFileName);
                        MyData.Duration = atoi(szTemp);

                        GetPrivateProfileString(szSection,"spawntype","0",szTemp,MAX_STRING,INIFileName);
                        MyData.SpawnType = atoi(szTemp);

                        GetPrivateProfileString(szSection,"event","#*#EVENT_UNDEFINED#*#",szTemp,MAX_STRING,INIFileName);
                        strcpy(MyData.Event, szTemp);

                        EventList.push_back(MyData);

                        #if MQ2BLECH == 0
                                CUSTOMEVENT.AddEvent(szTemp,TimerCustomEvent,(void*)((int)EventList.size()-1));
                        #endif
                        #if MQ2BLECH == 1
                                BleachList.push_back(pMQ2Blech->AddEvent(szTemp,TimerCustomEvent,(void*)((int)EventList.size()-1)));
                        #endif
                        prev=i+1;
                }
        }
}
/*
//      Output's Timer Data to ${SpellTimer[SpawnID,SpellName]} depending on
//      SpawnID and SpellName, By calling getSpellTimer(SpawnID, SpellName)
*/
BOOL dataTimer(PCHAR szName, MQ2TYPEVAR &Dest) {
        long SpawnID=0;
        long Caster=CASTERME;
        vector<PSPELL> SpellSearch;

        long iParm=0;
        long iParm2=0;
        char zParm[MAX_STRING];
        char zParm2[MAX_STRING];
        do {
                GetArg(zParm,szName,++iParm,FALSE,FALSE,FALSE,',');
                iParm2=0;
                strcpy(Buffer,"");
                do {            
                        GetArg(zParm2,zParm,++iParm2);
                        if(zParm2[0]==0) break;
                        if(iParm2>1) strcat(Buffer," ");
                        strcat(Buffer,zParm2);
                }
                while(true);

                if(Buffer[0]==0) break;
                else if(SpawnID==0) SpawnID = atol(Buffer);
                else if(!stricmp(Buffer,"all") || !stricmp(Buffer,"any")) Caster = CASTERANY;
                else if(!stricmp(Buffer,"me")) Caster = CASTERME;
                else if(!stricmp(Buffer,"other")) Caster = CASTEROTHER;
                else if(PSPELL pSpell = (PSPELL)GetSpellByName(Buffer)) SpellSearch.push_back(pSpell);
        }
        while(true);

        Dest.DWord=getSpellTimer(SpawnID, Caster, SpellSearch);
        Dest.Type=pIntType;
        return true;
}

long getSpellTimer(long SpawnID, int Caster,vector<PSPELL> SpellSearch) {
    list<TimerSingle>::iterator CurList=TimerList.begin();
    list<TimerSingle>::iterator EndList=TimerList.end();
        time_t now = time(NULL);
        long int SpellTime = 0;
        if (SpellSearch.size()) {
                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(SpawnID)) {
                        while(CurList!=EndList) {
                                if(CurList->SpawnID && CurList->SpawnID == pSpawn->SpawnID && pSpawn->Type != SPAWN_CORPSE && CurList->pSpell && CurList->EventID == NOID && (CurList->Caster == Caster || Caster == CASTERANY)) {
                                        for (long c=0; c<(long)SpellSearch.size(); c++) {
                                                if ( !stricmp(SpellSearch[c]->Name,CurList->pSpell->Name) || (strstr(CurList->pSpell->Name,"Rk. II") && !strnicmp(SpellSearch[c]->Name,CurList->pSpell->Name,strlen(CurList->pSpell->Name)-8))) {
                                                        long int time = (long)difftime(CurList->timestamp, now);
                                                        if ( time > SpellTime ) SpellTime = time;
                                                }
                                        }
                                }
                                CurList++;
                        }
                }
        }
        if ( SpellTime ) return SpellTime;
        return 0;
}

/*
//      Create window and Read / write window settings
*/
VOID CreateTimerWindow() {
        if(!gbInZone || CastTimerWnd || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::CreateTimerWindow()");
        #endif  
        AddXMLFile("MQUI_CastTimerWnd.xml");
        if (pSidlMgr->FindScreenPieceTemplate("CastTimerWnd")) CastTimerWnd = new CCastTimerWnd;
}

/*
//      Destroy window and write window settings
*/
VOID DestroyTimerWindow() {
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::DestroyTimerWindow()");
        #endif          
        if(CastTimerWnd) CastTimerWnd->WriteWindowINI();
        delete CastTimerWnd;
        CastTimerWnd=0;
}

/*
//      Ingame commands:
//      /timer [on|off]
//      /timer clear [all|target|npc|pc|pet]
//      /timer autorecast #
*/
VOID MQTimerCommand(PSPAWNINFO pChar, PCHAR szLine) {   
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::MQTimerCommand()");
        #endif          
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;       
        long iParm=0;
        long cTarget=NOID;
        do {
                GetArg(Buffer,szLine,++iParm);
                if(Buffer[0]==0) return;
                if(!strnicmp(Buffer,"clear",5)) {
                        GetArg(Buffer,szLine,++iParm);
                        if(!strnicmp(Buffer,"all",3)) ClearAll();
                        else if(!strnicmp(Buffer,"target",6)) {
                                cTarget=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;
                                if(cTarget==NOID) return;
                                ClearTimer(cTarget,NONE);
                        }
                        else if(!strnicmp(Buffer,"pc",2)) ClearTimer(NOID,PC);
                        else if(!strnicmp(Buffer,"merc",2)) ClearTimer(NOID,MERCENARY);
                        else if(!strnicmp(Buffer,"npc",3)) ClearTimer(NOID,(eSpawnType)NPC);
                        else if(!strnicmp(Buffer,"pet",3)) ClearTimer(NOID,(eSpawnType)PET);
                }
                else if(!strnicmp(Buffer,"off",3)) {
                        CreateTimerWindow();
                        if(CastTimerWnd && CastTimerWnd->Show) {
                                ((CXWnd*)CastTimerWnd)->Show(0,0);
                                CastTimerWnd->UpdateState = 0;
                        }
                }
                else if(!strnicmp(Buffer,"on",2)) {
                        CreateTimerWindow();
                        if(CastTimerWnd && !CastTimerWnd->Show) {
                                ((CXWnd*)CastTimerWnd)->Show(1,1);
                                SetCXStr(&CastTimerWnd->StopButton->WindowText,"Pause");
                                SetCXStr(&CastTimerWnd->StopButton->Tooltip,"Click to Pause");
                                CastTimerWnd->UpdateState = 1;
                        }
                }
                else if(!strnicmp(Buffer,"autorecast",10)) {
                        GetArg(Buffer,szLine,++iParm);
                        if(CastTimerWnd) CastTimerWnd->AutoRecast = atol(Buffer);
                }
                else if(!strnicmp(Buffer,"watch",5)) {
                        cTarget=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;
                        if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(cTarget)) {
                                if (pSpawn->Type == SPAWN_NPC) AddSpawnTimer(pSpawn->SpawnID);
                        }
                }
        }
        while(true);
}

/*
//      Handel Spell Casting done by your self and call AddSpell() to
//      add a spell and call CastTimerWnd->UpdateList() to update the Target window
*/
PLUGIN_API VOID TimerCastHandle(int TargetID,int SpellID,long TickE) {
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::TimerCastHandle()");
        #endif  
        if(SpellID && TickE == CAST_RECOVER) TickE = CAST_SUCCESS;
        if(SpellID && TickE == CAST_OUTDOORS) TickE = CAST_IMMUNE;
        if(TickE != CAST_SUCCESS && TickE != CAST_IMMUNE && TickE != CAST_TAKEHOLD) return;
        AddSpell(TargetID,SpellID,TickE);
}

PLUGIN_API VOID OnPulse(VOID) {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return;

    list<TimerSingle>::iterator CurList=TimerList.begin();
    list<TimerSingle>::iterator EndList=TimerList.end();

        if (OldSpawnID != GetCharInfo()->pSpawn->SpawnID) {
                TimerSingle MyData;
                DefaultTimer(MyData);
                char emptyName[64] = {NULL};
                while(CurList!=EndList) {
                        if(CurList->SpawnID && OldSpawnID == CurList->SpawnID) {
                                SetTimerData(MyData, CurList);
                                MyData.SpawnID = GetCharInfo()->pSpawn->SpawnID;
                                strcpy(MyData.SpawnName,emptyName);

                                RestackTimer(CurList,MyData,1);
                        } else {
                                CurList++;
                        }
                }
                OldSpawnID = GetCharInfo()->pSpawn->SpawnID;
        }

        ParseSpellsOnce();

        CreateTimerWindow();

        CastingC=GetCharInfo()->pSpawn->CastingData.SpellID?GetCharInfo()->pSpawn->CastingData.SpellID:NOID;

        // casting window currently openened?
    if(pCastingWnd && (PCSIDLWND)pCastingWnd->Show) {
                if(CastingO==NOID) CastingO=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):0;
        }
        if(CastingC == NOID) CastingO = NOID;

        UpdateBuffsTarget();

        currentTargetID=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):NOID;
        time_t Now = time(NULL);
        if (Now < lastRun || Now >= lastRun+1) ClearTimer(NOID,NONE);

        if(!CastTimerWnd || CastTimerWnd->UpdateState != 1) return;

    CurList=TimerList.begin();
    EndList=TimerList.end();
        
        static bool anounceAutoRecast = true;
        if(GetCharInfo()->pSpawn->CastingData.SpellID) {
                while(CurList!=EndList) {
                        if (CurList->SpawnID!=NOID && CurList->AutoRecast && difftime(CurList->timestamp, Now) <= CastTimerWnd->AutoRecast && CurList->pSpell ) {
                                        if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID)) {
                                                if(pSpawn->Type != SPAWN_CORPSE) {
                                                        if(anounceAutoRecast) {
                                                                WriteChatf("[MQ2CastTimer] Auto Recasting %s", CurList->pSpell->Name);
                                                                anounceAutoRecast = false;
                                                        }

                                                        *(PSPAWNINFO*)ppTarget = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID);
                                                        if ( PCONTENTS pItem = ItemWithSpell(CurList->pSpell->ID)) {
                                                                if(!SpellTimer(3,pItem)) {
                                                                        sprintf(Buffer,"\"%s\" item -targetid|%d",pItem->Item->Name,CurList->SpawnID);
                                                                        CastTimerWnd->CastCommand((PSPAWNINFO)pCharSpawn, Buffer);
                                                                        break;
                                                                }
                                                        }
                                                        sprintf(Buffer,"\"%s\" %s -targetid|%d",CurList->pSpell->Name,CastTimerWnd->DefaultGem,CurList->SpawnID);
                                                        CastTimerWnd->CastCommand((PSPAWNINFO)pCharSpawn, Buffer);
                                                        break;
                                                }
                                        }
                        } else if( difftime(CurList->timestamp, Now) > CastTimerWnd->AutoRecast ) break;
                        CurList++;
                }
        } else {
                anounceAutoRecast = true;
        }

        if ( (Now < lastRun || Now >= lastRun+1 ) || currentTargetID != lastTargetID) {
                CastTimerWnd->UpdateList(currentTargetID);
                lastRun = Now;
                lastTargetID = currentTargetID;
        }
}

void __stdcall TimerEventOnAnother(unsigned int ID, void *pData, PBLECHVALUE pValues) {
        isParsed=true;
        if(!strlen(pValues->Value)) return;
        bool selfCast=false;
        PSPELL pSpell=NULL;

        PSPAWNINFO sSpawn = NULL;                       // Search Spawn
        PSPAWNINFO pSpawn = NULL;                       // Output Spawn

        SEARCHSPAWN ssSpawn;                            // Search String ( Spawn )
        ClearSearchSpawn(&ssSpawn);
        ssSpawn.ZRadius=SPELLTIMER_SEARCH_ZRADIUS;
        ssSpawn.FRadius=SPELLTIMER_SEARCH_RADIUS;
        for (int i=0; i<SPAWNSEARCHLOOP; i++) {
                if (pSpawn = NthNearestSpawn(&ssSpawn,i,(PSPAWNINFO)pCharSpawn)) if (pSpawn->Type != SPAWN_CORPSE) {
                        if (!stricmp(pValues->Value,pSpawn->DisplayedName)) {
                                //It was not a spell you casted your self, finding the highest level spell and auming that is the right one.
                                if(SpellEventList[(long)pData].pHSpell->ID) {
                                        if(CastingC!=NOID && !CastingLeft()) for (long c=0; c<(long)SpellEventList[(long)pData].pSpells.size(); c++) {
                                                if(CastingC==SpellEventList[(long)pData].pSpells[c]->ID) {
                                                        selfCast=true;
                                                        pSpell=SpellEventList[(long)pData].pSpells[c];
                                                        break;
                                                }
                                        }
                                        if(!selfCast) pSpell=SpellEventList[(long)pData].pHSpell;
                                        if(selfCast) {
                                                if(pSpell->TargetType == 5) AddSpellTimer(CastingO,pSpell,CAST_SUCCESS);
                                                else AddSpellTimer(pSpawn->SpawnID,pSpell,CAST_SUCCESS);
                                        }
                                        else {
                                                #if DEBUGGING
                                                        DebugSpewAlways("MQ2CastTimer::OTHER()");
                                                #endif                                          
                                                AddSpellTimer(pSpawn->SpawnID,pSpell,CAST_SUCCESS,true);
                                        }
                                }
                                return;
                        }
                }
        }
}

void __stdcall TimerEventWornOff(unsigned int ID, void *pData, PBLECHVALUE pValues) {
        isParsed=true;
        char *SpellName=0;
        char *SpawnName=0;
        // find spawn name
        PBLECHVALUE pValue=pValues;
        while(pValue) {
                if (!strcmp(pValue->Name,"SpellName")) SpellName=pValue->Value;
                else if (!strcmp(pValue->Name,"SpawnName")) {
                        SpawnName=pValue->Value;
                        break;
                }
                pValue=pValue->pNext;
        }

        list<TimerSingle>::iterator CurList;
        list<TimerSingle>::iterator EndList;
        list<TimerSingle>::iterator RemoveList;

        int spellCount = 0;
        if (SpellName && SpawnName) {
                CurList=TimerList.begin();
                EndList=TimerList.end();
                while(CurList!=EndList) {
                        if(CurList->SpawnID && CurList->pSpell && CurList->pSpell->ID && !stricmp(SpellName,CurList->pSpell->Name)) {
                                if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(CurList->SpawnID)) {
                                        if(!stricmp(SpawnName,pSpawn->DisplayedName)){
                                                RemoveList = CurList;
                                                if(pSpawn->Type == SPAWN_PLAYER) {
                                                        spellCount = 1;
                                                        break;
                                                } else {
                                                        spellCount++;
                                                        if(spellCount>=2) return;
                                                }
                                        }
                                }
                        }
                        CurList++;
                }
                if (spellCount == 1) {
                        if (CastTimerWnd && CastTimerWnd->ManualRecastCheckBox->Checked && RemoveList->Caster == CASTERME) {
                                TimerSingle MyData;
                                DefaultTimer(MyData);
                                SetTimerData(MyData, RemoveList);
                                MyData.timestamp = time(NULL);

                                char emptyName[64] = {NULL};
                                strcpy(MyData.SpawnName,emptyName);

                                MyData.Caster = CASTERME;

                                RestackTimer(RemoveList,MyData,1);
                        } else ClearOne(RemoveList);
                }
        }
}

void __stdcall TimerCustomEvent(unsigned int ID, void *pData, PBLECHVALUE pValues) {
        isParsed=true;
        char *SpawnName=0;
        // find spawn name
        PBLECHVALUE pValue=pValues;
        while(pValue) {
                if (!strcmp(pValues->Name,"SpawnName")) {
                        SpawnName=pValue->Value;
                        break;
                }
                pValue=pValue->pNext;
        }
 
        if (SpawnName) {
                PSPAWNINFO pSpawn = NULL;                       // Output Spawn
                SEARCHSPAWN ssSpawn;                            // Search String ( Spawn )
                ClearSearchSpawn(&ssSpawn);
                ssSpawn.ZRadius=SPELLTIMER_SEARCH_ZRADIUS;
                ssSpawn.FRadius=SPELLTIMER_SEARCH_RADIUS;
                if(EventList[(long)pData].SpawnType==1) ssSpawn.SpawnType = PC;
                else if(EventList[(long)pData].SpawnType==2) ssSpawn.SpawnType = NPC;
                for (int i=0; i<SPAWNSEARCHLOOP; i++) {
                        if (pSpawn = NthNearestSpawn(&ssSpawn,i,(PSPAWNINFO)pCharSpawn)) if (pSpawn->Type != SPAWN_CORPSE) {
                                if (!stricmp(SpawnName,pSpawn->DisplayedName)) {
                                        AddEventTimer((long)pData,pSpawn->SpawnID);
                                        return;
                                }
                        }
                }
        }
        else AddEventTimer((long)pData,GetCharInfo()->pSpawn->SpawnID);
}

/*
Handel incomming Text lines:
Goal is to make a qualified GUESS on what spell was casted
*/
#if MQ2BLECH == 0
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color) {
        if(!gbInZone || !GetCharInfo() || !GetCharInfo()->pSpawn) return 0;
        isParsed=false;
        CUSTOMEVENT.Feed(Line);
        if (!isParsed && Color==264) CASTONANOTHER.Feed(Line);
        if (!isParsed && Color==284) CASTWORNOFF.Feed(Line);
        return 0;
}
#endif

bool AcceptedSpells(PSPELL pSpell) {
        if(GetSpellDuration(pSpell,GetCharInfo()->pSpawn) <= SPELLDURATSION) return false;
        else if(!pSpell->CastOnAnother[0]) return false;
        else if(!strcmp(pSpell->CastOnAnother," staggers.")) return false;
    return true;
}

/* 
Parser that Select the spells that we want to look for, and save them in SpellEventList 
*/ 
VOID ParseSpellsOnce() {
    if(!gbInZone || SpellEventList.size() || !GetCharInfo() || !GetCharInfo()->pSpawn) return;
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::OTHER()");
        #endif

    int i;                  // Index Var's
    int tcount = 0;         // Total the Total Numbsr's of spells
    int Level=0;

    vector<int>  LastLevel;
    bool noAdd=false;
    PSPELL pSpell=NULL;
    // Parse Throght all the spells to find the spells we wis to parse for in game.
    for (DWORD dwSpellID = 0; dwSpellID < TOTAL_SPELL_COUNT; dwSpellID++) {
        pSpell = &(*((PSPELLMGR)pSpellMgr)->Spells[dwSpellID]);
                if(pSpell->ID && AcceptedSpells(pSpell)) {
            Level=0;
            for (int c=1; c<17; c++) {
                if(pSpell->Level[c-1] <= MAXSPELLLEVEL && pSpell->Level[c-1] > Level) Level = pSpell->Level[c-1];
            }
            if(Level) {
                if(pSpell->SpellType == 1 || pSpell->SpellType == 2 || pSpell->SpellType == 0) {
                    noAdd = false;
                    for (i=0; i<(int)LastLevel.size(); i++) {
                        if(!strcmp(SpellEventList[i].Event,pSpell->CastOnAnother)) {
                            if(Level > LastLevel[i]) {
                                SpellEventList[i].pHSpell = pSpell;
                                LastLevel[i] = Level;
                            }
                                                        SpellEventList[i].pSpells.push_back(pSpell);
                            noAdd = true;
                            break;
                        }
                    }
                    if(!noAdd) {
                        SpellEvent MyData;
                        strcpy (MyData.Event,pSpell->CastOnAnother);
                        MyData.pHSpell = pSpell;
                                                MyData.pSpells.push_back(pSpell);
                        SpellEventList.push_back(MyData);
                        LastLevel.push_back(Level);
                        sprintf(Buffer,"#*#%s",pSpell->CastOnAnother);
                                                #if MQ2BLECH == 0
                                                        CASTONANOTHER.AddEvent(Buffer,TimerEventOnAnother,(void*)((int)SpellEventList.size()-1));
                                                #endif
                                                #if MQ2BLECH == 1
                                                        BleachList.push_back(pMQ2Blech->AddEvent(Buffer,TimerEventOnAnother,(void*)((int)SpellEventList.size()-1)));
                                                #endif
                    }
                }
            }
            tcount++;
        }
    }
        strcpy(Buffer,"Your #SpellName# spell has worn off of #SpawnName#.#*#");
        #if MQ2BLECH == 0
                CASTWORNOFF.AddEvent(Buffer,TimerEventWornOff,(void*)0);
        #endif
        #if MQ2BLECH == 1
                BleachList.push_back(pMQ2Blech->AddEvent(Buffer,TimerEventWornOff,(void*)0));
        #endif
    WriteChatf("[MQ2CastTimer::ParseSpellsOnce] Total Number of spells %d, Spells Selected %d", tcount, SpellEventList.size());
    LoadCostumEvents();
}

PLUGIN_API VOID InitializePlugin(VOID) {
        DebugSpewAlways("MQ2CastTimer::InitializePlugin()");
    AddXMLFile("MQUI_CastTimerWnd.xml");
    AddCommand("/timer",MQTimerCommand);
    AddMQ2Data("SpellTimer",dataTimer);
        if(GetCharInfo() && GetCharInfo()->pSpawn) CreateTimerWindow();
        ClearAll();
}

PLUGIN_API VOID ShutdownPlugin(VOID) { 
        DebugSpewAlways("MQ2CastTimer::ShutdownPlugin()");
        RemoveXMLFile("MQUI_CastTimerWnd.xml");
    RemoveMQ2Data("SpellTimer");
        SpellEventList.clear();
    DestroyTimerWindow(); 
        #if MQ2BLECH == 1
                for(unsigned long i=0; i<BleachList.size(); i++) pMQ2Blech->RemoveEvent(BleachList[i]);
        #endif
    RemoveCommand("/timer");
}

// Called once directly before shutdown of the cleanui system, and also
// every time the game calls CDisplay::CleanGameUI()
PLUGIN_API VOID OnCleanUI(VOID) {
        #if DEBUGGING
                DebugSpewAlways("MQ2ChatWnd::OnCleanUI()");
        #endif  
        DestroyTimerWindow();
}

PLUGIN_API VOID SetGameState(DWORD GameState) {
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::SetGameState()");
        #endif          
        if(gGameState==GAMESTATE_INGAME && gbInZone && GetCharInfo() && GetCharInfo()->pSpawn) CreateTimerWindow();
        else if(gGameState==GAMESTATE_CHARSELECT || gGameState==GAMESTATE_PRECHARSELECT) {
                ClearAll();
                SpellEventList.clear();
                DestroyTimerWindow();
        }
}

PLUGIN_API VOID OnEndZone(VOID) {
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::OnEndZone()");
        #endif          
        if (!CastTimerWnd) {
                
                if (gGameState==GAMESTATE_INGAME) SetGameState(gGameState);
                if (!CastTimerWnd) return;
        }
}

PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn) {
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::OnRemoveSpawn(%s)",pSpawn->Name);
        #endif          
        list<TimerSingle>::iterator CurList=TimerList.begin();
        list<TimerSingle>::iterator EndList=TimerList.end();

        time_t now = time(NULL);

        TimerSingle MyData;
        DefaultTimer(MyData);
        while(CurList!=EndList) {
                if( CurList->SpawnID && CurList->SpawnID == pSpawn->SpawnID ) {
                        eSpawnType eST=GetSpawnType(pSpawn);
                        if( ( eST == PC || eST == MERCENARY ) && CurList->pSpell && CurList->pSpell->ID ) {
                                SetTimerData(MyData, CurList);
                                strcpy(MyData.SpawnName,pSpawn->Name);
                                MyData.TimeOut = now+600;
                                MyData.SpawnID = NOID;

                                RestackTimer(CurList,MyData,-1);
                        } else if ( CurList->SpawnWatch ) {
                                if ( CurList->SpawnWatch == GetCharInfo()->zoneId ) {
                                        TimerSingle MyData;
                                        DefaultTimer(MyData);

                                        SetTimerData(MyData, CurList);

                                        if ( MyData.TimeOut ) MyData.timestamp = now + MyData.TimeOut;
                                        else MyData.timestamp = now;
                                        MyData.SpawnID = NOID;

                                        RestackTimer(CurList,MyData,-1);
                                } else {
                                        CurList++;
                                }
                        } else {
                                ClearOne(CurList);
                        }
                } else {
                        CurList++;
                }
        }
}

PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn) {
        if ( !GetCharInfo()->pSpawn ) return;
        #if DEBUGGING
                DebugSpewAlways("MQ2CastTimer::OnAddSpawn1(%s) = %d",pNewSpawn->Name, pNewSpawn->SpawnID);
        #endif  
        list<TimerSingle>::iterator CurList=TimerList.begin();
        list<TimerSingle>::iterator EndList=TimerList.end();
        
        time_t now = time(NULL);
        char emptyName[64] = {NULL};

        TimerSingle MyData;
        while(CurList!=EndList) {
                DefaultTimer(MyData);
                if(pNewSpawn->SpawnID == GetCharInfo()->pSpawn->SpawnID ) OldSpawnID = pNewSpawn->SpawnID;
                if( !stricmp(pNewSpawn->Name,CurList->SpawnName) && CurList->pSpell && CurList->pSpell->ID ) {
                                SetTimerData(MyData, CurList);

                                strcpy(MyData.SpawnName,emptyName);
                                MyData.TimeOut = 0;                             
                                MyData.SpawnID = pNewSpawn->SpawnID;

                                RestackTimer(CurList,MyData,1);
                } else if ( CurList->SpawnWatch && CurList->SpawnWatch == GetCharInfo()->zoneId && CurList->SpawnID==NOID ) {
                        int distXY = 1;
                        int distZ = 15;
                        if ( pNewSpawn->X >= (CurList->SpawnWatchX-distXY) && pNewSpawn->X <= (CurList->SpawnWatchX+distXY) ) {
                                #if DEBUGGING
                                        WriteChatf("[MQ2CastTimer::OnAddSpawn] WATCH SPAWN X %d ::: %f ",pNewSpawn->X,CurList->SpawnWatchX);
                                #endif  
                                if ( pNewSpawn->Y >= (CurList->SpawnWatchY-distXY) && pNewSpawn->Y <= (CurList->SpawnWatchY+distXY) ) {
                                        #if DEBUGGING
                                                WriteChatf("[MQ2CastTimer::OnAddSpawn] WATCH SPAWN Y %d ::: %f ",pNewSpawn->Y,CurList->SpawnWatchY);
                                        #endif  
                                        if ( pNewSpawn->Z >= (CurList->SpawnWatchZ-distZ) && pNewSpawn->Z <= (CurList->SpawnWatchZ+distZ) ) {
                                                SetTimerData(MyData,CurList);

                                                if ( IsNamed(pNewSpawn) ) strcpy(MyData.SpawnName,pNewSpawn->Name);

                                                if ( MyData.TimeOut == 0 ) MyData.TimeOut = now - CurList->timestamp;
                                                MyData.timestamp = now + MyData.TimeOut;

                                                MyData.SpawnID = pNewSpawn->SpawnID;

                                                RestackTimer(CurList,MyData,1);
                                        } else {
                                                #if DEBUGGING
                                                        WriteChatf("[MQ2CastTimer::OnAddSpawn] WATCH SPAWN Z %d ::: %f ",pNewSpawn->Z,CurList->SpawnWatchZ);
                                                #endif  
                                        }
                                }
                        }
                }
                CurList++;
        }
}
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
