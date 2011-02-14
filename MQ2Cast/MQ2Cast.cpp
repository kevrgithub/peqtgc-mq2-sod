//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Projet: MQ2Cast.cpp          | Set DEBUGGING 0 or 1 (false/true) for DEBUGGING msg
// Author: s0rCieR                        | 
//                         A_Enchanter_00 |
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
// Last edited by: s0rCier 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
#define       DEBUGGING         0
 
#ifndef PLUGIN_API
  #include "../MQ2Plugin.h"
  #include "../Blech/Blech.h"
  PreSetup("MQ2Cast");
  PLUGIN_VERSION(8.1017);
#endif
 
#define       DELAY_CAST    12000
#define       DELAY_MEMO     6000
#define       DELAY_STOP     3000
#define       DELAY_PULSE     125
 
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
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
bool          Immobile = false;        // Immobile?
bool          Invisible= false;        // Invisibility Check?
bool          Twisting = false;        // Twisting?
bool          Casting  = false;        // Casting Window was opened?
long          Resultat = CAST_SUCCESS; // Resultat
long          ImmobileT= 0;            // Estimate when it be immobilized!
 
long          CastingD=NOID;           // Casting Spell Detected
long          CastingC=NOID;           // Casting Current ID
long          CastingE=CAST_SUCCESS;   // Casting Current Result
long          CastingL=NOID;           // Casting LastOne ID
long          CastingX=CAST_SUCCESS;   // Casting LastOne Result
long          CastingT=0;              // Casting Timeout
long          CastingO=NOID;           // Casting OnTarget
long          CastingP=0;              // Casting Pulse
 
long          TargI=0;                 // Target ID
long          TargC=0;                 // Target Current
 
long          StopF=FLAG_COMPLETE;     // Stop Event Flag Progress? 
long          StopE=DONE_SUCCESS;      // Stop Event Exit Value 
long          StopM=0;                 // Stop Event Mark 
 
long          MoveAP=FLAG_COMPLETE;        // Move Event MQ2AdvPath? 
long          MoveA=FLAG_COMPLETE;     // Move Event AdvPath? 
long          MoveS=FLAG_COMPLETE;     // Move Event Stick? 
 
long          MemoF=FLAG_COMPLETE;     // Memo Event Flag 
long          MemoE=DONE_SUCCESS;      // Memo Event Exit 
long          MemoM=0;                 // Memo Event Mark 
 
long          ItemF=FLAG_COMPLETE;     // Item Flag
long          ItemA[NUM_INV_SLOTS];         // Item Arrays
 
long          DuckF=FLAG_COMPLETE;     // Duck Flag
long          DuckM=0;                 // Duck Time Stamp
 
long          CastF=FLAG_COMPLETE;     // Cast Flag
long          CastE=CAST_SUCCESS;      // Cast Exit Return value
long          CastG=NOID;              // Cast Gem ID
void         *CastI=NULL;              // Cast ID   [spell/alt/item]
long          CastK=NOID;              // Cast Kind [spell/alt/item]
long          CastT=0;                 // Cast Time [spell/alt/item]
long          CastM=0;                 // Cast TimeMark Start Casting
long          CastR=0;                 // Cast Retry Counter
long          CastW=0;                 // Cast Retry Type
char          CastB[MAX_STRING];       // Cast Bandolier In
char          CastC[MAX_STRING];       // Cast SpellType
char          CastN[MAX_STRING];       // Cast SpellName
PSPELL        CastS=NULL;              // Cast Spell Pointer
 
bool          Parsed=false;            // BTree List Found Flags
Blech         LIST013('#');            // BTree List for OnChat Message on Color  13
Blech         LIST264('#');            // BTree List for OnChat Message on Color 264
Blech         LIST289('#');            // BTree List for OnChat Message on Color 289
Blech         UNKNOWN('#');            // BTree List for OnChat Message on UNKNOWN Yet Color
Blech         SUCCESS('#');            // BTree List for OnChat Message on SUCCESS Detection
 
PCONTENTS     fPACK=0;                 // ItemFound/ItemSearch - Find Pack Contents
PCONTENTS     fITEM=0;                 // ItemFound/ItemSearch - Find Item Contents
long          fSLOT=0;                 // ItemFound/ItemSearch - Find Item SlotID
 
PSPELL        fFIND;                   // SpellFind - Casting Spell Effect
void         *fINFO;                   // SpellFind - Casting Type Structure
int           fTYPE;                   // SpellFind - Casting Type
int           fTIME;                   // SpellFind - Casting Time
PCHAR         fNAME;                   // SpellFind - Casting Name
 
SPELLFAVORITE SpellToMemorize;         // Favorite Spells Array
long          SpellTotal;              // Favorite Spells Total
 
PCHAR         ListGems[]={"1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"};
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
#define aCastEvent(List,Value,Filter) List.AddEvent(Filter,CastEvent,(void*)Value);
 
void __stdcall CastEvent(unsigned int ID, void *pData, PBLECHVALUE pValues) {
  Parsed=true;
  if(CastingE<(long)pData) CastingE=(long)pData;
  #if DEBUGGING
    WriteChatf("[%d] MQ2Cast:[OnChat]: Result=[%d] Called=[%d].",(long)clock(),CastingE,(long)pData);
  #endif
}
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
PALTABILITY AltAbility(PCHAR ID) {
  if(ID[0]) {
    int Number=IsNumber(ID);
    int Values=atoi(ID);
    for(DWORD nAbility=0; nAbility < AA_CHAR_MAX_REAL; nAbility++) {
      if(GetCharInfo2()->AAList[nAbility].AAIndex) if(PALTABILITY pAbility=pAltAdvManager->GetAltAbility(GetCharInfo2()->AAList[nAbility].AAIndex)) {
        if(Number) {
          if(pAbility->ID == Values) return pAbility;
        } else if(PCHAR pName=pCDBStr->GetString(pAbility->nName,1,NULL)) {
          if(!stricmp(ID,pName)) return pAbility;
        }
      }
    }
  }
  return NULL;
}
 
void Bandolier(PCHAR zFormat, ...) {
  typedef VOID (__cdecl *BandolierCALL) (PCHAR);
  char zOutput[MAX_STRING]; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList);
  PMQPLUGIN pLook=pPlugins;
  while(pLook && strnicmp(pLook->szFilename,"MQ2Bandolier",12)) pLook=pLook->pNext;
  if(pLook && pLook->fpVersion>1.000)
    if(BandolierCALL Request=(BandolierCALL)GetProcAddress(pLook->hModule,"doBandolier"))
      Request(zOutput);
}
 
bool BardClass() {
  return (strncmp(pEverQuest->GetClassDesc(GetCharInfo2()->Class & 0xFF),"Bard",5))?false:true;
}
 
void Cast(PCHAR zFormat, ...) {
  char zOutput[MAX_STRING]={0}; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList); if(!zOutput[0]) return;
  Cast(GetCharInfo()->pSpawn,zOutput);
}

long CastingLeft() {
  long CL=0;
  if(pCastingWnd && (PCSIDLWND)pCastingWnd->Show) {
    CL=GetCharInfo()->pSpawn->CastingData.SpellETA - GetCharInfo()->pSpawn->TimeStamp;
    if(CL<1) CL=1;
  }
  return CL;
}
long Evaluate(PCHAR zFormat, ...) {
  char zOutput[MAX_STRING]={0}; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList); if(!zOutput[0]) return 1;
  ParseMacroData(zOutput); return atoi(zOutput);
}
 
void CastTimer(int TargetID,int SpellID,long TickE) {
  typedef VOID (__cdecl *CastTimerCALL) (int,int,long);
  PMQPLUGIN pLook=pPlugins;
  while(pLook && strnicmp(pLook->szFilename,"MQ2Casttimer",11)) pLook=pLook->pNext;
  if(pLook && pLook->fpVersion>1.1100)
    if(CastTimerCALL Request=(CastTimerCALL)GetProcAddress(pLook->hModule,"TimerCastHandle"))
      Request(TargetID, SpellID, TickE);
}
 
