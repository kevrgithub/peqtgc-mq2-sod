#include "../MQ2Plugin.h"
PreSetup("MQ2Exchange");
PLUGIN_VERSION(1.19);
PCONTENTS ifITEM;
PCONTENTS ifPACK;

bool      ItemFind(PCHAR ID, long B=0, long E=NUM_INV_SLOTS);
char      szArg1[MAX_STRING];
char      szArg2[MAX_STRING];
long      ifSLOT;
long      sfSLOT;
long      pfSLOT;

void SendModClick(PCHAR ScreenID, PCHAR ClickNotification, DWORD KeyState) {
  if(pInventoryWnd) if(CXWnd *Win=pInventoryWnd->GetChildItem(ScreenID)) {
    BOOL KeyboardFlags[4];
    *(DWORD*)&KeyboardFlags=*(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags;
    *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags=KeyState;
    SendWndClick2(Win,ClickNotification);
    *(DWORD*)&((PCXWNDMGR)pWndMgr)->KeyboardFlags=*(DWORD*)&KeyboardFlags;
  }
}

void Help() {
   WriteChatColor("MQ2 Exchange Item Commands: ");
   WriteChatColor("/exchange <itemname|ID> <slotname|slotnumber> - Exchanges Item to Slot");
   WriteChatColor("/exchange list - Displays the list of slot names and slot numbers");
   WriteChatColor("/exchange help - Displays all commands");
   WriteChatColor("/unequip <slotname|slotnumber> - Unequips Item from Slot");
}

void List() {
   WriteChatColor("MQ2 Exchange Item Slots:");
   for(int i = 0; szItemSlot[i]; i++)
      WriteChatf ("%s | %d",szItemSlot[i],i);
}

bool ItemFind(PCHAR ID, long B, long E) {
   long Search=IsNumber(ID);
   long Number=atoi(ID);
   ifITEM=0;
   ifSLOT=0;
   ifPACK=0;
   for(int iSlot=E-1;iSlot>=B;iSlot--)
      if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
         if((Search && Number==cSlot->Item->ItemNumber) || (!Search &&
!stricmp(ID,cSlot->Item->Name))) {
            ifITEM=cSlot;
            ifSLOT=iSlot;
            if(iSlot >= BAG_SLOT_START)
               ifPACK=cSlot;
            return true;
         } else
            if(cSlot->Item->Type==ITEMTYPE_PACK) {
               for(int iPack=0;iPack<cSlot->Item->Slots;iPack++)
                  if(PCONTENTS cPack=cSlot->Contents[iPack]) {
                     if((Search && Number==cPack->Item->ItemNumber) ||
(!Search && !stricmp(ID,cPack->Item->Name))) {
                        ifITEM=cPack;
                        ifSLOT=(iSlot-BAG_SLOT_START)*10+262+iPack;
                        ifPACK=cSlot;
                        return true;
                     }
                  }
            }
      }
      return false;
}

bool SlotFind(PCHAR ID) {
   if(IsNumber(ID)) {
      sfSLOT=atoi(ID);
      if(sfSLOT>=0 && sfSLOT<NUM_INV_SLOTS)
         return true;
   }
   else
      for(sfSLOT=0; szItemSlot[sfSLOT]; sfSLOT++) {
         if(!_stricmp(ID,szItemSlot[sfSLOT]))
            return true;
      }
   sfSLOT=-1;
   return false;
}

long PackFind(PITEMINFO Item) {
   pfSLOT=0;
   long pfSIZE=10;
   for(int iSlot=BAG_SLOT_START; iSlot<NUM_INV_SLOTS; iSlot++) {
      if(PCONTENTS cSlot=GetCharInfo2()->InventoryArray[iSlot]) {
         if(cSlot->Item->Type==ITEMTYPE_PACK &&
cSlot->Item->Combine!=2 && Item->Size <= cSlot->Item->SizeCapacity &&
(!pfSLOT || cSlot->Item->SizeCapacity<pfSIZE)) {
            for(int iPack=0;iPack<cSlot->Item->Slots;iPack++) {
               if(!cSlot->Contents[iPack]) {
                     pfSLOT=(iSlot-BAG_SLOT_START)*10+262+iPack;
                  pfSIZE=cSlot->Item->SizeCapacity;
                  break;
               }
            }
         }
      }
      else
         if(!pfSLOT) {
            pfSLOT=iSlot;
         }
   }
   return pfSLOT;
}

