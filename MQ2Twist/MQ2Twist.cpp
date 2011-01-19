
// MQ2Twist.cpp - Bard song twisting plugin for MacroQuest2
//
//    koad 03-24-04 Original plugin (http://macroquest.sourceforge.net/phpBB2/viewtopic.php?t=5962&start=2)
//    CyberTech 03-31-04 w/ code/ideas from Falco72 & Space-boy
//    Cr4zyb4rd 08-19-04 taking over janitorial duties
//    Pheph 08-24-04 cleaning up use of MQ2Data
//    Simkin 12-17-07 Updated for Secrets of Faydwer 10 Songs
//    MinosDis 05-01-08 Updated to fix /twist once
//    Itchybottom 04-03-09 Updated for EQEMU, and it's retarded quirks

/*
   MQ2Twist Version 1.4

      Usage:   
         /twist # # # # # - Twists in the order given.
            Valid options are 1 thru 10 for song gems, and 11  thru 19 for item clicks.
            These may be mixed in any order, and repeats are allowable. Up to 10 may be
            specified.
            If a song is specified with a duration longer than standard (ie, selos)
            that song will be twisted based on it's duration.  For example, riz+mana+selos
            would be a 2 song twist with selos pulsed every 2.5 min.
         /twist once # # # # # - Twists in the order given, then reverts to original twist
         /twist hold <gem #> - Pause twisting and sing only the specified song
         /sing <gem#> - alias for /twist hold
         /twist stop/end/off - stop twisting, does not clear the twist queue
         /stoptwist - alias for above
         /twist or /twist start - Resume the twist after using /twist hold or /twist stop
         /twist reset - Reset timers for item clicks and long duration songs
         /twist delay # - 10ths of a second, minimum of 30, default 33
         /twist adjust # - in ticks, how early to recast long duration songs
         /twist reload - reload the INI file to update item clicks
         /twist slots - List the slots/items defined in the INI and their #'s
         /twist quiet - Toggles songs listing and start/stop messages for one-shot twists

      ----------------------------
      Item Click Method:
       MQ2Twist uses /itemnotify slotname rightmouseup to perform item clicks.

       The INI file allows you to specify items by name (with name=itemname), or by
       inventory slot (with slot=slotname).  If both a name and slot are defined for an
       item, the plugin will attempt to swap the item into that slot (via the /exchange
       command) and replace the original item when casting is complete.
     
       The example INI file below contains examples of the types of usage.

      ----------------------------
      Examples:
         /twist 1
            Sing gem 1 forever
         /twist 1 2 3
            Twist gems 1,2, and 3 forever
         /twist 1 2 3 10
            Twist gems 1,2,3, and clicky 10, forever
         /twist hold 4 or /sing 4
            Sing gem 4 until another singing-related /twist command is given

      ----------------------------
      MQ2Data Variables:
         bool   Twist         Currently Twisting: true/false, if NULL plugin is not loaded
         Members:
            bool     Twisting  Currently twisting: true/false.
            int      Current   Returns the curent gem number being sung, -1 for item, or 0 if not twisting
            int      Next      Returns the next gem number to be sung, -1 for item, or 0 if not twistsing
            string   List      Returns the twist sequence in a format suitable for /twist

      ----------------------------
     
     The ini file has the format:
         [MQ2Twist]
         Delay=32       Delay between twists. Lag & System dependant.
         Adjust=1       This defines  how many ticks before the 'normal' recast time to cast a long song.
                        Long songs are defined as songs greater than 3 ticks in length.  If set to 1 tick,
                        and a song lasts 10 ticks, the song will be recast at the 8 tick mark, instead of
                        at the 9 tick mark as it normally would.

         [Click_11] thru [Click_19]
         CastTime=30              Casting Time, -1 to use the normal song delay
         ReCastTime=0             How often to recast, 0 to twist normally.
         Name="Fife of Battle"    Item name for /itemnotify
         Slot=neck                Slot name for /itemnotify

         Delay, CastTime and ReCastTime are specified in 10ths of a
         second, so 10 = 1 second, and so on.

         INI File Example:
            [MQ2Twist]
            Delay=31
            Quiet=0

            ;Shadowsong cloak
            [Click_11]
            CastTime=30
            ReCastTime=350
            Name=Shadowsong Cloak
            Slot=DISABLED

            ;girdle of living thorns (current belt will be swapped out)
            [Click_12]
            CastTime=0
            ReCastTime=11600
            Name=Girdle of Living Thorns
            Slot=waist

            ;nature's melody
            [Click_13]
            CastTime=-1
            ReCastTime=135
            Name=DISABLED
            Slot=mainhand

            ;lute of the flowing waters
            [Click_14]
            CastTime=0
            ReCastTime=0
            Name=Lute of the Flowing Waters
            Slot=DISABLED

            [Click_15] ... [Click_19]
            CastTime=33
            ReCastTime=0
            Name=DISABLED
            Slot=DISABLED

      ----------------------------

Changes:
   12-23-08 
      Updated array lengths to add more clicky slots

   12-17-07
      Support  items Secrets of Faydwer 10 Songs
     
   10-05-04
      Support "swap in and click" items

   09-15-04
      Support extra spell slot from Omens of War AA

   09-01-04
      Command: /twist quiet to toggle some of the spam on/off
      Various code fixes/speedups

   08-29-04
      Moved LONGSONG_ADJUST into INI file and made /twist adjust command to set it on
      the fly

   08-25-04
      Changed output for /twist once to be slightly less misleading
      Reset click/song timers every time they're called with /twist hold or /twist once;
      if the user's specifying that song, they obviously want to cast it anyway.
      Removed the variable MissedNote as close inspection revealed the only place it was
      checked for was the line that set it. /boggle
      Minor code tweaks, cleanups, formatting changes, etc

   08-24-04 (Pheph)
      Modified it to use only one TLO, as I found it somewhat messy having 4 different ones.
      All the functionality of the old TLO's are now members of ${Twist}
      ${Twising} is now ${Twist.Twisting}, or just ${Twist}
      ${TwistCurrent} is now ${Twist.Current}
      ${TwistNext} is now ${Twist.Next}
      ${TwistList} is now ${Twist.List}

   08-23-04
     Reset_ItemClick_Timers was being called far too often.  Now the only time we reset
     is if a new list of songs are specified.  "/twist ${TwistList}" is a useful alias
     if you for some reason want the old behavior.
     Sing or /twist hold now resets the cast/item timer for that song only, rather than
     the entire list.
     Command: /twist reset calls Reset_ItemClick_Timers without interfering with the
     state of the current twists.
   
   08-22-04
     Command: /twist once [songlist] will cycle through the songs entered once, then
     revert to the old twist, starting with the song that was interrupted.
     Removed command "/twist on", it was making the string compare for "once" annoying,
     and I didn't think it was worth the effort for a redundant command.
     /twist delay with no argument now returns the delay without resetting it.  Values
     less than 30 now give a warning...maybe they're not bards or have some other
     reason for using a low value.

   08-19-04
      Minor revamp of item notification.  Removed ITEMNOTIFY define and kludged in some
      changes from Virtuoso65 to get casting by item name working.  /cast is no longer
      used.
      Added INI file support for above change.  File now uses distinct entries for item
      names and slots.  *Quotes not required for multi-word item names in INI.*
      Fixed the MQ2Data value TwistCurrent to display the current song as-advertised, and
      added a new value TwistNext with the old behavior of showing the next song in the
      queue. (Useful in scripting)
      Removed a few DebugSpews that were mega-spamming my debugger output.
      CastTime of -1 in the INI file now causes the default delay to be used.
   
   06-01-04
      Added LONGSONG_ADJUST (default to 1 tick) to help with the timing of recasting long
      songs, such as selo's.
      Twisting is now paused when you sit (this would include camping).  This fixes
      problems reported by Chyld989 (twisting across chars) and Kiniktoo (new autostand on
      cast 'feature' in EQ makes twisting funky)

   05-19-04
      Added workaround for incorrect duration assumption for durationtype=5 songs, such as
      Cassindra's Chant of Clarity or Cassindra's Chorus of Clarity.
      Added check of char state before casting a song. Actually added for 1.05
         Checked states and resulting action are:
            Feigned, or Ducking = /stand
            Stunned = Delay
            Dead - Stop twisting.
         If you're a monk using this to click your epic, you'll want to disable the autostand on feign code =)


   05-05-05
      Fixed CTD on song unmem or death, while twisting.  Oops
      Removed circle functionality.  It's better suited for a plugin like the MQ2MoveUtils
         plugin by tonio at http://macroquest.sourceforge.net/phpBB2/viewtopic.php?t=6973

   05-01-04
      Fixed problem with using pchar before state->ingame causing CTD on eq load (thanks MTBR)
      Fixed vc6 compile error w/ reset_itemclick_timers
      Replaced various incantations of pChar and pSpawn with GetCharInfo()
      Fixed /circle behavior w/ unspecified y/x
      Fixed /circle on when already circling and you want to update loc
      Added output of parsed circle parameters on start.

   04-25-04
      Converted to MQ2Data
         Top Level Objects:
            bool   Twisting      (if NULL plugin is not loaded)
            int      TwistCurrent
            string   TwistList
      Removed $Param synatax for above
      Added check to make sure item twists specified are defined
      Fixed error with twist parameter processing
      Changed twist startup output to be more verbose
      Command: /twist on added as alias for /twist start
      INI File is now named per-character (MQ2Twist_Charname.ini)
         * Be sure to rename existing ini files
      Modified twist routine to take into account songs with
         non-0 recast times or longer than 3 tick durations,
         and only re-cast them after the appropriate delay.
         This is for songs like Selos 2.5 min duration, etc.
         * Note that this makes no attempt to recover if the song
         effect is dispelled, your macro will need to take care
         of that.
      Added ability to compile-time change the method used for
         clicking items.

   04-13-04
      Changed /circle command to allow calling w/o specifying loc
      Corrected a problem with multiple consecutive missed notes
      Added handling of attempting to sing while stunned
      Command: /twist slots, to list the slot to # associations
      Command: /twist reload, to reload the ini file on the fly
      Command: /twist end, /twist off as aliases for /twist stop
      Command: /sing #, as an alias for /twist hold #

      Added support for item clickies.  Clickies are specified
      as "gem" 10-19. For example, /twist 1 2 10 12

      Added INI file support for storing item clicky info
      and default twist delay.

   04-11-04
      Integrated the /circle code from Easar, runs in a circle.  type
      /circle for help.
*/