void Exchange(PCHAR zFormat, ...) {
  typedef VOID (__cdecl *ExchangeCALL) (PCHAR);
  char zOutput[MAX_STRING]; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList);
  PMQPLUGIN pLook=pPlugins;
  while(pLook && strnicmp(pLook->szFilename,"MQ2Exchange",11)) pLook=pLook->pNext;
  if(pLook && pLook->fpVersion>1.1500)
    if(ExchangeCALL Request=(ExchangeCALL)GetProcAddress(pLook->hModule,"doExchange"))
      Request(zOutput);
}

void Execute(PCHAR zFormat, ...) {
  char zOutput[MAX_STRING]={0}; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList); if(!zOutput[0]) return;
  DoCommand(GetCharInfo()->pSpawn,zOutput);
}
 
bool Flags() {
  if(!BardClass() && pCastingWnd && (PCSIDLWND)pCastingWnd->Show) return true;
  if(CastF!=FLAG_COMPLETE) return true;
  if(DuckF!=FLAG_COMPLETE) return true;
  if(ItemF!=FLAG_COMPLETE) return true;
  if(MemoF!=FLAG_COMPLETE) return true;
  if(StopF!=FLAG_COMPLETE) return true;
  if(MoveS!=FLAG_COMPLETE) return true;
  if(MoveA!=FLAG_COMPLETE) return true;
  if(MoveAP!=FLAG_COMPLETE) return true;
  return false;
} 
 
long GEMID(DWORD ID) {
  for(int GEM=0; GEM < NUM_SPELL_GEMS; GEM++) if(GetCharInfo2()->MemorizedSpells[GEM] == ID) return GEM;
  return NOID;
}
 
bool GEMReady(DWORD ID) {
  if(pCastSpellWnd && ID < NUM_SPELL_GEMS)
    if((long)((PEQCASTSPELLWINDOW)pCastSpellWnd)->SpellSlots[ID]->spellicon!=NOID)
                if(BardClass() || (long)((PEQCASTSPELLWINDOW)pCastSpellWnd)->SpellSlots[ID]->spellstate!=1) return true;
  return false;
}

bool GiftOfMana() {
        for (unsigned long nBuff=0 ; nBuff < 35 ; nBuff++) {
                if (PSPELL pSpell=GetSpellByID(GetCharInfo2()->ShortBuff[nBuff].SpellID)) {
                        if (!stricmp("Gift of Mana",pSpell->Name)) {
                                return true;
                        }
                }
        }
        return false;
}

long ItemFound(long ID, long B, long E) {
  for(int iSlot=B; iSlot < E; iSlot++)
    if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
      if(ID == cSlot->Item->ItemNumber) {
        fSLOT=iSlot;
        fITEM=cSlot;
        fPACK=(iSlot>=BAG_SLOT_START)?cSlot:NULL;
        return true;
      } else if (cSlot->Item->Type == ITEMTYPE_PACK) {
        for(int iPack=0; iPack < cSlot->Item->Slots; iPack++) {
          if(PCONTENTS cPack=cSlot->Contents[iPack]) {
            if(ID == cPack->Item->ItemNumber) {
              fSLOT=(iSlot-BAG_SLOT_START)*10+262+iPack;
              fITEM=cPack; 
              fPACK=cSlot;
              return true;
            }
          }
        }
      }
    }
  fSLOT=0;
  fITEM=0;
  fPACK=0;
  return false;
}

long ItemSearch(PCHAR ID, long B, long E) {
  if(IsNumber(ID)) return ItemFound(atoi(ID),B,E);
  for(int iSlot=B; iSlot < E; iSlot++)
    if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
      if(!stricmp(ID,cSlot->Item->Name)) {
        fSLOT=iSlot;
        fITEM=cSlot;
        fPACK=(iSlot>=BAG_SLOT_START)?cSlot:NULL;
        return true;
      } else if (cSlot->Item->Type == ITEMTYPE_PACK) {
        for(int iPack=0; iPack < cSlot->Item->Slots; iPack++) {
          if(PCONTENTS cPack=cSlot->Contents[iPack]) {
            if(!stricmp(ID,cPack->Item->Name)) {
              fSLOT=(iSlot-BAG_SLOT_START)*10+262+iPack;
              fITEM=cPack; 
              fPACK=cSlot;
              return true;
            }
          }
        }
      }
    }
  fSLOT=0;
  fITEM=0;
  fPACK=0;
  return false;
} 
 
void MemoLoad(long Gem, PSPELL Spell) {
  if(!Spell || Spell->Level[GetCharInfo2()->Class-1]>GetCharInfo2()->Level) return;
  for(int sp=0;sp<NUM_SPELL_GEMS;sp++)
    if(SpellToMemorize.SpellId[sp]==Spell->ID) SpellToMemorize.SpellId[sp]=0xFFFFFFFF;
  SpellToMemorize.SpellId[((DWORD)Gem<NUM_SPELL_GEMS)?Gem:4]=Spell->ID;
}
 
float Speed() {
  float MySpeed=0.0f;
  if(PSPAWNINFO Self=GetCharInfo()->pSpawn) {
    MySpeed=Self->SpeedRun;
    if(PSPAWNINFO Mount=FindMount(Self))
      MySpeed=Mount->SpeedRun;
  }
  return MySpeed;
}
 
void Success(PSPELL Cast) {
  SUCCESS.Reset();
  if(Cast) {
    char Temps[MAX_STRING];
    bool Added=false;
    if(Cast->CastOnYou[0]) {
      sprintf(Temps,"%s#*#",Cast->CastOnYou);
      aCastEvent(SUCCESS,CAST_SUCCESS,Temps);
      Added=true;
    }
    if(Cast->CastOnAnother[0]) {
      sprintf(Temps,"#*#%s#*#",Cast->CastOnAnother);
      aCastEvent(SUCCESS,CAST_SUCCESS,Temps);
      Added=true;
    }
    if(!Added)
      aCastEvent(SUCCESS,CAST_SUCCESS,"You begin casting#*#");
  }
}
 
bool Moving() {
  long  MyTimer=(long)clock();
  if(Speed()!=0.0f) ImmobileT=MyTimer+500;
  return (!MQ2Globals::gbMoving && (!ImmobileT || MyTimer>ImmobileT));
}
 
BOOL Open(PCHAR WindowName) {
  PCSIDLWND pWnd=(PCSIDLWND)FindMQ2Window(WindowName);
  return (!pWnd)?false:(BOOL)pWnd->Show;
}
 
BOOL Paused() {
  if(pLootWnd && (PCSIDLWND)pLootWnd->Show)         return true;
  if(pBankWnd && (PCSIDLWND)pBankWnd->Show && !BardClass())         return true;
  if(pMerchantWnd && (PCSIDLWND)pMerchantWnd->Show && !BardClass()) return true;
  if(pTradeWnd && (PCSIDLWND)pTradeWnd->Show)       return true;
  if(pGiveWnd && (PCSIDLWND)pGiveWnd->Show)         return true;
  if(Open("TributeMasterWnd"))      return true;
  if(Open("GuildTributeMasterWnd")) return true;
  if(Open("GuildBankWnd"))          return true;
  return false;
}
 
