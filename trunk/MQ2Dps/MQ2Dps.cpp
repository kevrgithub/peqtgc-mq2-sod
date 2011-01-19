// MQ2Dps.cpp : Defines the entry point for the DLL application.

// Author: Shire
// Version: 0.9.96 alpha
// Updated 03-10-05 by koad for some changes to compile by Cr4zyB4rd
// cleaned up some things, removed param handlers, and included MunkyBoy's focus fix

// Know bug: Sort functions
// #define MAX_CHARS 50, change 50 to the number of people fighting.

#include "../MQ2Plugin.h"
#include "string.h"
#include "MQ2Dps.h"

PreSetup("MQ2Dps");

/////////////////////////////////
class MQ2DpsType : public MQ2Type
{
   private:
      int _id;
   public:
      enum DpsMembers
      {
         Version=1,
         Name=2,
         Dps=3,
         MyPet=4,
         Me=5,
      };
      /*CDpsType(int id)
      {
         _id = id;
      }*/
      MQ2DpsType() : MQ2Type("dps")
      {
         //_id = -1;
         TypeMember(Version);
         TypeMember(Name);
         TypeMember(Dps);
         TypeMember(MyPet);
         TypeMember(Me);
         _id = -1;
      }
      ~MQ2DpsType()
      {
      }
      void SetIndex(int id)
      {
         _id = id;
      }
      bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
      {
         PMQ2TYPEMEMBER pMember=MQ2DpsType::FindMember(Member);
         if (!pMember)
            return false;
         switch((DpsMembers)pMember->ID)
         {
         case Version:
            Dest.Ptr= ver;
            Dest.Type=pStringType;
            return true;
         case Name:
            //WriteChatColor(Index);
            if (_id >= 0)
            {
               //WriteChatColor("123");
               //unsigned long N=atoi(Index);
               if (strcmp(_name[_id], "\0") != 0)
               {
                  char tmp_str[MAX_STRING] = {0};
                  strcpy(tmp_str,_name[_id]);
                  Dest.Ptr= &tmp_str[0];
                  Dest.Type=pStringType;
               }
               else
               {
                  Dest.Ptr = "-";
                  Dest.Type=pStringType;
               }
               _id = -1;
               return true;
            }
            //WriteChatColor("456");
            return false;
         case Dps:
            if (_id >= 0)
            {
               //unsigned long N=atoi(Index);
               int dps = 0;
               if ( (dps = _paramDps(_id)) != -1)
               {
                  Dest.Int = dps;
                  Dest.Type=pIntType;
               }
               else
               {
                  Dest.Ptr = "-";
                  Dest.Type=pStringType;
               }
               _id = -1;
               return true;
            }
            return false;
         case Me:
            if (_playerid != -1)
            {
               int res = _paramDps(_playerid);
               if (res != -1)
               {
                  Dest.Int = res;
                  Dest.Type=pIntType;
               }
               else
               {
                  Dest.Ptr = "-";
                  Dest.Type=pStringType;
               }
            return true;
            }
            else
            {
               Dest.Ptr = "-";
               Dest.Type=pStringType;
               return true;
            }
         case MyPet:
            if (_petid != -1)
            {
               int res = _paramDps(_petid);
               if (res != -1)
               {
                  Dest.Int = res;
                  Dest.Type=pIntType;
               }
               else
               {
                  Dest.Ptr = "-";
                  Dest.Type=pStringType;
               }
            return true;
            }
            else
            {
               Dest.Ptr = "-";
               Dest.Type=pStringType;
               return true;
            }
         }
         return false;
      }

      bool ToString(MQ2VARPTR VarPtr, PCHAR Destination)
      {
         /*if (BzDone)
            strcpy(Destination,"TRUE");
         else
            strcpy(Destination,"FALSE");*/
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
      /*long dps()
      {
         return _paramDps(_id);
      }
      long meleedmg()
      {
         return _meleedmg[_id];
      }
      long castdmg()
      {
         return _castdmg[_id];
      }*/
};

//MQ2DpsType Dps[MAX_CHARS];
class MQ2DpsType *pDpsType=0;
/////////////////////////////////
class CDpsWnd : public CCustomWnd
{
public:
   CDpsWnd():CCustomWnd("DpsWindow")
   {
      //Add each button here
      ONButton = (CButtonWnd*)GetChildItem("DPSButtonON");               //ON Button
      OFFButton = (CButtonWnd*)GetChildItem("DPSButtonOFF");            //OFF Button
      RESETButton = (CButtonWnd*)GetChildItem("DPSButtonRESET");         //RESET Button
      CloseButton = (CButtonWnd*)GetChildItem("CloseButton");             //Close Button
      SetWndNotification(CDpsWnd);
   }

   ~CDpsWnd()
   {
   }