bool CheckValidExchange(PITEMINFO swapInItem, PCONTENTS bagSlot, int toSlot) {
   if( (GetCharInfo2()->Class!=8) &&
(GetCharInfo()->pSpawn->CastingData.SpellETA) ) {
      MacroError("Exchange: Cannot /exchange while casting");
   }
   if(toSlot>=BAG_SLOT_START && toSlot<NUM_INV_SLOTS) return true;
   if(bagSlot->Item->Type==ITEMTYPE_PACK) {
      if(GetCharInfo2()->InventoryArray[toSlot]) {
         if(GetCharInfo2()->InventoryArray[toSlot]->Item->Size > bagSlot->Item->SizeCapacity) {
            MacroError("Exchange: %s is too large to fit in %s",GetCharInfo2()->InventoryArray[toSlot]->Item->Name,bagSlot->Item->Name);
            return false;
         }
      }
   }
   if(toSlot==22) {
      if((swapInItem->EquipSlots&(1<<11)) || (swapInItem->EquipSlots&(1<<22)))
         return true;
      else {
         MacroError("Exchange: Cannot equip %s in the ammo slot.",swapInItem->Name);
         return false;
      }
   }
   if(toSlot==0xd && GetCharInfo2()->InventoryArray[0xe] &&((swapInItem->ItemType==0x1) || (swapInItem->ItemType==0x4))) {
      WriteChatf("Exchange: Cannot equip %s when %s is in the offhand slot",swapInItem->Name,GetCharInfo2()->InventoryArray[0xe]->Item->Name);
      return false;
   }
   if(!(swapInItem->Classes&(1<<((GetCharInfo2()->Class)-1)))) {
      MacroError("Exchange: Cannot equip %s. Class restriction.",swapInItem->Name);
      return false;
   }
   DWORD myRace=GetCharInfo2()->Race;
   switch(myRace)
   {
      case 0x80:
         myRace=0xc;
         break;
      case 0x82:
         myRace=0xd;
         break;
      case 0x14a:
         myRace=0xe;
         break;
      case 0x20a:
         myRace=0xf;
         break;
      default:
         myRace--;
   }
   if(!(swapInItem->Races&(1<<myRace))) {
      MacroError("Exchange: Cannot equip %s. Race restriction.",swapInItem->Name);
      return false;
   }
   DWORD Deity=GetCharInfo2()->Deity-200;
   if((swapInItem->Diety!=0) && !(swapInItem->Diety&(1<<Deity))) {
      MacroError("Exchange: Cannot equip %s. Deity restriction.",swapInItem->Name);
      return false;
   }
   return true;
}

VOID ExchangeItem(PSPAWNINFO pChar, PCHAR szLine) {
   if(!GetCharInfo2())
      return;
   GetArg(szArg1,szLine,1);
   GetArg(szArg2,szLine,2);
   if(!stricmp(szArg1,"list") || !stricmp(szArg2,"list"))
      List();
   else if(!_stricmp(szArg1,"help"))
      Help();
   else if(!szArg1[0] || !szArg2[0])
      MacroError("Usage: /exchange <itemname|itemID> <slotname|slotnumber>");
   else if(GetCharInfo2()->Cursor)
      MacroError("Exchange: Your mouse pointer must be clear to move an item.");
   else if(!SlotFind(szArg2))
      MacroError("Exchange: %s slot not found",szArg2);
   else if(!ItemFind(szArg1))
      MacroError("Exchange: Couldn't find %s in your inventory",szArg1);
   else if(ifSLOT<BAG_SLOT_START || ifSLOT==sfSLOT ||
!CheckValidExchange(ifITEM->Item,ifPACK,sfSLOT))
      return;
   else {
      sprintf(szArg2,"InvSlot%d",sfSLOT);
      pInvSlotMgr->MoveItem(ifSLOT,0x1F,1,1);
      SendModClick(szArg2,"leftmouseup",0);
      pInvSlotMgr->MoveItem(0x1F,ifSLOT,1,1);
   }
}

VOID UnequipItem(PSPAWNINFO pChar, PCHAR szLine) {
   if(GetCharInfo()->pSpawn->CastingData.SpellETA) {
      MacroError("Unequip: Cannot /unequip while casting");
      return;
   }
   if(!GetCharInfo2())
      return;
   else if(!_stricmp(szLine,"list"))
      List();
   else if(!_stricmp(szLine,"help"))
      Help();
   else if(!szLine[0])
      MacroError("Usage: /unequip <slotname|slotnumber>");
   else if(GetCharInfo2()->Cursor)
      MacroError("Unequip: Your mouse pointer must be clear to move an item.");
   else if(!SlotFind(szLine))
      MacroError("Unequip: %s slot not found.",szLine);
   else {
        if(sfSLOT>=BAG_SLOT_START)
         return;
      PCONTENTS uiCONT=GetCharInfo2()->InventoryArray[sfSLOT];
      if(!uiCONT)
         MacroError("Unequip: There is nothing in the %s slot to unequip",szLine);
      else if(!PackFind(uiCONT->Item))
         MacroError("Unequip: %s is too large for bags, or no room.",uiCONT->Item->Name);
      else {
         sprintf(szArg2,"InvSlot%d",sfSLOT);
         SendModClick(szArg2,"leftmouseup",1);
         pInvSlotMgr->MoveItem(0x1F,pfSLOT,1,1);
      }
   }
}

PLUGIN_API VOID doExchange(PCHAR szLine) {
   ExchangeItem(GetCharInfo()->pSpawn,szLine);
}

PLUGIN_API VOID doUnequip(PCHAR szLine) {
   UnequipItem(GetCharInfo()->pSpawn,szLine);
}

PLUGIN_API VOID InitializePlugin(VOID) {
   AddCommand("/exchange",ExchangeItem);
   AddCommand("/unequip",UnequipItem);
}

PLUGIN_API VOID ShutdownPlugin(VOID) {
   RemoveCommand("/exchange");
   RemoveCommand("/unequip");
}