void Reset() {
  TargI=0;                 // Target ID
  TargC=0;                 // Target Check ID
  StopF=FLAG_COMPLETE;     // Stop Event Flag Progress? 
  StopE=DONE_SUCCESS;      // Stop Event Exit Value 
  MoveAP=FLAG_COMPLETE;    // Stop Event MQ2AdvPath? 
  MoveA=FLAG_COMPLETE;     // Stop Event AdvPath? 
  MoveS=FLAG_COMPLETE;     // Stop Event Stick? 
  MemoF=FLAG_COMPLETE;     // Memo Event Flag 
  MemoE=DONE_SUCCESS;      // Memo Event Exit 
  ItemF=FLAG_COMPLETE;     // Item Flag
  DuckF=FLAG_COMPLETE;     // Duck Flag
  CastF=FLAG_COMPLETE;     // Cast Flag
  CastE=CAST_SUCCESS;      // Cast Exit Return value
  CastG=NOID;              // Cast Gem ID
  CastI=NULL;              // Cast ID   [spell/alt/item/disc]
  CastK=NOID;              // Cast Kind [spell/alt/item/disc] [-1=unknown]
  CastT=0;                 // Cast Time [spell/alt/item/disc]
  CastB[0]=0;              // Cast Bandolier In
  CastB[0]=0;              // Cast Bandolier Out
  CastC[0]=0;              // Cast SpellType
  CastN[0]=0;              // Cast SpellName
  CastR=1;                 // Cast Retry Counter
  CastW=0;                 // Cast Retry Type
  Invisible=false;         // Invisibility Check?
  ZeroMemory(&SpellToMemorize,sizeof(SPELLFAVORITE));
  strcpy(SpellToMemorize.Name,"Mem a Spell");
  SpellToMemorize.inuse=1;
  for(int sp=0;sp<NUM_SPELL_GEMS;sp++) SpellToMemorize.SpellId[sp]=0xFFFFFFFF;
  SpellTotal=0;
}
 
long SlotEquip(PITEMINFO Item, long CurrSlotID, long WantSlotID) {
  BYTE Effects=((long)Item->Clicky.SpellID==NOID)?0:Item->Clicky.EffectType;
  long MaxSlot=(Effects==1 || Effects==3 || Effects==5)?NUM_INV_SLOTS-1:BAG_SLOT_START-1;
  if(CurrSlotID<=MaxSlot) return CurrSlotID;
  if(WantSlotID>NOID && WantSlotID<=MaxSlot) {
    if(WantSlotID>BAG_SLOT_START-1) {
      PCONTENTS pSlot=GetCharInfo2()->InventoryArray[WantSlotID];
      if(!pSlot || pSlot->Item->Type!=ITEMTYPE_PACK) return WantSlotID;
    } else if(Item->EquipSlots&(1<<WantSlotID)) return WantSlotID;
  }
  for(long Desired=MaxSlot; Desired>=0; Desired--) {
    if(Desired>BAG_SLOT_START-1) {
      PCONTENTS pSlot=GetCharInfo2()->InventoryArray[Desired];
      if (!pSlot || pSlot->Item->Type!=ITEMTYPE_PACK) return Desired;
    } else if(Item->EquipSlots&(1<<Desired)) return Desired;
  }
  return NOID;
}
 
long SlotID(PCHAR ID) {
  long Search=IsNumber(ID); DWORD Number=atoi(ID);
  if(Search) return (Number<NUM_INV_SLOTS)?Number:NOID;
  for(Number=0; szItemSlot[Number]; Number++) if(!stricmp(ID,szItemSlot[Number])) return Number;
  return NOID;
}
 
PSPELL SpellBook(PCHAR ID) {
  if(ID[0]) {
    if(IsNumber(ID)) {
      int Number=atoi(ID);
      for(DWORD nSpell=0; nSpell < NUM_BOOK_SLOTS; nSpell++)
        if(GetCharInfo2()->SpellBook[nSpell]==Number)
          return GetSpellByID(Number);
    } else {
      for(DWORD nSpell=0; nSpell < NUM_BOOK_SLOTS; nSpell++)
        if(PSPELL pSpell=GetSpellByID(GetCharInfo2()->SpellBook[nSpell]))
                if(!stricmp(ID,pSpell->Name)) return pSpell;
    }
  }
  return NULL;
}
 