   int WndNotification(CXWnd *pWnd, unsigned int Message, void *unknown)
   {
       //ON Button
      if (pWnd==(CXWnd*)ONButton)
      {
         if (Message==XWM_LCLICK)
            DoCommand((PSPAWNINFO)pCharSpawn,"/dps on");
         else
            DebugSpew("ONButton message %Xh / %d",Message,Message);
      }
      //OFF Button
      if (pWnd==(CXWnd*)OFFButton)
      {
         if (Message==XWM_LCLICK)
            DoCommand((PSPAWNINFO)pCharSpawn,"/dps off");
         else
            DebugSpew("OFFButton message %Xh / %d",Message,Message);
      }
      //RESET Button
      if (pWnd==(CXWnd*)RESETButton)
      {
         if (Message==XWM_LCLICK)
            DoCommand((PSPAWNINFO)pCharSpawn,"/dps reset");
         else
            DebugSpew("RESETButton message %Xh / %d",Message,Message);
      }
      //Close Button
      if (pWnd==(CXWnd*)CloseButton)
       {
         if (Message==XWM_LCLICK)
            pXWnd()->Show(0,0);
         else
            DebugSpew("CloseButton message %Xh / %d",Message,Message);
      }
      return 0;
   };
   CButtonWnd *ONButton;
   CButtonWnd *OFFButton;
   CButtonWnd *RESETButton;
   CButtonWnd *CloseButton;

};

CDpsWnd *MyWnd = 0;


void CreateDpsWindow()
{
   if (MyWnd)  return;
   if (pSidlMgr->FindScreenPieceTemplate("DpsWindow")) {
      MyWnd = new CDpsWnd();
      ReadWindowINI((PCSIDLWND)MyWnd);
      WriteWindowINI((PCSIDLWND)MyWnd);
   }
}

void DestroyDpsWindow()
{
   if (MyWnd)
   {
      WriteWindowINI((PCSIDLWND)MyWnd);
      delete MyWnd;
      MyWnd=0;
   }
}


// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
   if (debug) DebugSpewAlways("Initializing MQ2Dps");
   WriteChatColor("Loading MQ2Dps v0.9.94 ...");

   AddXMLFile("MQUI_DpsWnd.xml");
   AddMQ2Data("Dps",dataDps);
   AddCommand("/dps",CmdDps);

   pDpsType = new MQ2DpsType;

   char val[MAX_STRING] = {0};
   GetPrivateProfileString("Window","LogonShow","1",val,MAX_STRING,INIFileName);            _logonshowwindow = atoi(val) == 1 ? 1 : 0;   strcpy(&val[0], "\0");
   GetPrivateProfileString("General Options","Sort","0",val,MAX_STRING,INIFileName);         _sort = atoi(val);   strcpy(&val[0], "\0");
   GetPrivateProfileString("General Options","PetFollowOwner","1",val,MAX_STRING,INIFileName);   _petowner = atoi(val)  == 1 ? 1 : 0;
   GetPrivateProfileString("General Options","AbsoluteDps","1",val,MAX_STRING,INIFileName);   _absdps = atoi(val)  == 1 ? 1 : 0;
   _focusdps = 0;
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
   if (debug) DebugSpewAlways("Shutting down MQ2Dps");

   DestroyDpsWindow();
   RemoveXMLFile("MQUI_DpsWnd.xml");

   RemoveMQ2Data("Dps");
   RemoveCommand("/dps");

   delete pDpsType;
}

PLUGIN_API VOID OnCleanUI(VOID)
{
   DestroyDpsWindow();
}

PLUGIN_API VOID OnReloadUI()
{
   if (_logonshowwindow)
   {
      CreateDpsWindow();
   }
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
   if (GameState==GAMESTATE_INGAME && !MyWnd)
   {
       CreateDpsWindow();
   }
   ResetGV();
}

// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
// but after MQ filters and chat events are taken care of.
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
   if (debug) DebugSpewAlways("MQ2Dps::OnIncomingChat(%s)",Line);

   char strl[MAX_STRING] = {'\0'};
   char strd[MAX_STRING] = {'\0'};
   sprintf (strl,"%s",Line);


   if (_ON)
   {
      if (strstr(Line,"has taken") && strstr(Line,"damage from your"))
      {
         if ( (_focusdps && strstr(strlwr(strl),strlwr(strFocusName))) || !(_focusdps) ) {
            if (debug) DebugSpewAlways("Call Event_Dot(%s)",Line);
            Event_Dot(Line);
         }
      }

      if (strstr(Line,"for") && strstr(Line,"points of damage"))
      {
         if ( (_focusdps && strstr(strlwr(strl),strlwr(strFocusName))) || !(_focusdps) ) {
            if (debug) DebugSpewAlways("Call Event_Melee(%s)",Line);
            Event_Melee(Line);
         }
      }

      if (strstr(Line,"hit") && strstr(Line,"non-melee") && (strstr(Line,"You") == NULL))
      {
         if ( (_focusdps && strstr(strlwr(strl),strlwr(strFocusName))) || !(_focusdps) ) {
            if (debug) DebugSpewAlways("Call Event_Cast(%s)",Line);
            Event_Cast(Line);
         }
      }

      if ( (strstr(Line,"dies") || ((strstr(Line,"have") || strstr(Line,"has")) && strstr(Line,"slain"))))
      {
         if (_focusdps) {
            sprintf(strd, "%s has been slain by", strlwr(strFocusName));
            if ( strstr(strlwr(strl), strd) || strstr(strlwr(strl), "you have") ) {
               if (debug) DebugSpewAlways("Call Event_MobDead(%s)",Line);
               Event_MobDead(Line);
               _focusdps = 0;
               memset(strFocusName, '\0', MAX_STRING);
            }
         } else {
            if (debug) DebugSpewAlways("Call Event_MobDead(%s)",Line);
            Event_MobDead(Line);
         }
      }
      /*if ( strstr(Line,"holy blade cleanses") && strstr(Line,"target") )
      {
         if (debug) DebugSpewAlways("Call Event_OtherMeleeDmg(%s)",Line);
         Event_OtherMeleeDmg(Line);
      }*/
   }
   return 0;
}