#include "../MQ2Plugin.h"

PreSetup("MQ2Twist");

typedef struct _ITEMCLICK {
   int cast_time;
   int recast;
   long castdue;
   int disabled;
   int nousename;
   CHAR slot[MAX_STRING];
   CHAR name[MAX_STRING];
} ITEMCLICK;

const int MAX_SONG=11;         // old MAX used for clickies, songs, and gems      
const int MAX_TWIST=20;         // how many total songs can be twisted .. /twist 1 2 3 1 2 4 ...
const int MAX_CLICKIES=20;      // how many clickies can we have
const int MAX_GEMS=10;         // how many spell gems are there


int MQ2TwistEnabled = 0;
int LONGSONG_ADJUST=1;          // In TICKS, not seconds.  Used for long songs (greater than 3 ticks in duration). See docs.
int CAST_TIME=35;
int NumSongs=0;               // number of songs in current twist   
int AltNumSongs=0;            // alt - if doing twist once - alt saves normal twist values.
int Song[MAX_TWIST];         // song[n] = stores twist list
int AltSong[MAX_TWIST];         // when twist once - saves noraml twist
long SongNextCast[MAX_TWIST];   // twist list cast time
ITEMCLICK ItemClick[MAX_CLICKIES];
int CurrSong=0;               // Something strange with CurrSong. Looks like CurrSong always 1+    
int AltCurrSong=0;
bool RecastAlt=false;
bool UsingAlt=false;
int PrevSong=0;
int LastAltSong=0;
int HoldSong=0;
long CastDue=0;
bool bTwist=false;
bool altTwist=false;
bool quiet;
CHAR SwappedOutItem[MAX_STRING];
CHAR SwappedOutSlot[MAX_STRING];