bool SpellFind(PCHAR ID, PCHAR TYPE) {
  if(ID[0]) {
    // assume it's an alt ability
    if(!TYPE[0] || !strnicmp(TYPE,"alt",3)) {
      if(PALTABILITY Search=AltAbility(ID)) {
        if(PSPELL spell=GetSpellByID(Search->SpellID)) {
          fFIND=spell; 
          fINFO=Search; 
          fTIME=fFIND->CastTime; 
          fNAME=(PCHAR)fFIND->Name; 
          fTYPE=TYPE_ALT; 
          return true; 
        } 
      }
    }
    // assume it's a spell
    if(!TYPE[0] || !strnicmp(TYPE,"gem",3) || IsNumber(TYPE)) {
      if(PSPELL Search=SpellBook(ID)) {
        fFIND=Search;
        fINFO=Search;
        fTIME=pCharData1->GetAACastingTimeModifier((EQ_Spell*)fFIND)+
              pCharData1->GetFocusCastingTimeModifier((EQ_Spell*)fFIND,0)+
              fFIND->CastTime;
        fNAME=(PCHAR)fFIND->Name;
        fTYPE=TYPE_SPELL;
        return true;
      }
    }
    // assume it's an item clicky
    if(ItemSearch(ID,0,NUM_INV_SLOTS)) if(fITEM->Item->Clicky.SpellID) {
      fFIND=GetSpellByID(fITEM->Item->Clicky.SpellID);
      fINFO=fITEM;
      fTIME=fITEM->Item->CastTime;
      fNAME=(PCHAR)fITEM->Item->Name;
      fTYPE=TYPE_ITEM;
      return true;
    }
  }
  fFIND=NULL;
  fINFO=NULL;
  fTYPE=0;
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
 
bool SpellReady(PCHAR ID) { 
        if(ID[0]==0) return true;
  if(IsNumber(ID)) {
    long number=atoi(ID)-1;
        if((DWORD)number<NUM_SPELL_GEMS) return GEMReady(number);
  }
  if(ID[0]=='M' && strlen(ID)==1) return !(Twisting=Evaluate("${If[${Twist.Twisting},1,0]}")?true:false);
  char zName[MAX_STRING]; GetArg(zName,ID,1,FALSE,FALSE,FALSE,'|');
  char zType[MAX_STRING]; GetArg(zType,ID,2,FALSE,FALSE,FALSE,'|');
  if(SpellFind(zName,zType)) if(!SpellTimer(fTYPE,fINFO)) return true;
  return false;
}
 
void Stick(PCHAR zFormat, ...) {
  typedef VOID (__cdecl *StickCALL) (PSPAWNINFO,PCHAR);
  char zOutput[MAX_STRING]; va_list vaList; va_start(vaList,zFormat);
  vsprintf(zOutput,zFormat,vaList);
  PMQPLUGIN pLook=pPlugins;
  while(pLook && strnicmp(pLook->szFilename,"MQ2MoveUtils",12)) pLook=pLook->pNext;
  if(pLook && pLook->fpVersion>0.9999 && pLook->RemoveSpawn)
    if(StickCALL Request=(StickCALL)GetProcAddress(pLook->hModule,"StickCommand"))
      Request(GetCharInfo()->pSpawn,zOutput);
}
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
class MQ2CastType *pCastType=0; 
class MQ2CastType : public MQ2Type {
private:
  char Temps[MAX_STRING];
public:
  enum CastMembers {
    Active=1,
    Effect=2,
    Stored=3,
    Result=4,
    Return=5,    
    Status=6,
    Timing=7,
    Taken=8,
    Ready=9,
  };
  MQ2CastType():MQ2Type("Cast") {
    TypeMember(Active);
    TypeMember(Effect);
    TypeMember(Stored);
    TypeMember(Result);
    TypeMember(Return);
    TypeMember(Status);
    TypeMember(Timing);
    TypeMember(Taken);
    TypeMember(Ready);
  }
  bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
    PMQ2TYPEMEMBER pMember=MQ2CastType::FindMember(Member); 
    if(pMember) switch((CastMembers)pMember->ID) {
      case Active:
        Dest.DWord=(gbInZone);
        Dest.Type=pBoolType;
        return true;
      case Effect:
        Dest.DWord=GetCharInfo()->pSpawn->CastingData.SpellID;
        if((long)Dest.DWord==NOID && CastF!=FLAG_COMPLETE) Dest.DWord=CastS->ID;
        if((long)Dest.DWord!=NOID) {
          Dest.Ptr=GetSpellByID(Dest.DWord); 
          Dest.Type=pSpellType; 
        }
        return true; 
      case Stored:
        if(CastingL!=NOID) {
          Dest.Ptr=GetSpellByID(CastingL);
          Dest.Type=pSpellType;
        } 
        return true;
      case Timing:
        Dest.DWord=(DWORD)CastingLeft(); 
        Dest.Type=pIntType;
        return true;
      case Status: 
        strcpy(Temps,"");               
        if( CastingC!=NOID ||
                        CastF!=FLAG_COMPLETE || 
                        (pCastingWnd && (PCSIDLWND)pCastingWnd->Show)) strcat(Temps,"C");
        if(StopF!=FLAG_COMPLETE) strcat(Temps,"S");
        if(MoveA!=FLAG_COMPLETE) strcat(Temps,"A");
        if(MoveAP!=FLAG_COMPLETE) strcat(Temps,"P");
        if(MoveS!=FLAG_COMPLETE) strcat(Temps,"F");
        if(MemoF!=FLAG_COMPLETE) strcat(Temps,"M");
        if(DuckF!=FLAG_COMPLETE) strcat(Temps,"D");
        if(ItemF!=FLAG_COMPLETE) strcat(Temps,"E");
        if(!Temps[0]) strcat(Temps,"I");
        Dest.Ptr=Temps; 
        Dest.Type=pStringType; 
        return true;
      case Result:
      case Return:
        switch((pMember->ID==Result)?CastingX:Resultat) {
          case DONE_PROGRESS:
          case CAST_SUCCESS:      strcpy(Temps,"CAST_SUCCESS");     break;
          case CAST_INTERRUPTED:  strcpy(Temps,"CAST_INTERRUPTED"); break;
          case CAST_RESIST:       strcpy(Temps,"CAST_RESIST");      break;
          case CAST_COLLAPSE:     strcpy(Temps,"CAST_COLLAPSE");    break;
          case CAST_RECOVER:      strcpy(Temps,"CAST_RECOVER");     break;
          case CAST_FIZZLE:       strcpy(Temps,"CAST_FIZZLE");      break; 
          case CAST_STANDING:     strcpy(Temps,"CAST_STANDING");    break; 
          case CAST_STUNNED:      strcpy(Temps,"CAST_STUNNED");     break;
          case CAST_INVISIBLE:    strcpy(Temps,"CAST_INVISIBLE");   break;
          case CAST_NOTREADY:     strcpy(Temps,"CAST_NOTREADY");    break;
          case CAST_OUTOFMANA:    strcpy(Temps,"CAST_OUTOFMANA");   break;
          case CAST_OUTOFRANGE:   strcpy(Temps,"CAST_OUTOFRANGE");  break;
          case CAST_NOTARGET:     strcpy(Temps,"CAST_NOTARGET");    break; 
          case CAST_CANNOTSEE:    strcpy(Temps,"CAST_CANNOTSEE");   break;
          case CAST_COMPONENTS:   strcpy(Temps,"CAST_COMPONENTS");  break;
          case CAST_OUTDOORS:     strcpy(Temps,"CAST_OUTDOORS");    break;
          case CAST_TAKEHOLD:     strcpy(Temps,"CAST_TAKEHOLD");    break; 
          case CAST_IMMUNE:       strcpy(Temps,"CAST_IMMUNE");      break; 
          case CAST_DISTRACTED:   strcpy(Temps,"CAST_DISTRACTED");  break;
          case CAST_ABORTED:      strcpy(Temps,"CAST_CANCELLED");   break;
          case CAST_UNKNOWN:      strcpy(Temps,"CAST_UNKNOWN");     break; 
          default:                strcpy(Temps,"CAST_NEEDFIXTYPE"); break; 
        }
        Dest.Ptr=Temps; 
        Dest.Type=pStringType; 
        return true; 
      case Ready:
        Dest.DWord=(gbInZone && !Flags() && !Paused() && !Open("SpellBookWnd") && 
                    !(GetCharInfo()->Stunned) && SpellReady(Index));
        Dest.Type=pBoolType;
        return true;
      case Taken:
        Dest.DWord=(CastingX==CAST_TAKEHOLD);
        Dest.Type=pBoolType;
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
  ~MQ2CastType() { } 
}; 
 
BOOL dataCast(PCHAR szName, MQ2TYPEVAR &Dest) {
  Dest.DWord=1;
  Dest.Type=pCastType;
  return true;
}
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
void StopEnding() {
  if(MoveS!=FLAG_COMPLETE) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: Stick UnPause Request.",(long)clock());
    #endif
    Stick("unpause");
    MoveS=FLAG_COMPLETE;
  }
  if(MoveA!=FLAG_COMPLETE) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: AdvPath UnPause Request.",(long)clock());
    #endif
    Execute("/varcalc PauseFlag 0");
    MoveA=FLAG_COMPLETE;
  }
  if(MoveAP!=FLAG_COMPLETE) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: MQ2AdvPath UnPause Request.",(long)clock());
    #endif
    Execute("/afollow unpause");
    MoveAP=FLAG_COMPLETE;
  }
}
 
void StopHandle() {
  if(StopF==FLAG_REQUEST) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: Request.",(long)clock());
    #endif
    StopM=(long)clock()+DELAY_STOP;
    StopF=FLAG_PROGRESS1;
    StopE=DONE_PROGRESS;
  }
  if(Evaluate("${If[${Stick.Status.Equal[ON]},1,0]}")) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: Stick Pause Request.",(long)clock());
    #endif
    Stick("pause");
    MoveS=FLAG_PROGRESS1;
  }
  if(Evaluate("${If[${Bool[${FollowFlag}]},1,0]}")) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: AdvPath Pause Request.",(long)clock());
    #endif
    Execute("/varcalc PauseFlag 1");
    MoveA=FLAG_PROGRESS1;
  }
  if( MoveAP!=FLAG_PROGRESS1 && ( Evaluate("${If[${Bool[${AdvPath.Following}]},1,0]}") || Evaluate("${If[${Bool[${AdvPath.Playing}]},1,0]}") ) ) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: MQ2AdvPath Pause Request.",(long)clock());
    #endif
    Execute("/afollow pause");
    MoveAP=FLAG_PROGRESS1;
  }
  if(Immobile=Moving()) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Immobilize]: Complete.",(long)clock());
    #endif
    StopF=FLAG_COMPLETE;
    StopE=DONE_SUCCESS;
  }
  if((long)clock() > StopM) {
    WriteChatf("[%d] MQ2Cast:[Immobilize]: Aborting!",(long)clock());
    StopF=FLAG_COMPLETE;
    StopE=DONE_ABORTED;
    return;
  }
  if(StopF==FLAG_PROGRESS1) {
    StopF=FLAG_PROGRESS2;
    if(Speed()!=0.0f) {
      MQ2Globals::ExecuteCmd(FindMappableCommand("back"),1,0);
      MQ2Globals::ExecuteCmd(FindMappableCommand("back"),0,0);
    }
  }
}
 