void Event_Dot (char* line)
{
   char str[MAX_STRING] = "\0";
   char name[MAX_STRING] = "\0";
   int pos = 0;
   int pos1 = 0;
   int pos2 = 0;
   int id = 0;
   PSPAWNINFO pPet = NULL;

   //fight = 1;
   if (debug) DebugSpewAlways("Entering Dot");

   strcpy(name,((PCHARINFO)pCharData)->Name);
   if (IdInList(name) == -1) AddToList(name);

   if ((id = IdInList(name)) != -1 )
   {
      pos1 = (int)(strstr(line,"has taken") - line) + 10;
      pos2 = (int)(strstr(line,"damage") - line) - 1;
      strncpy(str, &line[pos1], pos2 - pos1);
      strcpy(&str[pos2-pos1],"\0");
      _castdmg[id] += atoi(str);
      if (_fight[id] == 0 )
      {
         time( &_timeI[id] );
         _fight[id] = 1;
      }
   }
   if (debug) DebugSpewAlways("Leaving Dot");
}

void Event_Melee (char* line)
{
   char str[MAX_STRING] = "\0";
   char name[MAX_STRING] = "\0";
   int pos = 0;
   int pos1 = 0;
   int pos2 = 0;
   int id = 0;
   char h[14][20] = {"hit","pierce","claws","backstab","punch","kick","crush","slice","bite","bash","slash","maul","sting","frenzy"};

   fight = 1;
   if (debug) DebugSpewAlways("Entering Melee");

   for (int i = 0; i <= 12; i++)
   {
      pos = (int)(strstr(line,h[i]) - line + 1);
      if ( pos >= 0)
      {
         strncpy(name,&line[0],pos-2);
         strcpy(&name[pos-2],"\0");
         if (IdInList(name) == -1) AddToList(name);
      }
   }

   if (debug) DebugSpewAlways("Melee point 2");
   if ((id = IdInList(name)) != -1 )
   {
      pos1 = (int)(strstr(line,"for") - line) + 4;
      pos2 = (int)(strstr(line,"point") - line) - 1;
      strncpy(str, &line[pos1], pos2 - pos1);
      strcpy(&str[pos2-pos1],"\0");
      if (atoi(str) <= 31999) _meleedmg[id] += atoi(str); //_meleedmg[id] += atoi(str);
      if (_fight[id] == 0 )
      {
         time( &_timeI[id] );
         _fight[id] = 1;
      }
   }
   if (debug) DebugSpewAlways("Leaving Melee");
}

void Event_OtherMeleeDmg (char* line)
{
   // Actually Paladin Slay Undead
   char str[MAX_STRING] = "\0";
   char name[MAX_STRING] = "\0";
   int pos = 0;
   int pos1 = 0;
   int pos2 = 0;
   int id = 0;
   char h[1][20] = {"'s"};

   //fight = 1;
   if (debug) DebugSpewAlways("Entering OtherMelee");

   for (int i = 0; i <= 0; i++)
   {
      pos = (int)(strstr(line,h[i]) - line + 1);
      if ( pos >= 0)
      {
         strncpy(name,&line[0],pos-2);
         strcpy(&name[pos-2],"\0");
         if (IdInList(name) == -1) AddToList(name);
      }
   }

   if (debug) DebugSpewAlways("Melee point 2");
   if ((id = IdInList(name)) != -1 )
   {
      pos1 = (int)(strstr(line,"(") - line) + 1;
      pos2 = (int)(strstr(line,")") - line) - 1;
      strncpy(str, &line[pos1], pos2 - pos1);
      strcpy(&str[pos2-pos1],"\0");
      _meleedmg[id] += atoi(str);
      if (_fight[id] == 0 )
      {
         time( &_timeI[id] );
         _fight[id] = 1;
      }
   }
   if (debug) DebugSpewAlways("Leaving OtherMelee");
}