long GetTime();
VOID TwistCommand(PSPAWNINFO pChar, PCHAR szLine);
VOID StopTwistCommand(PSPAWNINFO pChar, PCHAR szLine);
VOID SingCommand(PSPAWNINFO pChar, PCHAR szLine);
BOOL dataTwist(PCHAR szIndex, MQ2TYPEVAR &Ret);
CHAR MQ2TwistTypeTemp[MAX_STRING]={0};

//get current timestamp in tenths of a second
long GetTime()
{
   SYSTEMTIME st;
   ::GetSystemTime(&st);
   long lCurrent=0;
   lCurrent  = st.wDay    * 24 * 60 * 60 * 10;
   lCurrent += st.wHour        * 60 * 60 * 10;
   lCurrent += st.wMinute           * 60 * 10;
   lCurrent += st.wSecond                * 10;
   lCurrent += (long)(st.wMilliseconds/100);
   return (lCurrent);
}

VOID MQ2TwistDoCommand(PSPAWNINFO pChar, PCHAR szLine)
{
   HideDoCommand(pChar, szLine, FromPlugin);
}

VOID DoSwapOut()
{
   CHAR szTemp[MAX_STRING];
   if (SwappedOutItem[0]) {
      sprintf(szTemp,"/exchange \"%s\" %s",SwappedOutItem,SwappedOutSlot);
      MQ2TwistDoCommand(NULL, szTemp);
      SwappedOutItem[0]=0;
   }
}

VOID DoSwapIn(int Index)
{
   CHAR szTemp[MAX_STRING];
   if (strnicmp(ItemClick[Index].slot,"DISABLED",8)) {
      sprintf(szTemp,"${InvSlot[%s].Item.Name}",ItemClick[Index].slot);
      ParseMacroData(szTemp);
      strcpy(SwappedOutItem,szTemp);
      strcpy(SwappedOutSlot,ItemClick[Index].slot);
      sprintf(szTemp,"/exchange \"%s\" %s",ItemClick[Index].name,ItemClick[Index].slot);
      MQ2TwistDoCommand(NULL, szTemp);
   }
}

VOID Reset_ItemClick_Timers()
{
   int i;
   for (i=0;i<MAX_CLICKIES;i++) {
      ItemClick[i].castdue = 0;
   }
   for (i=0;i<MAX_TWIST;i++) {
      SongNextCast[i] = 0;
   }
}


VOID Update_INIFileName() {
   if (GetCharInfo()) {
      sprintf(INIFileName,"%s\\MQ2Twist_%s.ini",gszINIPath,GetCharInfo()->Name);
   } else {
      sprintf(INIFileName,"%s\\MQ2Twist.ini",gszINIPath);
   }
}