void MemoHandle() {
  if(!pSpellBookWnd) MemoE=DONE_ABORTED;
  else {
    bool Complete=true;
    for(int sp=0; sp<NUM_SPELL_GEMS; sp++)
      if(SpellToMemorize.SpellId[sp]!=0xFFFFFFFF &&
         SpellToMemorize.SpellId[sp]!=
         GetCharInfo2()->MemorizedSpells[sp]) {
        Complete=false;
        break;
      }
    if(!Complete) {
      if(MemoF==FLAG_REQUEST) {
        #if DEBUGGING 
          WriteChatf("[%d] MQ2Cast:[Memorize]: Immobilize.",(long)clock());
        #endif
        MemoF=FLAG_PROGRESS1; 
        MemoE=DONE_PROGRESS;
        MemoM=(long)clock()+DELAY_STOP+DELAY_MEMO*SpellTotal;
        if(StopF==FLAG_COMPLETE) StopE=DONE_SUCCESS;
        if(StopF==FLAG_COMPLETE) StopF=FLAG_REQUEST;
        if(StopF!=FLAG_COMPLETE) StopHandle();
      }
      if(MemoF==FLAG_PROGRESS1 && StopE==DONE_SUCCESS) {
        #if DEBUGGING
          WriteChatf("[%d] MQ2Cast:[Memorize]: Spell(s).",(long)clock());
        #endif
        MemoF=FLAG_PROGRESS2;
        DWORD Favorite=(DWORD)&SpellToMemorize;
        pSpellBookWnd->MemorizeSet((int*)Favorite,NUM_SPELL_GEMS);
      }
      if(StopE==DONE_ABORTED || (long)clock()>MemoM) MemoE=DONE_ABORTED;
    } else {
      #if DEBUGGING
        WriteChatf("[%d] MQ2Cast:[Memorize]: Complete.",(long)clock());
      #endif
      MemoF=FLAG_COMPLETE;
      MemoE=DONE_SUCCESS;
    }
  }
  if(MemoE==DONE_ABORTED || !pSpellBookWnd) {
    WriteChatf("[%d] MQ2Cast:[Memorize]: Aborting!",(long)clock());
    MemoF=FLAG_COMPLETE;
  }
  if(MemoF==FLAG_COMPLETE && (pSpellBookWnd && (PCSIDLWND)pSpellBookWnd->Show)) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Memorize]: Closebook.",(long)clock());
    #endif
    Execute("/book");
  }
}
 
void ItemHandle(bool SwapIn) {
  if(GetCharInfo2()->Cursor || 
    (pCastingWnd && (PCSIDLWND)pCastingWnd->Show) || 
    (pSpellBookWnd && (PCSIDLWND)pSpellBookWnd->Show)
    ) return;
  if(!SwapIn && ItemF!=FLAG_COMPLETE && !GetCharInfo()->pSpawn->CastingData.SpellETA) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Swapping]: Out.",(long)clock());
    #endif
    for(int X=0;X<NUM_INV_SLOTS;X++) 
      if(ItemA[X]) Exchange("%d %d",ItemA[X],X);
    for(int Y=0;Y<NUM_INV_SLOTS;Y++) 
      if(PCONTENTS Cont=GetCharInfo2()->InventoryArray[Y])
        if(ItemA[Y] && Cont->Item->ItemNumber==ItemA[Y]) ItemA[Y]=0;
    ItemF=FLAG_COMPLETE;
    for(int Z=0;Z<NUM_INV_SLOTS && ItemF==FLAG_COMPLETE;Z++) if(ItemA[Z]) ItemF=FLAG_PROGRESS1;
  }
  if(SwapIn && ItemF==FLAG_COMPLETE && (CastB[0] || CastK==TYPE_ITEM)) {
    memset(&ItemA,0,sizeof(ItemA));
    for(int X=0;X<NUM_INV_SLOTS;X++)
      if(PCONTENTS Cont=GetCharInfo2()->InventoryArray[X])
        if(Cont->Item->Type!=ITEMTYPE_PACK)
          ItemA[X]=Cont->Item->ItemNumber;
    if(CastB[0]) Bandolier(CastB);
    if(CastK==TYPE_ITEM) {
      if(ItemFound(((PCONTENTS)CastI)->Item->ItemNumber,0,NUM_INV_SLOTS)) {
        long wSLOT=SlotID(CastC);
        long eSLOT=SlotEquip(fITEM->Item,fSLOT,wSLOT);
        if(fSLOT!=eSLOT) Exchange("%d %d",((PCONTENTS)CastI)->Item->ItemNumber,eSLOT);
      }
    }
    for(int Y=0;Y<NUM_INV_SLOTS;Y++)
      if(PCONTENTS Cont=GetCharInfo2()->InventoryArray[Y])
        if(Cont->Item->Type==ITEMTYPE_PACK || Cont->Item->ItemNumber==ItemA[Y]) ItemA[Y]=0;
    for(int Z=0;Z<NUM_INV_SLOTS && ItemF==FLAG_COMPLETE;Z++) if(ItemA[Z]) ItemF=FLAG_PROGRESS1;
    if(ItemF!=FLAG_COMPLETE) {
      #if DEBUGGING
        WriteChatf("[%d] MQ2Cast:[Swapping]: In.",(long)clock());
      #endif
    }
  }
}
 
void DuckHandle() {
  if(DuckF==FLAG_REQUEST) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Duck]: Request.",(long)clock());
    #endif
    DuckF=FLAG_PROGRESS1;
    DuckM=0;
  }
  if(DuckF==FLAG_PROGRESS1) {
    if(GetCharInfo()->pSpawn->Mount) {
      if(!DuckM) {
        #if DEBUGGING
          WriteChatf("[%d] MQ2Cast:[Duck]: Dismount.",(long)clock());
        #endif
        DuckM=(long)clock();
        Execute("/dismount");
      }
    } else DuckF=FLAG_PROGRESS2;
 }
 if(DuckF==FLAG_PROGRESS2) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Duck]: StopCast.",(long)clock());
    #endif
        Execute("/stopcast");
    CastingE=CAST_ABORTED;
    DuckF=FLAG_COMPLETE;
  }
}
 