void Event_Cast (char* line)
{
   char str[MAX_STRING] = "\0";
   char name[MAX_STRING] = "\0";
   int pos = 0;
   int pos1 = 0;
   int pos2 = 0;
   int id = 0;
   char h[1][20] = {"hit"};

   //fight = 1;
   if (debug) DebugSpewAlways("Entering Cast");

   for (int i = 0; i <= 0; i++)
   {
      pos = (int)(strstr(line,h[i]) - line + 1);
      if ( pos >= 0)
      {
         strncpy(name,&line[0],pos-2);
         strcpy(&name[pos-2],"\0");
         if (IdInList(name) == -1) AddToList(name);
      }
   }

   if ((id = IdInList(name)) != -1 )
   {
      pos1 = (int)(strstr(line,"for") - line) + 4;
      pos2 = (int)(strstr(line,"points") - line) - 1;
      strncpy(str, &line[pos1], pos2 - pos1);
      strcpy(&str[pos2-pos1],"\0");
      _castdmg[id] += atoi(str);
      if (_fight[id] == 0 )
      {
         time( &_timeI[id] );
         _fight[id] = 1;
      }
   }
   if (debug) DebugSpewAlways("Leaving Cast");
}

void Event_MobDead (char* line)
{
   char str[MAX_STRING] = "\0";
   time_t tmptime = 0;
   long _totaldmg = 0;
   PSPAWNINFO pChar = NULL;

   _mobtotaldmg = 0;
   _mobtotalcast = 0;

   if (debug) DebugSpewAlways("Entering MobDead");

   // Find the minimum value of _timeI[]
      int minIndex = 0;
       time_t maxVal = _timeI[minIndex];
      int nb = IdInList("\0");
       for (int i = 0; i < nb; i++) {
         if (_timeI[i] < _timeI[minIndex]) {
               minIndex = i;
         }
      }
      // Calculate elapsed time: TimeNow - FirstHitTime
      tmptime = 0;
      if (_timeI[minIndex] != 0)
      {
         time_t t = 0;
         time( &t );
         tmptime = t - _timeI[minIndex];
      }

   if (_absdps)
      for (int i = 0; i < IdInList("\0"); i++) {
         _timeI[i] = _timeI[minIndex];
         //sprintf(str,"\ag%d,%d\ax",tmptime,_timeI[i]);
         //WriteChatColor(str);
      }

   if (_sort) Sort(_sort);

   if (_petowner)
   {
      for (int i = 0; i <= IdInList("\0")-1; i++)
      {
         DebugSpewAlways("Entering MobDead, loop %d",i);

         pChar = GetCharFromPetName(_name[i]);
         //WriteChatColor("Before");
         //WriteChatColor(_name[i]);
         if (pChar != NULL)
         {
            //WriteChatColor(pChar->Name);
            if (IdInList(pChar->Name) != -1)
            {
               // Don't write pet stats, they will be wrote
               // when the char stats are written (see WriteStats)
               //WriteChatColor("non null, is in list");
               //WriteChatColor(pChar->Name);
            }
            else
            {
               // If the owner did not do any dmg he is not in
               // the list and don't have stats, displaying the name.
               //sprintf(str,"\ar#\ax %s",pChar->Name);
               //WriteChatColor(str);
               //WriteChatColor("non null");
               WriteStats(i,1);
            }
         }
         else
         {
            // Write player and players's pet stats.
            //WriteChatColor("null");
            WriteStats(i,0);
         }
      }
   }
   else {
      for (int i = 0; i <= IdInList("\0")-1; i++)
      {
         //WriteChatColor(_name[i]);
         WriteStats(i,1);
      }
   }

   if (_mobtotaldmg != 0)
   {
      if (tmptime != 0)
         sprintf(str,"\ay#\ax Mob total hps: \ar%d\ax (\ag%d\ax) in %d, DPS: \ay%d\ax", _mobtotaldmg, _mobtotalcast,tmptime, _mobtotaldmg / tmptime);
      else
         sprintf(str,"\ay#\ax Mob total hps: \ar%d\ax (\ag%d\ax)", _mobtotaldmg, _mobtotalcast);
      WriteChatColor(str);
      sprintf(str,"\ag-----------------------------------------------------\ax");
      WriteChatColor(str);

   //fight = 0;
   ResetGV();
   }

   if (debug) DebugSpewAlways("Leaving MobDead");
}

int IdInList (char* name)
{
   DebugSpewAlways("IdInList");

   if (strcmp(name,"You") == 0) name = ((PCHARINFO)pCharData)->Name;

   for (int i = 0; i <= MAX_CHARS - 1; i++)
   {
      if (strcmp(_name[i], name) == 0)
      {
         if (debug) DebugSpewAlways("Leaving IdInList, found %s", name);
         return i;
      }
   }

   if (debug) DebugSpewAlways("Leaving IdInList, not found");
   return -1;
}

void AddToList (char* name)
{
   PSPAWNINFO pSpawn = NULL;
   if (debug) DebugSpewAlways("AddToList(%s)",name);

   int res = 0;
   char str[MAX_STRING] = "\0";

   if (strcmp(name,"You") == 0) name = ((PCHARINFO)pCharData)->Name;

   res = IdInList("\0");

   if (debug) DebugSpewAlways("AddToList before sprintf");

   //sprintf(str,"Added to list %s %d %d",name,res,IsNpc(name));

   if (debug) DebugSpewAlways("AddToList after sprintf");

   if (_playerid == -1)
      if (strcmp(name,((PCHARINFO)pCharData)->Name) == 0) _playerid = res;
   if (debug) DebugSpewAlways("AddToList(%s), between if",name);
   if (_petid == -1)
      pSpawn = GetPetFromCharName(((PCHARINFO)pCharData)->Name);
      if (pSpawn != NULL) if (strcmp(name,pSpawn->Name ) == 0) _petid = res;
   if (debug) DebugSpewAlways("AddToList(%s), after ifs",name);

   strcpy(_name[res],name);
   _isnpc[res] = IsNpc(name);
   //WriteChatColor(str);

   if (debug) DebugSpewAlways("Leaving AddToList(%s)",name);
}