VOID Load_MQ2Twist_INI()
{
   CHAR szTemp[MAX_STRING]={0};
   CHAR szSection[MAX_STRING]={0};

   Update_INIFileName();

   CAST_TIME = GetPrivateProfileInt("MQ2Twist","Delay",33,INIFileName);
   sprintf(szTemp, "%d", CAST_TIME);
   WritePrivateProfileString("MQ2Twist","Delay",szTemp,INIFileName);
   quiet = GetPrivateProfileInt("MQ2Twist","Quiet",0,INIFileName)? 1 : 0;
   sprintf(szTemp, "%d", quiet);
   WritePrivateProfileString("MQ2Twist","Quiet",szTemp,INIFileName);
   
   LONGSONG_ADJUST = GetPrivateProfileInt("MQ2Twist","Adjust",1,INIFileName);
   sprintf(szTemp, "%d", LONGSONG_ADJUST);
   WritePrivateProfileString("MQ2Twist","Adjust",szTemp,INIFileName);
   
   for (int i=0;i<MAX_CLICKIES;i++) {
      sprintf(szSection, "Click_%d", MAX_GEMS+1+i);
      ItemClick[i].cast_time = GetPrivateProfileInt(szSection,"CastTime",0,INIFileName);
      ItemClick[i].recast = GetPrivateProfileInt(szSection,"ReCastTime",0,INIFileName);

      GetPrivateProfileString(szSection,"Name","DISABLED",ItemClick[i].name,MAX_STRING,INIFileName);
      GetPrivateProfileString(szSection,"Slot","DISABLED",ItemClick[i].slot,MAX_STRING,INIFileName);
      if(!strnicmp("DISABLED", ItemClick[i].name, 8)) {
         if (!strnicmp("DISABLED", ItemClick[i].slot, 8)) {
            ItemClick[i].disabled = true;
            DebugSpew("MQ2Twist: Slot %d disabled",i+1);
         } else {
            ItemClick[i].nousename = true;
            ItemClick[i].disabled = false;
         }
      } else ItemClick[i].disabled = false;
      // Write the values above back to disk, mostly to initialize it for easy editing.
      sprintf(szTemp, "%d", ItemClick[i].cast_time);
      WritePrivateProfileString(szSection,"CastTime",szTemp,INIFileName);
      // If the CastTime is set to -1 in the INI file, use the default.
      ItemClick[i].cast_time = ItemClick[i].cast_time==-1 ? CAST_TIME : ItemClick[i].cast_time;

      sprintf(szTemp, "%d", ItemClick[i].recast);
      WritePrivateProfileString(szSection,"ReCastTime",szTemp,INIFileName);
      WritePrivateProfileString(szSection,"Name",ItemClick[i].name,INIFileName);
      WritePrivateProfileString(szSection,"Slot",ItemClick[i].slot,INIFileName);
      DebugSpewAlways("Initializing MQ2Twist: Processed %s", szSection);
   }
}

VOID SingCommand(PSPAWNINFO pChar, PCHAR szLine)
{
   CHAR szTemp[MAX_STRING]={0};
   CHAR szMsg[MAX_STRING]={0};
   int i;

   GetArg(szTemp,szLine,1);
   i=atoi(szTemp);

   if (i>=1 && i<=MAX_GEMS+MAX_CLICKIES) { // valid range? -- note range starts at 1 - array indexes 0.
      HoldSong = i;
      bTwist=true;
      CastDue = -1;
      sprintf(szMsg, "MQ2Twist::Holding Twist and casting gem %d", HoldSong);
      WriteChatColor(szMsg,USERCOLOR_DEFAULT);
      MQ2TwistDoCommand(pChar,"/stopsong");
      if (i>MAX_GEMS) { //item?
         ItemClick[i-MAX_GEMS-1].castdue = 0;
      } else SongNextCast[i] = 0; //nope, song -- this is I , but should it be I-1?
   } else WriteChatColor("MQ2Twist::Invalid gem specified, ignoring",USERCOLOR_DEFAULT);
}

VOID StopTwistCommand(PSPAWNINFO pChar, PCHAR szLine)
{
   bTwist=false;
   HoldSong=0;
   MQ2TwistDoCommand(pChar,"/stopsong");
   WriteChatColor("MQ2Twist::Stopping Twist",USERCOLOR_DEFAULT);
}

VOID PrepNextSong() {
   if (CurrSong>NumSongs) {
      if (altTwist) {
         NumSongs=AltNumSongs;
         CurrSong=PrevSong=AltCurrSong;
         for (int i=0; i<NumSongs; i++) Song[i]=AltSong[i];
         altTwist=false;
         if (!quiet) WriteChatColor("MQ2Twist::One-shot twist ended, normal twist will resume next pulse",USERCOLOR_DEFAULT);
      } else CurrSong=1;
   }
}