void CastHandle() {
 
  // we got the casting request cookies, request immobilize/memorize if needed.
  if(CastF==FLAG_REQUEST) {
    #if DEBUGGING
      WriteChatf("[%d] MQ2Cast:[Casting]: Request.",(long)clock());
    #endif
    CastF=FLAG_PROGRESS1;
    if(StopF==FLAG_COMPLETE) StopF=DONE_SUCCESS;
    if(StopF==FLAG_COMPLETE && CastT>100 && !BardClass()) StopF=FLAG_REQUEST;
    if(MemoF!=FLAG_COMPLETE) MemoHandle();
    else if(StopF!=FLAG_COMPLETE) StopHandle();
  }
 
  // waiting on the casting results to take actions.
  if(CastF==FLAG_PROGRESS3 && CastingE!=DONE_PROGRESS) {
    CastF=FLAG_PROGRESS4;
    if(CastR) CastR--;
    if(CastR) {
      if((CastingE==CAST_SUCCESS && CastW!=RECAST_LAND) || 
         (CastingE==CAST_COLLAPSE) || 
         (CastingE==CAST_FIZZLE) || 
         (CastingE==CAST_INTERRUPTED) || 
         (CastingE==CAST_RECOVER) || 
         (CastingE==CAST_RESIST)) {
        #if DEBUGGING 
          WriteChatf("[%d] MQ2Cast:[Casting]: AutoRecast [%d].",(long)clock(),CastingE);
        #endif 
        if(CastW!=RECAST_ZERO && !TargC) TargC=(pTarget)?((PSPAWNINFO)pTarget)->SpawnID:0;
        CastM=(long)clock()+DELAY_CAST;
        CastF=FLAG_REQUEST;
      }
    }
  }
 
  // casting is over, grab latest casting results and exit.
  if(CastF==FLAG_PROGRESS4) {
    if(CastE>CastingE) CastingE=CastE;
      if(!Twisting) {
        #if DEBUGGING
          WriteChatf("[%d] MQ2Cast:: Casting Complete ID[%d] Result=[%d]",(long)clock(),CastingL,CastingX);
        #endif
                    CastTimer(CastingO,CastingC,((CastingE<CAST_SUCCESS)?CAST_SUCCESS:CastingE)); // patches for ae but sound illogicials
      }

    CastF=FLAG_COMPLETE;
  } 
 
  // evaluate if we are taking too long, or immobilize/memorize event failed.
  if(CastF!=FLAG_COMPLETE) {
    if(StopE==DONE_ABORTED || MemoE==DONE_ABORTED || (long)clock()>CastM) {
      WriteChatf("[%d] MQ2Cast:[Casting]: Aborting!",(long)clock()); 
      CastF=FLAG_PROGRESS4; 
      CastE=CAST_NOTREADY; 
    } 
  }
 
  // waiting for opportunity to start casting, end if conditions not favorables.
  if(CastF==FLAG_PROGRESS1) {
    if(pCastingWnd && (PCSIDLWND)pCastingWnd->Show) return; // casting going on
    CastingC=CastS->ID;

    CastF=FLAG_PROGRESS4;
    if(TargC && (!pTarget || (pTarget && ((PSPAWNINFO)pTarget)->SpawnID!=TargC))) {
      if(CastW==RECAST_DEAD)      CastE=CAST_NOTARGET;
      else if(CastW==RECAST_LAND) CastE=CAST_ABORTED;
    } else if(Invisible && GetCharInfo()->pSpawn->HideMode) { CastE=CAST_INVISIBLE;
    } else if(GetCharInfo()->Stunned)                       { CastE=CAST_STUNNED;
    } else if(StopF!=FLAG_COMPLETE || MemoF!=FLAG_COMPLETE) { CastF=FLAG_PROGRESS1;
    } else {
      long TimeReady=SpellTimer(CastK,CastI);    // get estimate time before it's ready.
      if(TimeReady>3000)  CastE=CAST_NOTREADY;   // if estimate higher then 3 seconds aborts.
      else if(!TimeReady) CastF=FLAG_PROGRESS2;  // estimate says it's ready so cast it
      else CastF=FLAG_PROGRESS1;                 // otherwise give it some time to be ready.
    }
  }
 
  // we got the final approbation to cast, so lets do it.
  if(CastF==FLAG_PROGRESS2) {
    #if DEBUGGING 
      WriteChatf("[%d] MQ2Cast:[Casting]: Cast.",(long)clock());
    #endif
    Success(CastS);
        ItemHandle(true);
    CastF=FLAG_PROGRESS3;
    CastE=DONE_PROGRESS;
    CastingT=(long)clock()+CastT+250+(pConnection->Last)*4;
    CastingE=DONE_PROGRESS;
    CastingC=CastS->ID;
    if((long)GetCharInfo()->pSpawn->CastingData.SpellID>0) {
      CastingX=(CastingE<CAST_SUCCESS)?CAST_SUCCESS:CastingE;
      CastingL=CastingC;
      if(CastK==TYPE_SPELL)     Execute("/multiline ; /stopsong ; /cast \"%s\"",CastN);
          else if(CastK==TYPE_ITEM) {
                  if (!stricmp(CastN,"Fellowship Registration Insignia")) Execute("/multiline ; /stopsong ; /nomodkey /itemnotify ${FindItem[=%s].InvSlot} rightmouseup",CastN);
                  else Execute("/multiline ; /stopsong ; /nomodkey /cast item \"%s\"",CastN);
          }
      else if(CastK==TYPE_ALT)  Execute("/multiline ; /stopsong ; /alt activate %d",((PALTABILITY)CastI)->ID);
    } else {
      if(CastK==TYPE_SPELL)     Cast("\"%s\"",CastN);
          else if(CastK==TYPE_ITEM) {
                  if (!stricmp(CastN,"Fellowship Registration Insignia")) Execute("/nomodkey /itemnotify ${FindItem[=%s].InvSlot} rightmouseup",CastN);
                  else Execute("/nomodkey /cast item \"%s\"",CastN);
          }
      else if(CastK==TYPE_ALT)  Execute("/alt activate %d",((PALTABILITY)CastI)->ID);
    }
  }
} 
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
PLUGIN_API VOID CastCommand(PSPAWNINFO pChar, PCHAR Cmd) {
  Resultat=CAST_DISTRACTED;
  if(!gbInZone || Flags() || Paused() || 
     (pSpellBookWnd && (PCSIDLWND)pSpellBookWnd->Show)
    ) return;
  Reset();
  char zParm[MAX_STRING]; 
  long iParm=0;
  do {
    GetArg(zParm,Cmd,++iParm);
    if(zParm[0]==0) { break;
    } else if(!strnicmp(zParm,"-targetid|",10))  { TargI=atoi(&zParm[10]);
    } else if(!strnicmp(zParm,"-kill",5))        { CastW=RECAST_DEAD; CastR=9999;
    } else if(!strnicmp(zParm,"-maxtries|",10))  { CastW=RECAST_LAND; CastR=atoi(&zParm[10]);
    } else if(!strnicmp(zParm,"-recast|",8))     { CastW=RECAST_ZERO; CastR=atoi(&zParm[8]);
    } else if(!strnicmp(zParm,"-setin|",6))      { GetArg(CastB,zParm,2,FALSE,FALSE,FALSE,'|');
    } else if(!strnicmp(zParm,"-bandolier|",11)) { GetArg(CastB,zParm,2,FALSE,FALSE,FALSE,'|');
    } else if(!strnicmp(zParm,"-invis",6))       { Invisible=true;
    } else if(zParm[0]!='-' && CastN[0]==0) {
      GetArg(CastN,zParm,1,FALSE,FALSE,FALSE,'|');
      GetArg(CastC,zParm,2,FALSE,FALSE,FALSE,'|');
    } else if(zParm[0]!='-' && CastC[0]==0) {
      GetArg(CastC,zParm,1,FALSE,FALSE,FALSE,'|');
    }
  } while(true);
  Resultat=CAST_SUCCESS;
  if(GetCharInfo()->Stunned)                            Resultat=CAST_STUNNED;
  else if(Invisible && GetCharInfo()->pSpawn->HideMode) Resultat=CAST_INVISIBLE;
  else if(!SpellFind(CastN,CastC))                      Resultat=CAST_UNKNOWN;
  else if(fTYPE!=TYPE_SPELL && SpellTimer(fTYPE,fINFO)) Resultat=CAST_NOTREADY;
  else if(TargI) {
    if(PSPAWNINFO Target=(PSPAWNINFO)GetSpawnByID(TargI)) *(PSPAWNINFO*)ppTarget=Target;
    else Resultat=CAST_NOTARGET;
  }
  if(Resultat==CAST_SUCCESS && fTYPE==TYPE_SPELL) {
    if(!BardClass()) {
      CastG=GEMID(fFIND->ID);
      if(CastG==NOID) {
        CastG=atoi(&CastC[(strnicmp(CastC,"gem",3))?0:3])-1;
        MemoLoad(CastG,fFIND);
        SpellTotal=1;
        MemoF=FLAG_REQUEST;
        MemoE=DONE_SUCCESS;
      }
    } else Resultat=CAST_DISTRACTED;
  }
  if(Resultat!=CAST_SUCCESS) {
    #if DEBUGGING 
      WriteChatf("[%d] MQ2Cast:[Casting]: Complete. [%d]",(long)clock(),Resultat);
    #endif
    return;
  }
  CastF=FLAG_REQUEST;
  CastI=fINFO;
  CastK=fTYPE;
  CastT=fTIME;
  CastS=fFIND;
  CastM=(long)clock()+DELAY_CAST;
  strcpy(CastN,fNAME);
  #if DEBUGGING
    WriteChatf("[%d] MQ2Cast:[Casting]: Name<%s> Type<%d>.",(long)clock(),CastN,CastK);
  #endif
  CastHandle();
}
 