void ResetGV (void)
{
   for (int j = 0; j <= MAX_CHARS-1; j++)
   {
      strcpy (_name[j],"\0");
      _meleedmg[j] = 0;
      _castdmg[j] = 0;
      _timeI[j] = 0;
      _timeF[j] = 0;
      _fight[j] = 0;
      _isnpc[j] = 0;
      _written[j] = 0;
   }
   _playerid = -1;
   _petid = -1;
}

void WriteStats(long i, bool pet)
{
   // Count total damages and display char stats, recursive function.

   char str[MAX_STRING] = "\0";
   char strout[MAX_STRING] = "\0";
   char petname[MAX_STRING] = "\0";
   long tmptime = 0;
   long totaldmg = 0;
   PSPAWNINFO pPet = NULL;

   if (_written[i] == 0 && (_meleedmg[i] != 0 || _castdmg[i] != 0) )
   {
      time( &_timeF[i] );
      time_t tmptime = _timeF[i] - _timeI[i];

      totaldmg = _meleedmg[i] + _castdmg[i];

      if (_isnpc[i] != 1)
      {
         _mobtotalcast += _castdmg[i];
         _mobtotaldmg += totaldmg;
      }

      if (debug) DebugSpewAlways("Entering MobDead, loop %d, before IF",i);

      if (pet) strcat(strout, "     ");

      sprintf(str,"\ar#\ax");
      strcat(strout,str);

      if (_isnpc[i] != 1)
         sprintf(str," %s ",_name[i]);
      else
         sprintf(str," \ar%s\ax ",_name[i]);
      strcat(strout,str);

      sprintf(str,"total dmg: \ar%d\ax (\ag%d\ax)",totaldmg,_castdmg[i]);
      strcat(strout,str);

      if (tmptime != 0)
      {
         sprintf(str," in %d, DPS: \ay%d\ax",tmptime,totaldmg/tmptime);
         strcat(strout,str);
      }

      if (debug) DebugSpewAlways("MobDead, loop %d, before IF: %s",i,strout);
      WriteChatColor(strout);
      _written[i] = 1;

      pPet = GetPetFromCharName(_name[i]);
      if (pPet != NULL)
      {
         sprintf(petname, "%s", pPet->Name);
         CleanupName(petname,FALSE);
         if (strstr(petname, "#") != NULL) strcpy(petname,&petname[1]);
         int tmpID = IdInList(petname);
         if ( tmpID != -1)
         {
            WriteStats(tmpID, 1);
         }
      }
   }
}

int IntArrayMax(int a[], int size)
{
   //char str[MAX_STRING);
   if (size == -1) return -1;
   //assert(size > 0);        // Note 1.
    int maxVal = a[0];     // Note 2.
    for (int i=1; i<size; i++) {
        if (a[i] > maxVal) {
            maxVal = a[i];
        }
    }
    return maxVal;
}