VOID DisplayTwistHelp() {
   WriteChatColor("MQ2Twist - Twist song or songs",USERCOLOR_DEFAULT);
   WriteChatColor("Usage:   /twist <gem#> - Twists in the order given.",USERCOLOR_DEFAULT);
   WriteChatColor("  Valid options are 1 thru 10 for song gems, and 11 thru 19 for item clicks.",USERCOLOR_DEFAULT);
   WriteChatColor("  These may be mixed in any order, and repeats are allowable.",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist hold <gem #> - Pause twisting and sing only the specified song",USERCOLOR_DEFAULT);
   WriteChatColor("  /sing <gem#> - alias for /twist hold",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist once <gem#> Twists once in the order given, then reverts to original twist",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist or /twist start - Resume the twist after using /twist hold or /twist stop",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist reset - Reset timers for item clicks and long duration songs",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist delay # - 10ths of a second, minimum of 30, default 33",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist adjust # - in ticks, how early to recast long duration songs",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist stop/end/off - stop twisting, does not clear the twist queue",USERCOLOR_DEFAULT);
   WriteChatColor("  /stoptwist - alias for /twist stop",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist reload - reload the INI file to update item clicks",USERCOLOR_DEFAULT);
   WriteChatColor("Usage: /twist slots - List the slots defined in the INI and their #'s",USERCOLOR_DEFAULT);
}

// **************************************************  *************************
// Function:      TwistCommand
// Description:   Our /twist command. sing for me!
// **************************************************  *************************
VOID TwistCommand(PSPAWNINFO pChar, PCHAR szLine)
{
   CHAR szTemp[MAX_STRING]={0};
   CHAR szMsg[MAX_STRING]={0};
   CHAR szChat[MAX_STRING]={0};
   PSPELL pSpell;
   int i;

   GetArg(szTemp,szLine,1);

   if (NumSongs && (!strlen(szTemp) || !strnicmp(szTemp,"start", 5))) {
      WriteChatColor("MQ2Twist::Starting Twist",USERCOLOR_DEFAULT);
      DoSwapOut();
      bTwist=true;
      HoldSong=0;
      CastDue = -1;
      return;
   }

   if (!strnicmp(szTemp,"stop", 4) || !strnicmp(szTemp,"end", 3) || !strnicmp(szTemp,"off", 3)) {
      DoSwapOut();
      StopTwistCommand(pChar, szTemp);
      return;
   }

   if (!strnicmp(szTemp,"slots", 5)) {
      WriteChatColor("MQ2Twist 'Song' Numbers for right click effects:",USERCOLOR_DEFAULT);
      for (i=0;i<10;i++) {
         if (ItemClick[i].disabled) break;
         if (ItemClick[i].nousename) {
            sprintf(szMsg, "  %d = %s (slot)", i+10, ItemClick[i].slot);
         } else {
            sprintf(szMsg, "  %d = %s (name) %s (slot)", i+10, ItemClick[i].name, ItemClick[i].slot);
       }
         WriteChatColor(szMsg,USERCOLOR_DEFAULT);
      }
      WriteChatColor("---",USERCOLOR_DEFAULT);
      return;
   }

   if (!strnicmp(szTemp,"reload", 6)) {
      WriteChatColor("MQ2Twist::Re-Loading INI Values",USERCOLOR_DEFAULT);
      Load_MQ2Twist_INI();
      return;
   }

   if (!strnicmp(szTemp,"delay", 5)) {
      GetArg(szTemp,szLine,2);
      if (strlen(szTemp)>0) {
         i=atoi(szTemp);
         if (i<=30) {
            WriteChatColor("MQ2Twist::WARNING delay specified is less than standard song cast time",CONCOLOR_RED);
         }
         CAST_TIME=i;
         Update_INIFileName();
         WritePrivateProfileString("MQ2Twist","Delay",itoa(CAST_TIME, szTemp, 10),INIFileName);
         sprintf(szMsg, "MQ2Twist::Set delay to %d, INI updated", CAST_TIME);
      } else sprintf(szMsg, "MQ2Twist::Delay %d", CAST_TIME);
      WriteChatColor(szMsg,USERCOLOR_DEFAULT);
      return;
   }
   
   if (!strnicmp(szTemp,"quiet", 5)) {
      quiet=!quiet;
      sprintf(szTemp,"%d",quiet);
      WritePrivateProfileString("MQ2Twist","Quiet",szTemp,INIFileName);
      sprintf(szMsg,"MQ2Twist::Now being %s",quiet ? "quiet" : "noisy");
      WriteChatColor(szMsg,USERCOLOR_DEFAULT);
      return;
   }
   
   if (!strnicmp(szTemp,"adjust", 6)) {
      GetArg(szTemp,szLine,2);
      if (strlen(szTemp)>0) {
         i=atoi(szTemp);
         LONGSONG_ADJUST=i;
         Update_INIFileName();
         WritePrivateProfileString("MQ2Twist","Adjust",itoa(LONGSONG_ADJUST, szTemp, 10),INIFileName);
         sprintf(szMsg, "MQ2Twist::Long song adjustment set to %d, INI updated", LONGSONG_ADJUST);
      } else sprintf(szMsg, "MQ2Twist::Long song adjustment: %d", LONGSONG_ADJUST);
      WriteChatColor(szMsg,USERCOLOR_DEFAULT);
      return;
   }

   if (!strnicmp(szTemp,"hold", 4)) {
      GetArg(szTemp,szLine,2);
      SingCommand(pChar, szTemp);
      return;
   }

   if (!strnicmp(szTemp,"reset", 5)) {
      Reset_ItemClick_Timers();
      WriteChatColor("MQ2Twist::Timers reset",CONCOLOR_YELLOW);
      return;
   }

   // check help arg, or display if we have no songs defined and /twist was used
   if (!strlen(szTemp) || !strnicmp(szTemp,"help", 4)) {
      DisplayTwistHelp();
      return;
   }

   // if we are "one-shot twisting", save the current song array and current song
   if (!strnicmp(szTemp,"once", 4)) {
      WriteChatColor("MQ2Twist one-shot twisting:",CONCOLOR_YELLOW);
      if (altTwist) {
         CurrSong=NumSongs+1;
         PrepNextSong(); // If CurrSong > NumSongs reload the song list
      }
      if (NumSongs) {
         AltNumSongs=NumSongs;
         AltCurrSong=CurrSong;
         for (i=0; i<NumSongs; i++) AltSong[i]=Song[i];
      }
      altTwist=true;
   } else altTwist=false;

   DoSwapOut();
   DebugSpew("MQ2Twist::TwistCommand Parsing twist order");
   NumSongs=0;
   HoldSong=0;
   if (!altTwist) {
      if (!quiet) {
         WriteChatColor("MQ2Twist Twisting:",CONCOLOR_YELLOW);
      } else WriteChatColor("MQ2Twist::Starting Twist",USERCOLOR_DEFAULT);
   }
   for (i=0 + altTwist ? 1 : 0; i<MAX_TWIST; i++)
   {
      GetArg(szTemp,szLine,i+1);
      if (!strlen(szTemp))  break;

     // MAX_GEMS = 10   MAX_CLICKIES = 20    /twist 11 --> 11-10-1
    
    
      Song[NumSongs]=atoi(szTemp);
      if (Song[NumSongs]>=1 && Song[NumSongs]<=MAX_GEMS+MAX_CLICKIES) {
         if ((Song[NumSongs]>MAX_GEMS) && ItemClick[Song[NumSongs]-MAX_GEMS-1].disabled) {
            sprintf(szChat, " Undefined item specified (%s) - ignoring (see INI file)", szTemp);
            WriteChatColor(szChat,CONCOLOR_RED);
         } else {
            sprintf(szMsg, " %s - ", szTemp);

            if (Song[NumSongs]<=MAX_GEMS) {
               pSpell=GetSpellByID(GetCharInfo2()->MemorizedSpells[Song[NumSongs]-1]);
               if (altTwist) SongNextCast[NumSongs] = 0;
               if (pSpell) strcat(szMsg, pSpell->Name);
            } else {
               if (ItemClick[Song[NumSongs]-MAX_GEMS-1].nousename) {
                  strcat(szMsg, ItemClick[Song[NumSongs]-MAX_GEMS-1].slot);
               } else {
                  strcat(szMsg, ItemClick[Song[NumSongs]-MAX_GEMS-1].name);
               }
               if (altTwist) ItemClick[NumSongs].castdue = 0;
            }
            if (!quiet) WriteChatColor(szMsg,COLOR_LIGHTGREY);
            NumSongs++;
         }
      } else {
         sprintf(szChat, " Invalid gem specified (%s) - ignoring", szTemp);
         WriteChatColor(szChat,CONCOLOR_RED);
      }
   }

   sprintf(szTemp, "Twisting %d song%s", NumSongs, NumSongs>1 ? "s" : "");
   if (!quiet) WriteChatColor(szTemp,CONCOLOR_YELLOW);

   if (NumSongs>0) bTwist=true;
   CurrSong = 1;
   PrevSong = 1;
   CastDue = -1;
   MQ2TwistDoCommand(pChar,"/stopsong");
   if (!altTwist) Reset_ItemClick_Timers();
}

/*
Checks to see if character is in a fit state to cast next song/item

Note 1: Do not try to correct SIT state, or you will have to stop the
twist before re-memming songs

Note 2: Since the auto-stand-on-cast bullcrap added to EQ a few patches ago,
chars would stand up every time it tried to twist a song.  So now
we stop twisting at sit.
*/
BOOL CheckCharState() {
   if (!bTwist) return FALSE;

   if (GetCharInfo()) {
      if (GetCharInfo()->Stunned==1) return FALSE;
      switch (GetCharInfo()->standstate) {
       case STANDSTATE_SIT:
          WriteChatColor("MQ2Twist::Stopping Twist",USERCOLOR_DEFAULT);
          bTwist = FALSE;
          return FALSE;
          break;
       case STANDSTATE_FEIGN:
          MQ2TwistDoCommand(NULL,"/stand");
          return FALSE;
          break;
       case STANDSTATE_DEAD:
          WriteChatColor("MQ2Twist::Stopping Twist",USERCOLOR_DEFAULT);
          bTwist = FALSE;
          return FALSE;
          break;
       default:
          break;
      }

	  //	  if(InHoverState()) {
//          bTwist=FALSE;
//          return FALSE;
//      }
   }

   if (pCastingWnd) {
      PCSIDLWND pCastingWindow = (PCSIDLWND)pCastingWnd;
      if (pCastingWindow->Show != 0) return FALSE;
      // Don't try to twist if the casting window is up, it implies the previous song
      // is still casting, or the user is manually casting a song between our twists
   }
   return TRUE;
}
class MQ2TwistType *pTwistType=0;

class MQ2TwistType : public MQ2Type
{
public:
   enum TwistMembers
   {
      Twisting=1,
      Next=2,
      Current=3,
      List=4,
   };

   MQ2TwistType():MQ2Type("twist")
   {
      TypeMember(Twisting);
      TypeMember(Next);
      TypeMember(Current);
      TypeMember(List);
   }
   ~MQ2TwistType()
   {
   }

   bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
   {
      PMQ2TYPEMEMBER pMember=MQ2TwistType::FindMember(Member);
      if (!pMember)
         return false;
      switch((TwistMembers)pMember->ID)
      {
      case Twisting:
         /* Returns: bool
            0 - Not Twisting
            1 - Twisting
         */
         Dest.Int=bTwist;
         Dest.Type=pBoolType;
         return true;
      case Next:
         /* Returns: int
            0 - Not Twisting
            -1 - Casting Item
            1-9 - Current Gem
         */
         Dest.Int=HoldSong ? HoldSong : Song[CurrSong-1];
         if (Dest.Int>9) Dest.Int = -1;
         if (!bTwist) Dest.Int = 0;

         Dest.Type=pIntType;
         return true;
      case Current:
         Dest.Int=HoldSong ? HoldSong : Song[PrevSong-1];
         if (Dest.Int>9) Dest.Int = -1;
         if (!bTwist) Dest.Int = 0;

         Dest.Type=pIntType;
         return true;
      case List:
         /* Returns: string
            Space separated list of gem and item #'s being twisted, in order
         */
         int a;
         CHAR szTemp[MAX_STRING] = {0};

         MQ2TwistTypeTemp[0] = 0;
         for (a=0; a<NumSongs; a++) {
            sprintf(szTemp, "%d ", Song[a]);
            strcat(MQ2TwistTypeTemp, szTemp);
         }

         Dest.Ptr=&MQ2TwistTypeTemp[0];
         Dest.Type=pStringType;
         return true;
      }
      return false;
   }

   bool ToString(MQ2VARPTR VarPtr, PCHAR Destination)
   {
      if (bTwist)
         strcpy(Destination,"TRUE");
      else
         strcpy(Destination,"FALSE");
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

BOOL dataTwist(PCHAR szName, MQ2TYPEVAR &Dest)
{
   Dest.DWord=1;
   Dest.Type=pTwistType;
   return true;
}


// ******************************
// **** MQ2 API Calls Follow ****
// ******************************

PLUGIN_API VOID InitializePlugin(VOID)
{
   DebugSpewAlways("Initializing MQ2Twist");

   AddCommand("/twist",TwistCommand,0,1,1);
   AddCommand("/sing",SingCommand,0,1,1);
   AddCommand("/stoptwist",StopTwistCommand,0,0,1);;
   AddMQ2Data("Twist",dataTwist);

   pTwistType = new MQ2TwistType;
}

PLUGIN_API VOID ShutdownPlugin(VOID)
{
   DebugSpewAlways("MQ2Twist::Shutting down");

   RemoveCommand("/twist");
   RemoveCommand("/sing");
   RemoveCommand("/stoptwist");
   RemoveMQ2Data("Twist");

   delete pTwistType;
}

PLUGIN_API VOID OnPulse(VOID)
{
   CHAR szTemp[MAX_STRING] = {0};
   CHAR mzTemp[MAX_STRING] = {0};
   PSPELL pSpell;
   int TmpRecastTimer, TmpSpellDuration;
   
   
   if (!MQ2TwistEnabled || !CheckCharState()) return;

   if ((HoldSong>0) || ((NumSongs==1) && !altTwist)) {
      // DebugSpew("MQ2Twist::Pulse - Single Song");
	   if ( (CastDue<0 && CheckCharState() == 1) || ( ((CastDue-GetTime()) <= 0 ) && (GetCharInfo()->pSpawn->CastingData.SpellID == -1) ) ) {
         int SongTodo = HoldSong ? HoldSong : Song[0];
         if (SongTodo <= MAX_GEMS) {
            DebugSpew("MQ2Twist::Pulse - Single Song (Casting Gem %d)", SongTodo);
			sprintf(szTemp,"/stopsong"); 
			sprintf(szTemp, "%d", CAST_TIME); 
			sprintf(szTemp,"/cast %d",SongTodo);
			MQ2TwistDoCommand(NULL,szTemp);
            CastDue = GetTime()+CAST_TIME;
         } else {
         int itemID = SongTodo-MAX_GEMS-1;
            if (ItemClick[itemID].castdue-GetTime() <= 0) {
               if (ItemClick[itemID].nousename) {
                  DebugSpew("MQ2Twist::Pulse - Single Song (Casting Item %d - %s)", SongTodo, ItemClick[itemID].slot);
                  sprintf(szTemp,"/itemnotify %s rightmouseup", ItemClick[itemID].slot);
			   } else {
                  DebugSpew("MQ2Twist::Pulse - Single Song (Casting Item %d - %s)", SongTodo, ItemClick[itemID].name);
                  DoSwapIn(itemID);
                  sprintf(szTemp,"/itemnotify ${FindItem[%s].InvSlot.Name} rightmouseup", ItemClick[itemID].name);
			   }
			   MQ2TwistDoCommand(NULL,"/stopsong");
			   MQ2TwistDoCommand(NULL,szTemp);
               ItemClick[itemID].castdue = ItemClick[itemID].recast ? (GetTime()+ItemClick[itemID].cast_time+ItemClick[itemID].recast) : (GetTime()+CAST_TIME);
               CastDue = ItemClick[itemID].castdue;
			}
         }
      }
   } else {
      UsingAlt=false;
      int SongTodo = Song[CurrSong-1];
     if (RecastAlt) {
        SongTodo=LastAltSong;
        RecastAlt=false;
     }
      if (NumSongs && ((CastDue-GetTime()) <= 0)) {
         DoSwapOut();
         if (SongTodo <= MAX_GEMS) {
            if (SongNextCast[CurrSong-1]-GetTime() <= 0 && CheckCharState() == 1) {
               DebugSpew("MQ2Twist::OnPulse - Next Song = %s", szTemp);
               //sprintf(szTemp,"/multiline ; /stopsong ; /cast %d", SongTodo);
			   sprintf(szTemp,"/stopsong");
			   sprintf(szTemp,"/cast %d", SongTodo);
            if (altTwist) {
               LastAltSong=SongTodo;
               UsingAlt=true;
            }
            PrevSong=CurrSong;
                       
               MQ2TwistDoCommand(NULL,szTemp);
               pSpell=GetSpellByID(GetCharInfo2()->MemorizedSpells[Song[CurrSong-1]-1]);
               if(!pSpell) {
                  WriteChatColor("Songs not present - suspending twist.  /twist to resume",CONCOLOR_RED);
                  bTwist = FALSE;
                  return;
               }
               CastDue = GetTime()+CAST_TIME;
               TmpRecastTimer = pSpell->RecastTime/100;
               TmpSpellDuration = GetSpellDuration(pSpell,GetCharInfo()->pSpawn)*60; // duration in 10's of a second

            // duration > 18 secs
            if (TmpSpellDuration > 180) SongNextCast[CurrSong-1] = CastDue + TmpSpellDuration - (LONGSONG_ADJUST*180);

            // recast > 0 secs/
            else if (TmpRecastTimer > 0) SongNextCast[CurrSong-1] = CastDue + TmpRecastTimer;

            // normal
            else SongNextCast[CurrSong-1] = CastDue;
               PrevSong=CurrSong;
            } // if it's not time for currsong to be re-sung, skip it in the twist
            CurrSong++;
            PrepNextSong();
         } else {
         int itemID = SongTodo-MAX_GEMS-1;
            if (ItemClick[itemID].castdue-GetTime() <= 0) {
               if (ItemClick[itemID].nousename) {
                  DebugSpew("MQ2Twist::Pulse - Next Song (Casting Slot %d - %s)", SongTodo, ItemClick[itemID].slot);
                  //sprintf(szTemp,"/multiline ; /stopsong ; /itemnotify %s rightmouseup", ItemClick[itemID].slot);
				  sprintf(szTemp,"/itemnotify %s rightmouseup", ItemClick[itemID].slot);
               } else {
                  DebugSpew("MQ2Twist::Pulse - Next Song (Casting Item %d - %s)", SongTodo, ItemClick[itemID].name);
                  DoSwapIn(itemID);
                  //sprintf(szTemp,"/multiline ; /stopsong ; /itemnotify ${FindItem[%s].InvSlot.Name} rightmouseup", ItemClick[itemID].name);
				  sprintf(szTemp,"/itemnotify ${FindItem[%s].InvSlot.Name} rightmouseup", ItemClick[itemID].name);
               }
			   MQ2TwistDoCommand(NULL,"/stopsong");
               MQ2TwistDoCommand(NULL,szTemp);
               ItemClick[itemID].castdue = ItemClick[itemID].recast ? (GetTime()+ItemClick[itemID].cast_time+ItemClick[itemID].recast) : (GetTime()+CAST_TIME);
               CastDue = GetTime()+ItemClick[itemID].cast_time;
            }
            PrevSong=CurrSong;   // Increment twist position even if we didn't do an itemnotify - this might have a long recast
            CurrSong++;         // interval set, and we just skip it until it's time to recast, rather than keep a separate timer.
            PrepNextSong();
         }   
      }
   }     
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
   CHAR szMsg[MAX_STRING]={0};
   if (!bTwist || !MQ2TwistEnabled) return 0;
    DebugSpew("MQ2Twist::OnIncomingChat(%s)",Line);
   
   if ( !strcmp(Line,"You miss a note, bringing your song to a close!") ||
        !strcmp(Line,"Your spell is interrupted.") ) {
      // DebugSpew("MQ2Twist::OnIncomingChat - Song Interrupt Event");
     if (!HoldSong) CurrSong=PrevSong;
     if (UsingAlt) RecastAlt=true;
      CastDue = -1;
      SongNextCast[CurrSong-1] = -1;
      return 0;
   }

   if (!strcmp(Line,"Your song ends.")) {
	   int song_ended = 0;
	   while (song_ended <= 3) {
		   song_ended++;
	   }
	   if (!HoldSong) CurrSong=PrevSong;
	   if (UsingAlt) RecastAlt=true;
	   CastDue = GetTime() + 5;
	   SongNextCast[CurrSong-1] = -1;
	   return 0;
   }

   if (!strcmp(Line,"You can't cast spells while stunned!") ) {
      DebugSpew("MQ2Twist::OnIncomingChat - Song Interrupt Event (stun)");
      if (!HoldSong) CurrSong=PrevSong;
     if (altTwist) CurrSong=LastAltSong;
      CastDue = GetTime() + 10;
      // Wait one second before trying again, to avoid spamming the trigger text w/ cast attempts
      return 0;
   }
   return 0;
}

PLUGIN_API VOID SetGameState(DWORD GameState)
{
   DebugSpew("MQ2Twist::SetGameState()");
   if (GameState==GAMESTATE_INGAME)
   {
      MQ2TwistEnabled = true;
      Load_MQ2Twist_INI();
   } else {
      MQ2TwistEnabled = false;
   }
} 