PLUGIN_API VOID DuckCommand(PSPAWNINFO pChar, PCHAR Cmd) {
  if(gbInZone) {
    if(CastF!=FLAG_COMPLETE) CastR=0;
    if((pCastingWnd && (PCSIDLWND)pCastingWnd->Show) && CastingLeft()>500) {
      DuckF=FLAG_REQUEST;
      DuckHandle();
    }
  }
  Resultat=CAST_SUCCESS;
} 
 
PLUGIN_API VOID MemoCommand(PSPAWNINFO pChar, PCHAR zLine) {
  Resultat=CAST_DISTRACTED;
  if(!gbInZone || Flags() || Paused() || !pSpellBookWnd) return;
  if(GetCharInfo()->Stunned) {
    Resultat=CAST_STUNNED;
    return;
  }
  Reset();
  long iParm=0;
  char zParm[MAX_STRING];
  char zTemp[MAX_STRING];
  Resultat=CAST_SUCCESS;
  do {
    GetArg(zParm,zLine,++iParm);
    if(!zParm[0]) break;
    GetArg(zTemp,zParm,1,FALSE,FALSE,FALSE,'|');
    if(PSPELL Search=SpellBook(zTemp)) {
      GetArg(zTemp,zParm,2,FALSE,FALSE,FALSE,'|');
      long Gem=atoi(&zTemp[(strnicmp(zTemp,"gem",3))?0:3])-1;
      if(!((DWORD)Gem<NUM_SPELL_GEMS)) {
        GetArg(zTemp,zLine,1+iParm);
        Gem=atoi(&zTemp[(strnicmp(zTemp,"gem",3))?0:3])-1;
        if((DWORD)Gem<NUM_SPELL_GEMS) iParm++;
      }
      MemoLoad(Gem,Search);
    }
  } while(true);
  for(int sp=0;sp<NUM_SPELL_GEMS;sp++)
    if(SpellToMemorize.SpellId[sp] != 0xFFFFFFFF &&
       SpellToMemorize.SpellId[sp] !=
       GetCharInfo2()->MemorizedSpells[sp]) SpellTotal++;
  if(SpellTotal) {
    MemoF=FLAG_REQUEST;
    MemoE=DONE_SUCCESS;
    MemoHandle();
  }
}
 
PLUGIN_API VOID SpellSetDelete(PSPAWNINFO pChar,PCHAR Cmd) {
  Resultat=CAST_ABORTED;
  if(!gbInZone) return;
  else if(!Cmd[0]) MacroError("Usage: /ssd setname");
  else {
    Resultat=CAST_SUCCESS;
    sprintf(INIFileName,"%s\\%s_%s.ini",gszINIPath,EQADDR_SERVERNAME,GetCharInfo()->Name);
    WritePrivateProfileString("MQ2Cast(SpellSet)",Cmd,NULL,INIFileName);
  }
}
 
PLUGIN_API VOID SpellSetList(PSPAWNINFO pChar, PCHAR Cmd) {
  Resultat=CAST_SUCCESS;
  if(!gbInZone) return;
  char Keys[MAX_STRING*10]={0}; 
  char Temp[MAX_STRING];
  PCHAR pKeys=Keys; 
  long Disp=0;
        Resultat=CAST_SUCCESS;
        sprintf(INIFileName,"%s\\%s_%s.ini",gszINIPath,EQADDR_SERVERNAME,GetCharInfo()->Name);
        WriteChatf("MQ2Cast:: SpellSet [\ay Listing... \ax].",Disp);
  GetPrivateProfileString("MQ2Cast(SpellSet)",NULL,"",Keys,MAX_STRING*10,INIFileName);
  while(pKeys[0]) {
        GetPrivateProfileString("MQ2Cast(SpellSet)",pKeys,"",Temp,MAX_STRING,INIFileName);
    if(Temp[0]) {
      if(!Disp) WriteChatf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
      WriteChatf("\ay%s\ax",pKeys);
      Disp++;
    }
    pKeys+=strlen(pKeys)+1;
  }
  if(Disp) WriteChatf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
  WriteChatf("MQ2Cast:: SpellSet [\ay %d Displayed\ax ].",Disp);
}
 
PLUGIN_API VOID SpellSetMemorize(PSPAWNINFO pChar, PCHAR Cmd) {
  Resultat=CAST_UNKNOWN;
  if(!gbInZone)   return;
  else if(!Cmd[0]) MacroError("Usage: /ssm setname");
  else {
    char List[MAX_STRING];
    Resultat=CAST_SUCCESS;
    sprintf(INIFileName,"%s\\%s_%s.ini",gszINIPath,EQADDR_SERVERNAME,GetCharInfo()->Name);
                GetPrivateProfileString("MQ2Cast(SpellSet)",Cmd,"",List,MAX_STRING,INIFileName);
    if(List[0]) MemoCommand(GetCharInfo()->pSpawn,List);
  }
}
 
PLUGIN_API VOID SpellSetSave(PSPAWNINFO pChar, PCHAR Cmd) {
  if(!gbInZone) return;
  char zSet[MAX_STRING]; GetArg(zSet,Cmd,1);
  char zGem[MAX_STRING]; GetArg(zGem,Cmd,2);
  Resultat=CAST_ABORTED;
  if(!zSet[0]) {
    MacroError("Usage: /sss setname <gemlist>");
    return;
  }
  if(!zGem[0]) sprintf(zGem,"123456789");
  char zLst[MAX_STRING]={0};
  char zTmp[MAX_STRING];
  long find=0;
  for(int g=0;g<NUM_SPELL_GEMS;g++)
    if((long)GetCharInfo2()->MemorizedSpells[g]>0)
      if(strstr(zGem,ListGems[g])) {
        sprintf(zTmp,"%d|%d",GetCharInfo2()->MemorizedSpells[g],g+1);
        if(find) strcat(zLst," ");
        strcat(zLst,zTmp);
        find++;
      }
  Resultat=CAST_UNKNOWN;
  if(find) {
    Resultat=CAST_SUCCESS;
    sprintf(INIFileName,"%s\\%s_%s.ini",gszINIPath,EQADDR_SERVERNAME,GetCharInfo()->Name);
                WritePrivateProfileString("MQ2Cast(SpellSet)",Cmd,zLst,INIFileName);
  } 
}
 
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=//
 