void Sort (int opt)
{
   if (debug) DebugSpewAlways("Entering Sort");

   char str[MAX_STRING] = {0};
   int maxid = 0;
   long max = 0;
   char max_name[MAX_STRING] = {0};
   long max_meleedmg = 0;
   long max_castdmg = 0;
   time_t max_timeI = 0;
   bool max_isnpc = 0;
   int listsize = IdInList("\0");

   //sprintf(str,"OPT: %d",opt);
   WriteChatColor(str);

   if (debug) DebugSpewAlways("SORT point 1");
   //WriteChatColor("Sorting ...");
   //for (int h= 0; h < listsize; h++)
   //{
      //sprintf (str,"%d, value: %d",h,_meleedmg[h]);
      //WriteChatColor(str);
   //}
   for (int i = 0; i < listsize; i++)
   {
      maxid = i;
      for (int j = i; j < listsize; j++)
      {
         //sprintf (str,"j = %d",j);
         //WriteChatColor(str);
         switch (opt)
         {
            case MELEE_DMG:
               if (_meleedmg[j] > _meleedmg[maxid])
               {
                  //sprintf (str,"%d-%d maxid = %d, anc: %d , %d>%d.",i,j,j,maxid,_meleedmg[j],_meleedmg[maxid]);
                  //WriteChatColor(str);
                  maxid = j;
                  //sprintf (str,"maxid = %d",maxid);
                  //WriteChatColor(str);
               }
               break;
            case CAST_DMG:
               if (_castdmg[j] > _castdmg[maxid])
                  maxid = j;
               break;
            case TOTAL_DMG:
               if (_meleedmg[j] + _castdmg[j] > _meleedmg[maxid] + _castdmg[maxid])
                  maxid = j;
               break;
            case DPS:
               if (_paramDps(j) > _paramDps(maxid))
               {
                  //sprintf (str,"Switching %d with %d(m): %d>%d.",j,maxid,_paramDps(j),_paramDps(maxid));
                  //WriteChatColor(str);
                  maxid = j;
               }
               break;
         }
      }
      if (debug) DebugSpewAlways("SORT point 2");

      //Name
      strcpy(max_name , _name[i]);
      strcpy(_name[i] , _name[maxid]);
      strcpy(_name[maxid] , max_name);
      //Melee dmg
      max_meleedmg = _meleedmg[i];
      _meleedmg[i] = _meleedmg[maxid];
      _meleedmg[maxid] = max_meleedmg;
      //Cast dmg
      max_castdmg = _castdmg[i];
      _castdmg[i] = _castdmg[maxid];
      _castdmg[maxid] = max_castdmg;
      //TimeI
      max_timeI = _timeI[i];
      _timeI[i] = _timeI[maxid];
      _timeI[maxid] = max_timeI;
      //Isnpc
      max_isnpc = _isnpc[i];
      _isnpc[i] = _isnpc[maxid];
      _isnpc[maxid] = max_isnpc;
   }
   if (debug) DebugSpewAlways("Leaving Sort");
}
void ReadWindowINI(PCSIDLWND pWindow)
{
   CHAR Buffer[MAX_STRING] = {0};
   pWindow->Location.top      = GetPrivateProfileInt("Settings","ChatTop",      357,INIFileName);
   pWindow->Location.bottom   = GetPrivateProfileInt("Settings","ChatBottom",      620,INIFileName);
   pWindow->Location.left      = GetPrivateProfileInt("Settings","ChatLeft",      164,INIFileName);
   pWindow->Location.right    = GetPrivateProfileInt("Settings","ChatRight",      375,INIFileName);
   pWindow->Locked             = GetPrivateProfileInt("Settings","Locked",         0,INIFileName);
   pWindow->Fades             = GetPrivateProfileInt("Settings","Fades",         1,INIFileName);
   pWindow->TimeMouseOver       = GetPrivateProfileInt("Settings","Delay",         2000,INIFileName);
   pWindow->FadeDuration       = GetPrivateProfileInt("Settings","Duration",      500,INIFileName);
   pWindow->Alpha            = GetPrivateProfileInt("Settings","Alpha",         255,INIFileName);
   pWindow->FadeToAlpha      = GetPrivateProfileInt("Settings","FadeToAlpha",   255,INIFileName);
   pWindow->BGType            = GetPrivateProfileInt("Settings","BGType",         1,INIFileName);
   pWindow->BGColor.R         = GetPrivateProfileInt("Settings","BGTint.red",      255,INIFileName);
   pWindow->BGColor.G         = GetPrivateProfileInt("Settings","BGTint.green",   255,INIFileName);
   pWindow->BGColor.B         = GetPrivateProfileInt("Settings","BGTint.blue",   255,INIFileName);

   GetPrivateProfileString("Settings","WindowTitle","DPS Window",Buffer,MAX_STRING,INIFileName);
   SetCXStr(&pWindow->WindowText,Buffer);
}