PLUGIN_API VOID InitializePlugin(VOID) {
  aCastEvent(LIST289, CAST_COLLAPSE    ,"Your gate is too unstable, and collapses#*#");
  aCastEvent(LIST289, CAST_CANNOTSEE   ,"You cannot see your target#*#");
  aCastEvent(LIST289, CAST_COMPONENTS  ,"You are missing some required components#*#");
  aCastEvent(LIST289, CAST_COMPONENTS  ,"You need to play a#*#instrument for this song#*#");
  aCastEvent(LIST289, CAST_DISTRACTED  ,"You are too distracted to cast a spell now#*#");
  aCastEvent(LIST289, CAST_DISTRACTED  ,"You can't cast spells while invulnerable#*#");
  aCastEvent(LIST289, CAST_DISTRACTED  ,"You *CANNOT* cast spells, you have been silenced#*#");
  aCastEvent(LIST289, CAST_IMMUNE      ,"Your target has no mana to affect#*#");
  aCastEvent(LIST013, CAST_IMMUNE      ,"Your target is immune to changes in its attack speed#*#");
  aCastEvent(LIST013, CAST_IMMUNE      ,"Your target is immune to changes in its run speed#*#");
  aCastEvent(LIST289, CAST_IMMUNE      ,"Your target cannot be mesmerized#*#");
  aCastEvent(UNKNOWN, CAST_IMMUNE      ,"Your target looks unaffected#*#");
  aCastEvent(LIST264, CAST_INTERRUPTED ,"Your spell is interrupted#*#");
  aCastEvent(UNKNOWN, CAST_INTERRUPTED ,"Your casting has been interrupted#*#");
  aCastEvent(LIST289, CAST_FIZZLE      ,"Your spell fizzles#*#");
  aCastEvent(LIST289, CAST_FIZZLE      ,"You miss a note, bringing your song to a close#*#");
  aCastEvent(LIST289, CAST_NOTARGET    ,"You must first select a target for this spell#*#");
  aCastEvent(LIST289, CAST_NOTARGET    ,"This spell only works on#*#");
  aCastEvent(LIST289, CAST_NOTARGET    ,"You must first target a group member#*#");
  aCastEvent(LIST289, CAST_NOTREADY    ,"Spell recast time not yet met#*#");
  aCastEvent(LIST289, CAST_OUTOFMANA   ,"Insufficient Mana to cast this spell#*#");
  aCastEvent(LIST289, CAST_OUTOFRANGE  ,"Your target is out of range, get closer#*#");
  aCastEvent(LIST289, CAST_OUTDOORS    ,"This spell does not work here#*#");
  aCastEvent(LIST289, CAST_OUTDOORS    ,"You can only cast this spell in the outdoors#*#");
  aCastEvent(LIST289, CAST_OUTDOORS    ,"You can not summon a mount here#*#");
  aCastEvent(LIST289, CAST_OUTDOORS    ,"You must have both the Horse Models and your current Luclin Character Model enabled to summon a mount#*#");
  aCastEvent(LIST264, CAST_RECOVER     ,"You haven't recovered yet#*#");
  aCastEvent(LIST289, CAST_RECOVER     ,"Spell recovery time not yet met#*#");
  aCastEvent(LIST289, CAST_RESIST      ,"Your target resisted the#*#spell#*#");
  aCastEvent(LIST289, CAST_STANDING    ,"You must be standing to cast a spell#*#");
  aCastEvent(LIST289, CAST_STUNNED     ,"You can't cast spells while stunned#*#");
  aCastEvent(LIST289, CAST_SUCCESS     ,"You are already on a mount#*#");
  aCastEvent(LIST289, CAST_TAKEHOLD    ,"Your spell did not take hold#*#");
  aCastEvent(LIST289, CAST_TAKEHOLD    ,"Your spell would not have taken hold#*#");
  aCastEvent(LIST289, CAST_TAKEHOLD    ,"Your spell is too powerfull for your intended target#*#");
  aCastEvent(LIST289, CAST_TAKEHOLD    ,"You need to be in a more open area to summon a mount#*#");
  aCastEvent(LIST289, CAST_TAKEHOLD    ,"You can only summon a mount on dry land#*#");
  aCastEvent(LIST289, CAST_TAKEHOLD    ,"This pet may not be made invisible#*#");
  pCastType= new MQ2CastType;
  AddMQ2Data("Cast",dataCast);
  AddCommand("/casting"  ,CastCommand);
  AddCommand("/interrupt",DuckCommand);
  AddCommand("/memorize" ,MemoCommand);
  AddCommand("/ssd",SpellSetDelete);
  AddCommand("/ssl",SpellSetList);
  AddCommand("/ssm",SpellSetMemorize);
  AddCommand("/sss",SpellSetSave);
}
 
PLUGIN_API VOID ShutdownPlugin(VOID) { 
  RemoveMQ2Data("Cast");
  delete pCastType;
  RemoveCommand("/casting");
  RemoveCommand("/interrupt");
  RemoveCommand("/memorize");
  RemoveCommand("/ssd");
  RemoveCommand("/ssl");
  RemoveCommand("/ssm");
  RemoveCommand("/sss");
}
 
PLUGIN_API VOID OnEndZone(VOID) {
  Reset();
  CastingO=NOID;
  CastingC=NOID;
  CastingE=CAST_SUCCESS;
  CastingT=0;
  ImmobileT=0;
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color) {
  if(gbInZone) {
    if(CastingC!=NOID && !Twisting) {
      Parsed=false;
      if(Color==264)      { LIST264.Feed(Line); SUCCESS.Feed(Line); }
      else if(Color==289) { LIST289.Feed(Line);                     }
      else if(Color==13)  { LIST013.Feed(Line);                     }
      if(!Parsed) {
        UNKNOWN.Feed(Line);
        if(Parsed) WriteChatf("\arMQ2Cast::Note for Author[\ay%s\ar]=(\ag%d\ar)\ax",Line,Color);
      }
    }
  }
  return 0;
}
 
PLUGIN_API VOID OnPulse(VOID) {
  if(gbInZone && (long)clock()>CastingP && GetCharInfo() && GetCharInfo()->pSpawn) { 
        CastingP=(long)clock()+DELAY_PULSE;
        
    // evaluate immobile flag and handle immobilize request
    Immobile=Moving();
    if(StopF!=FLAG_COMPLETE) StopHandle();
    CastingD=GetCharInfo()->pSpawn->CastingData.SpellID ;
 
        // casting window currently openened?
    if(pCastingWnd && (PCSIDLWND)pCastingWnd->Show) {
      Casting=true;
      if(CastingO==NOID) CastingO=(pTarget)?((long)((PSPAWNINFO)pTarget)->SpawnID):0;
 
      // was this an unecpected cast?
      if(CastingD!=CastingC && CastingD!=NOID) {
        CastingE=DONE_PROGRESS;
        CastingC=CastingD;
        CastingT=GetCharInfo()->pSpawn->CastingData.SpellETA  - 
                 GetCharInfo()->pSpawn->TimeStamp +
                 clock()+450+(pConnection->Last)*4;
        Success(GetSpellByID(CastingD));
      }
 
      // are we attempting to interrupt this?
      if(DuckF!=FLAG_COMPLETE) DuckHandle();
      return;
    }
 
    // wait for incoming chat, timers, and windows to be closed.
    DuckF=FLAG_COMPLETE;
    Twisting=Evaluate("${If[${Twist.Twisting},1,0]}")?true:false;
    if(Casting) {
      if(CastingC==CastingD)
        if(PSPELL Spell=GetSpellByID(CastingC)) 
          switch(Spell->TargetType) {
            case 18: // Uber Dragons
            case 17: // Uber Giants
            case 16: // Plant
            case 15: // Corpse
            case 14: // Pet
            case 11: // Summoned
            case 10: // Undead
            case  9: // Animal
            case  5: // Single
              if(!pTarget) CastingE=CAST_NOTARGET;
              break;
          }
 
      // re-evaluate casting timer after cast window close
      CastingT=clock()+450+(pConnection->Last)*2;
      Casting=false;
    }
    if(CastingE==DONE_PROGRESS) {
      if((long)clock()>CastingT) CastingE=CAST_SUCCESS;
      else if(!Twisting) return;
    }
    if(Paused()) {
      if((long)GetCharInfo()->pSpawn->CastingData.SpellID>0) Execute("/stopsong");
      return;
    }
 
    // give time to proceed other casting events
    if(MemoF!=FLAG_COMPLETE) MemoHandle();
    if(MemoF!=FLAG_COMPLETE) return;
    if(CastF!=FLAG_COMPLETE) CastHandle();

    // make sure we get final casting results
    if((CastF==FLAG_COMPLETE && CastingC!=NOID && CastingD==NOID) ||
       (BardClass() && CastingC!=NOID && (CastingD!=NOID))) {
      CastingX=(CastingE<CAST_SUCCESS)?CAST_SUCCESS:CastingE;
      CastingL=CastingC;
      CastingE=DONE_COMPLETE;
          if(!Twisting) {
        #if DEBUGGING
          WriteChatf("[%d] MQ2Cast:: Casting Complete ID[%d] Result=[%d]",(long)clock(),CastingL,CastingX);
        #endif
                    CastTimer(CastingO,CastingC,CastingX); // patches for ae but sound illogicials
      }
      CastingC=NOID;
      CastingO=NOID;
    }
 
    // make sure we finish other casting events
    if(CastF==FLAG_COMPLETE) {
      if(ItemF!=FLAG_COMPLETE) ItemHandle(false);
      StopEnding();
    }
  }
}