void WriteWindowINI(PCSIDLWND pWindow)
{
   CHAR szTemp[MAX_STRING] = {0};
   if (pWindow->Minimized)
   {
      WritePrivateProfileString("Settings","ChatTop",      itoa(pWindow->OldLocation.top,      szTemp,10),INIFileName);
      WritePrivateProfileString("Settings","ChatBottom",   itoa(pWindow->OldLocation.bottom,   szTemp,10),INIFileName);
      WritePrivateProfileString("Settings","ChatLeft",   itoa(pWindow->OldLocation.left,      szTemp,10),INIFileName);
      WritePrivateProfileString("Settings","ChatRight",   itoa(pWindow->OldLocation.right,   szTemp,10),INIFileName);
   }
   else
   {
      WritePrivateProfileString("Settings","ChatTop",      itoa(pWindow->Location.top,         szTemp,10),INIFileName);
      WritePrivateProfileString("Settings","ChatBottom",   itoa(pWindow->Location.bottom,      szTemp,10),INIFileName);
      WritePrivateProfileString("Settings","ChatLeft",   itoa(pWindow->Location.left,      szTemp,10),INIFileName);
      WritePrivateProfileString("Settings","ChatRight",   itoa(pWindow->Location.right,      szTemp,10),INIFileName);
   }
   WritePrivateProfileString("Settings","Locked",      itoa(pWindow->Locked,         szTemp,10),INIFileName);

   GetCXStr(pWindow->WindowText,szTemp);
   WritePrivateProfileString("Settings","WindowTitle",                              szTemp,INIFileName);

   WritePrivateProfileString("Settings","Fades",      itoa(pWindow->Fades,            szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","Delay",      itoa(pWindow->MouseOver,         szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","Duration",   itoa(pWindow->FadeDuration,         szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","Alpha",      itoa(pWindow->Alpha,            szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","FadeToAlpha",   itoa(pWindow->FadeToAlpha,         szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","BGType",      itoa(pWindow->BGType,            szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","BGTint.red",   itoa(pWindow->BGColor.R,         szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","BGTint.green",   itoa(pWindow->BGColor.G,      szTemp,10),INIFileName);
   WritePrivateProfileString("Settings","BGTint.blue",   itoa(pWindow->BGColor.B,         szTemp,10),INIFileName);
}
/*
int TimeArrayMaxArray(time_t a[], int size)
{
   //char str[MAX_STRING);
   if (size == -1) return -1;

   //assert(size > 0);        // Note 1.
   int minIndex = 0;
    time_t maxVal = a[minIndex];     // Note 2.
    for (int i=1; i<size; i++) {
        if (a[i] < a[minIndex]) {
            minIndex = i;
        }
    }
    return minIndex;
}
*/
void CmdDps(PSPAWNINFO pChar, PCHAR szLine)
{
   char Arg1[MAX_STRING] = {0};
   char Arg2[MAX_STRING] = {0};
   char str[MAX_STRING] = "/0";

   GetArg(Arg1,szLine,1);
   GetArg(Arg2,szLine,2);
   //WriteChatColor(Arg2);

   if (strlen(Arg1) == 0)
   {
      Usage();
   }
   else
   {
      if (strcmp(Arg1,"on") == 0)   {
         sprintf (str,"DPS PLUGIN: Dps is now ON");
         _ON = 1;
         WriteChatColor(str,CONCOLOR_YELLOW);
         //INI
         char val[MAX_STRING] = {0};
         sprintf (val,"%d",_ON);
         WritePrivateProfileString("General Options","ON",(LPCSTR)val,INIFileName);
      }
      else if (strcmp(Arg1,"off") == 0) {
         sprintf (str,"DPS PLUGIN: Dps is now OFF");
         _ON = 0;
         WriteChatColor(str,CONCOLOR_YELLOW);
         //INI
         char val[MAX_STRING] = {0};
         sprintf (val,"%d",_ON);
         WritePrivateProfileString("General Options","ON",(LPCSTR)val,INIFileName);
      }
      else if (strcmp(Arg1,"reset") == 0) {
         sprintf (str,"DPS PLUGIN: variables reset");
         ResetGV();
         WriteChatColor(str,CONCOLOR_YELLOW);
      }
      else if (strcmp(Arg1,"showwindow") == 0) {
         if (MyWnd)
            {
               delete MyWnd;
               MyWnd=0;
            }

         DebugTry(CScreenPieceTemplate *templ=pSidlMgr->FindScreenPieceTemplate("DpsWindow"));
         if (templ)
         MyWnd = new CDpsWnd;
         ReadWindowINI((PCSIDLWND) MyWnd);
      }
      else if (strcmp(Arg1,"logonwindow") == 0) {
         if (strcmp(Arg2,"true") == 0 || strcmp(Arg2,"TRUE") == 0 || strcmp(Arg2,"") == 0) {
            _logonshowwindow = 1;
            sprintf (str,"DPS PLUGIN: Displaying DPS Window at logon.");
            WriteChatColor(str,CONCOLOR_YELLOW);}
         else if (strcmp(Arg2,"false") == 0 || strcmp(Arg2,"FALSE") == 0) {
            _logonshowwindow = 0;
            sprintf (str,"DPS PLUGIN: No longer displaying DPS Window at logon.");
            WriteChatColor(str,CONCOLOR_YELLOW);}
         else { Usage(); }
         //INI
         char val[MAX_STRING] = {0};
         sprintf (val,"%d",_logonshowwindow);
         WritePrivateProfileString("Window","LogonShow",(LPCSTR)val,INIFileName);
      }
      else if (strcmp(Arg1,"sort") == 0) {
         if (strcmp(Arg2,"melee") == 0 || strcmp(Arg2,"MELEE") == 0) {
            _sort = MELEE_DMG;
            sprintf (str,"DPS PLUGIN: Sorting by Melee dmg.");
            WriteChatColor(str,CONCOLOR_YELLOW);
         }
         else if (strcmp(Arg2,"cast") == 0 || strcmp(Arg2,"CAST") == 0) {
            _sort = CAST_DMG;
            sprintf (str,"DPS PLUGIN: Sorting by Cast dmg.");
            WriteChatColor(str,CONCOLOR_YELLOW);
         }
         else if (strcmp(Arg2,"total") == 0 || strcmp(Arg2,"TOTAL") == 0) {
            _sort = TOTAL_DMG;
            sprintf (str,"DPS PLUGIN: Sorting by Total dmg.");
            WriteChatColor(str,CONCOLOR_YELLOW);
         }
         else if (strcmp(Arg2,"dps") == 0 || strcmp(Arg2,"DPS") == 0) {
            _sort = DPS;
            sprintf (str,"DPS PLUGIN: Sorting by dps.");
            WriteChatColor(str,CONCOLOR_YELLOW);
         }
         else if (strcmp(Arg2,"none") == 0 || strcmp(Arg2,"NONE") == 0) {
            _sort = NO_SORT;
            sprintf (str,"DPS PLUGIN: Not sorting.");
            WriteChatColor(str,CONCOLOR_YELLOW);
         }
         else { Usage(); }

         char val[MAX_STRING] = {0};
         sprintf (val,"%d",_sort);
         WritePrivateProfileString("General Options","Sort",(LPCSTR)val,INIFileName);
      }
      else if (strcmp(Arg1,"petfollowowner") == 0) {
         if (strcmp(Arg2,"true") == 0 || strcmp(Arg2,"TRUE") == 0 || strcmp(Arg2,"") == 0) {
            _petowner = 1;
            sprintf (str,"DPS PLUGIN: Pet'stats will follow owner.");
            WriteChatColor(str,CONCOLOR_YELLOW);
            }
         else if (strcmp(Arg2,"false") == 0 || strcmp(Arg2,"FALSE") == 0) {
            _petowner = 0;
            sprintf (str,"DPS PLUGIN: Pet'stats will not follow owner.");
            WriteChatColor(str,CONCOLOR_YELLOW);
            }
         else { Usage(); }

         char val[MAX_STRING] = {0};
         sprintf (val,"%d",_petowner);
         WritePrivateProfileString("General Options","PetFollowOwner",(LPCSTR)val,INIFileName);
      }
      else if (strcmp(Arg1,"absolutedps") == 0) {
         if (strcmp(Arg2,"true") == 0 || strcmp(Arg2,"TRUE") == 0 || strcmp(Arg2,"") == 0) {
            _absdps = 1;
            sprintf (str,"DPS PLUGIN: Absolute dps on.");
            WriteChatColor(str,CONCOLOR_YELLOW);
            }
         else if (strcmp(Arg2,"false") == 0 || strcmp(Arg2,"FALSE") == 0) {
            _absdps = 0;
            sprintf (str,"DPS PLUGIN: Absolute dps off.");
            WriteChatColor(str,CONCOLOR_YELLOW);
            }
         else { Usage(); }

         char val[MAX_STRING] = {0};
         sprintf (val,"%d",_absdps);
         WritePrivateProfileString("General Options","AbsoluteDps",(LPCSTR)val,INIFileName);
      }
      // Focus
      else if (strcmp(Arg1,"focus") == 0) {
         if (strcmp(Arg2,"") == 0 || strcmp(Arg2,"MYTARGET") == 0) {
            PSPAWNINFO psTarget = NULL;
            if (ppTarget && pTarget) {
            psTarget = (PSPAWNINFO)pTarget;
            strcpy(strFocusName,psTarget->DisplayedName);
            _focusdps = 1;
            sprintf (str,"DPS PLUGIN: Focus on %s.",strFocusName);
            } else {
            sprintf(str, "DPS PLUGIN: Focus requires a name parameter or a current target!");
            }
            WriteChatColor(str,CONCOLOR_YELLOW);
            }
         else {
            strcpy(strFocusName,Arg2);
            _focusdps = 1;
            sprintf (str,"DPS PLUGIN: Focus on %s.",strFocusName);
            WriteChatColor(str,CONCOLOR_YELLOW);
            }
         //else { Usage(); }

         //char val[MAX_STRING] = {0};
         //sprintf (val,"%d",_absdps);
         //WritePrivateProfileString("General Options","AbsoluteDps",(LPCSTR)val,INIFileName);
      }
      else
      {
         Usage();
      }
   }
}

void Usage(void)
{
   char str[MAX_STRING] = {0};
   //sprintf (str,"Syntax: /dps (on|off|reset| petfollowowner[true,false] | logonwindow[true,false] | showwindow | sort[none,melee,cast,total,dps])");
   //WriteChatColor(str,CONCOLOR_RED);
   WriteChatColor("Syntax: /dps option value, ex: /dps sort dps",USERCOLOR_DEFAULT);
   WriteChatColor("   on|off|reset , activate dps parsing or not. reset parser",USERCOLOR_DEFAULT);
   WriteChatColor("   petfollowowner[true,false], Pet's stats are displayed after owner's stats.",USERCOLOR_DEFAULT);
   WriteChatColor("   logonwindow[true,false], Activate the window wehn u log on.",USERCOLOR_DEFAULT);
   WriteChatColor("   showwindow[true,false], Make the window appear.",USERCOLOR_DEFAULT);
   WriteChatColor("   sort[none,melee,cast,total,dps], Sort player's dps.",USERCOLOR_DEFAULT);
   WriteChatColor("   absolutedps[true,false], Timers used for calculating player's dps is the same.",USERCOLOR_DEFAULT);
}

int _paramDps (int id)
{
   if (_fight[id])
   {
      time_t tmptime = 0;
      char str[MAX_STRING] = "/0";
      strcpy(str, "\0");
      time_t t = 0;

      time( &t );
      tmptime = t - _timeI[id];

      if (tmptime != 0)
      {
         return (_meleedmg[id] + _castdmg[id]) / (int)tmptime;
      }
   }
   return -1;
}

BOOL dataDps(PCHAR szName, MQ2TYPEVAR &Ret)
{
   if (szName != NULL)
   {
      int id = atoi(szName);
      pDpsType->SetIndex (id);
   }
   Ret.DWord=1;
   Ret.Type=pDpsType;
   return true;
}