// MQ2EQBC.cpp - Much of the code here came from MQ2Irc and MQ2Telnet.
// Author: Omnictrl (o)
// Contributor: ascii (a)
// Contributor: Vladus2000 (v)
// Contributor: pms (p)
// Contributor: ieatacid (i)
/*
* Version 1.0.o2 - 20050926
* - Now recognizes when server is shut down.
*
* Version 1.0.o3 - 20050926
* - Now attempts to close socket when plugin is unloaded.
*
* Version 1.0.o4 - 20050926
* - Added commands: togglecompatmode togglereconnect setreconnectsecs stopreconnect
*
* Version 1.0.o5 - 20050926
* - Handles zoning better.
*
* Version 1.0.o6 - 20050926
* Added relog command, somewhat experimental.
*
* Version 1.0.o7 - 20051006
* Added CMD Support (Ping/Pong implemented)
* Added Window support. With CommandHistory (shift up/shift down)
* Added TLO. ${EQBC.Connected}
* Added beta Netbot Support.
* Tested, but some things may have stability issues. Let me know.
* Requires eqbcs v1.0.3 or greater
*
* Version 1.0.o8 - 20051009
* Chat window locations now save/load by character.
* Requires eqbcs v1.0.3 or greater
*
* Version 1.0.o9 - 20051014
* Added color support [+c+], where c can be one of "yogbrtbmpwx"
* These are the same as the "\ac" codes for WriteChatColor.
* If the color char is uppercase, then dark version is used.
* Example: /bc Slow [+g+]Successful[+x+].
* Example: /bc Slow [+r+]Failed[+x+], retrying.
* Added new command to display colors, /bccmd colordump
* Added (untested) password ability. Requires recompile of eqbcs with
* login info redefined, see source comments both here and there.
*
* Version 1.0.o10 - 20051017
* Added code to fix problem with high input (ie. 6 Netbots at 125ms) causing
* TCP backups, eventual buffer overruns, and horrid lags on eqbcs.
* Warning: High amounts of traffic can lag you. ;-P
*
* Version 1.0.o11 - 20051021
* Fix for fun bug that set name as the name of the mount when
* /bccmd connect used after login.
*
* Version 1.0.o12 - 20051021
* In last fix, I forgot to change the char spawn pointer
* in DoCommand. Fixed now.
*
* Version 1.0.o13 - 20050123
* Fixed crash bug reported by LrdDread.
*
* Version 1.0.a14 - 20060423
* Added tell command /bct
*
* Version 1.0.a15 - 20060520
* In IRC compatibility mode /bct will display msg being sent
*
* Version 1.0.a16 - 20060701
* Added patch supplied by Sorcerer for updated Netbots support
*
* Version 1.0.a17 - 20060715
* - Added support for pseudo-channels.  A /bct to channel goes to everyone in
*   the channel.
* - New command: /bccmd channels channel_list
* - Added ability to escape characters \<char> will be translated to just <char>
*
* Version 1.1.a1 - 20060728
* - Changed version numbering system.  Hopefully Omni will change it back.
*   It's up to you to decide if the 'a' is "ascii" or "alpha" :)
* - Channel list saved in INI file and restored when you log in.
* - New command: /bccmd togglelocalecho.  When Local Echo is on, commands sent
*   to a channel you are in will be sent back to you (as per toomanynames)
*
* Version 1.2.v1 - 20070116
* - Added a hook plugins can implement named OnWriteBC.  I wanted to be able to
*   block certain messages from appearing, and this was the only way I knew how.
* - Added /bcaa to send a message to all including yourself.  /bcaa //taskquit
*   is an example of where this is useful
*
* Version 1.2.v2 - 20070324
* - Added better connection timeout handling
*
* Version 1.2.a3 - 20070617 (I'm still not playing release)
* - EQBC window will not close when escape key is pressed.
*
* Version 1.2.p4 - 20070712
* - Added /bccmd toggletellwatch to pass tells received along, credit for string logic to MQ2MasterMind
* - Updated HandleHelpRequest for /bca, /bcaa, and /bccmd toggletellwatch
*
* Version 1.2.a5 - 20080805 (Why did I start playing again release)
* - Fix for changes to EQUIStructs::_CSIDLWND structure
*
* Version 1.2.p6 - 20080909
* - Changed toggletellwatch to check against tell color instead of parsing lines
*   to prevent banker and trader spam
* - Fixed indenting. All indenting uses tabs now.
* - Slightly changed bca and bcaa function names for identification
*
* Version 1.2.p7 - 20081026
* - Added Fades, Alpha, and FadeToAlpha to Window INI Settings
* - /bcaa will now perform the command even if togglecontrol is disabled
* - Code cleanup/better handling for toggletellwatch
* - Added 'togglesilentcmd', which silences the 'CMD: [command]' message
* - Added INI support for silentcmd and tellwatch
*
* Version 1.3.p1 - 20081207
* - Ripped MQ2ChatWnd code to allow for '/bcfont' & '/bcmin' as well as
*   more INI file settings relating to the UI window
* - Moved UI data from [Settings] to [Window] as it is plentiful now
* - Added SaveByCharacter option to the INI to allow saving char UI data
*   to be optional, always using [Window] options instead
* - Added '/bccmd togglesavebychar' to allow toggle this setting.
* - Change UseMyTitle to 1 if you wish to use a custom window title instead of server IP
*   You need to create "WindowTitle=Custom Title" in [Window] and/or [CharName] to use it
*   You do not need to put the title in quotes, and fearless might flame you if you admit to doing it
* - Updated UI window to support '/bca', '/bcaa', and '/bct' command input
*
* Version 1.3.p2 - 20081208
* - '/bcfont' and '/bcmin' are now useable in the UI window ><
*
* Version 1.3.p3 - 20081214
* - Fix for vc6 by dkaa
*
* Version 1.3.i4 - 20081215
* - Added support for EQBC Interface by ieatacid
*
* Version 1.3.p5 - 20081219
* - Added toggleguildwatch, togglegroupwatch, and togglefswatch as
*   additions to the toggletellwatch idea
*/

#define PROG_VERSION "MQ2EQBC v1.3.p5"

#pragma comment(lib,"wsock32.lib")
#include "../MQ2Plugin.h"

#include <vector>

//Tim: 9/22/10
#include <sstream>
//End Tim

PreSetup("MQ2Eqbc");

// Shared Info
class EQBCType : public MQ2Type
{
public:
	enum VarMembers { Connected=1 };
	EQBCType();
	~EQBCType();
	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest);
	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination);
	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source);
	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source);
};

class EQBCType *pEQBCType=0;

CRITICAL_SECTION ConnectCS;
bool bConnecting=false;
bool bTriedConnect=false;
bool bConnected=false;
SOCKADDR_IN serverInfo;

CHAR szServer[MAX_STRING] = {0};
CHAR szPort[MAX_STRING] = {0};
CHAR szToonName[MAX_STRING] = {0};
CHAR szToonCmdStart[MAX_STRING] = {0};
CHAR szToonToRelog[MAX_STRING] = {0};
CHAR szColorChars[] = "yogurtbmpwx";

// Don't change MAX_PASSWORD without checking out the cmd buffer in eqbcs
#define MAX_PASSWORD 40
CHAR szPassword[MAX_PASSWORD] = {0};

#define SUPPORTED_COMMANDS "connect quit help status names colordump reconnect togglecontrol toggleautoconnect togglecompatmode togglereconnect togglewindow setreconnectsecs stopreconnect relog version channels togglelocalecho toggletellwatch toggleguildwatch togglegroupwatch togglefswatch togglesilentcmd togglesavebychar"
#define COLOR_NAME "\ay"
#define COLOR_NAME_BRACKET "\ar"
#define COLOR_OFF "\ax"

#define COLOR_STELL1 "\ax\ar[(\ax\aymsg\ax\ar)\ax\ay"
#define COLOR_STELL2 "\ax\ar]\ax "

#define CONNECT_START "LOGIN"
#define CONNECT_START2 "="
#define CONNECT_END ";"
#define CONNECT_PWSEP ":"
#define SEND_LINE_TERM "\n"

#define CMD_DISCONNECT "\tDISCONNECT\n"
#define CMD_NAMES "\tNAMES\n"
#define CMD_PONG "\tPONG\n"
#define CMD_MSGALL "\tMSGALL\n"
#define CMD_TELL "\tTELL\n"
#define CMD_CHANNELS "\tCHANNELS\n"
#define CMD_LOCALECHO "\tLOCALECHO "
#define CMD_BCI "\tBCI\n"

//Tim: 9/22/10
#define CMD_GVAR "\tGVAR\n"

void HandleGlobalVar(std::string line);
//End Tim

#define MAX_READBUF 512
#define MAX_COMMAND_HISTORY 50

WORD sockVersion;
WSADATA wsaData;
int nret;
LPHOSTENT hostEntry;
SOCKET theSocket;
CHAR *ireadbuf = new CHAR[MAX_READBUF];
int lastReadBufPos;

bool bForceCmd = false;
int bSetTitle = 0;
int bSaveCharacter = 1;
int bUseWindow=0;
int bAutoConnect = 0;
int bAllowControl = 1;
int bSocketGone = 0;
int bIrcCompatMode = 0;
int bReconnectMode = 0;
int bLocalEchoMode = 1;
int bDoTellWatch = 0;
int bDoGuildWatch = 0;
int bDoGroupWatch = 0;
int bDoFSWatch = 0;
int bSilentCmd = 0;
int iReconnectSeconds = 0;
unsigned long lastReconnectTimeSecs = 0;
unsigned long reloginBeforeSecs = 0;
clock_t lastPingTime = 0;

// ------------------------

typedef VOID (__cdecl *fNetBotOnMsg)(PCHAR, PCHAR);
typedef VOID (__cdecl *fNetBotOnEvent)(PCHAR);

void SendNetBotMsg(PCHAR szMess);
void SendNetBotEvent(PCHAR szMess);
void WriteOut(char *szText);
void transmit(bool handleDisconnect, PCHAR szMsg);
void HandleIncomingString(PCHAR rawmsg);
VOID BoxChatCommand(PSPAWNINFO pChar, PCHAR szLine);
VOID BoxChatTell(PSPAWNINFO pChar, PCHAR szLine);
VOID BoxChatAll(PSPAWNINFO pChar, PCHAR szLine);
VOID BoxChatAllButMe(PSPAWNINFO pChar, PCHAR szLine);
VOID EQBCMin(PSPAWNINFO pChar, PCHAR Line);
VOID EQBCFont(PSPAWNINFO pChar, PCHAR Line);

// Dedicated Window
class CMQ2EqbcWnd;
CMQ2EqbcWnd *chatWnd=0;

class CMQ2EqbcWnd : public CCustomWnd
{
private:
	vector<string> CommandHistory;
	int curCommand;
public:
	CTextEntryWnd *InputBox;
	CStmlWnd *StmlOut;
	CXWnd *OutWnd;
	struct _CSIDLWND *OutStruct;
public:
	// ---------- CMQ2EqbcWnd Methods
	CMQ2EqbcWnd(CXStr *Template):CCustomWnd(Template)
	{
		SetWndNotification(CMQ2EqbcWnd);
		StmlOut = (CStmlWnd *)GetChildItem("CWChatOutput");
		OutWnd = (CXWnd*)StmlOut;
		OutWnd->Clickable = 1;
		OutStruct = (_CSIDLWND *)GetChildItem("CWChatOutput");

		InputBox=(CTextEntryWnd*)GetChildItem("CWChatInput");
		InputBox->WindowStyle|=0x800C0;
		InputBox->UnknownCW|=0xFFFFFFFF;
		InputBox->SetMaxChars(512);
		BitOff(WindowStyle,CWS_CLOSE);
		CloseOnESC=0;

		curCommand = -1;
	}
	~CMQ2EqbcWnd()
	{
	};

	int WndNotification(CXWnd *pWnd, unsigned int Message, void *data)
	{
		static char szCommand[] = "/bccmd";
		//static char szAll[] = "/bcall";
		static char szBCT[] = "/bct";
		static char szBCA[] = "/bca";
		static char szBCAA[] = "/bcaa "; // trailing space intentional
		static char szBCMIN[] = "/bcmin";
		static char szBCFONT[] = "/bcfont";

		if (pWnd==(CXWnd*)InputBox)
		{
			if (Message==XWM_HITENTER)
			{
				CHAR szBuffer[513];
				GetCXStr((PCXSTR)InputBox->InputText,szBuffer,512);
				if (szBuffer[0])
				{
					// Command buffer.
					if (CommandHistory.size() == 0 || CommandHistory.front().compare(szBuffer) != 0)
					{
						if (CommandHistory.size() > MAX_COMMAND_HISTORY)
						{
							CommandHistory.pop_back();
						}
						CommandHistory.insert(CommandHistory.begin(), string(szBuffer));
						curCommand=-1;
					}

					// Check for commands.
					if (strnicmp(szBuffer, szCommand, strlen(szCommand)) == 0)
					{
						BoxChatCommand((PSPAWNINFO)pLocalPlayer, szBuffer+strlen(szCommand));
					}
					else if (strnicmp(szBuffer, szBCT, strlen(szBCT)) == 0)
					{
						BoxChatTell((PSPAWNINFO)pLocalPlayer, szBuffer+strlen(szBCT));
					}
					else if (strnicmp(szBuffer, szBCAA, strlen(szBCAA)) == 0)
					{
						BoxChatAll((PSPAWNINFO)pLocalPlayer, szBuffer+strlen(szBCAA));
					}
					else if (strnicmp(szBuffer, szBCA, strlen(szBCA)) == 0)
					{
						BoxChatAllButMe((PSPAWNINFO)pLocalPlayer, szBuffer+strlen(szBCA));
					}
					else if (strnicmp(szBuffer, szBCMIN, strlen(szBCMIN)) == 0)
					{
						EQBCMin((PSPAWNINFO)pLocalPlayer, szBuffer+strlen(szBCMIN));
					}
					else if (strnicmp(szBuffer, szBCFONT, strlen(szBCFONT)) == 0)
					{
						EQBCFont((PSPAWNINFO)pLocalPlayer, szBuffer+strlen(szBCFONT));
					}
					else
					{
						if(bConnected == false)
						{
							WriteOut("\ar#\a-w You are not connected.");
						}
						else
						{
							transmit(true, szBuffer);
						}
					}

					SetCXStr(&InputBox->InputText,"");
				}

				((CXWnd*)InputBox)->ClrFocus();
			}
			else if (Message == 0x16)
			{
				if (data)
				{
					int *pInt = (int *)data;
					int keyPress = pInt[1];
					//DebugSpew("Data: %Xx:%d", pInt[1], pInt[1]);

					if (keyPress == 200) // KeyUp: 0xC8
					{
						if (CommandHistory.size() > 0)
						{
							curCommand++;
							if (curCommand < ((int)CommandHistory.size()) && curCommand >= 0)
							{
								DebugSpew("Up: %d", curCommand);
								string s = (string)CommandHistory.at(curCommand);
								SetCXStr(&InputBox->InputText, (char *)s.c_str());
							}
							else
							{
								// Top
								curCommand = ((int)CommandHistory.size())-1;
							}
						}
					}
					else if (keyPress == 208) // KeyDown: 0xD0
					{
						if (CommandHistory.size() > 0)
						{
							curCommand--;
							if (curCommand >= 0 && CommandHistory.size() > 0)
							{
								string s = (string)CommandHistory.at(curCommand);
								SetCXStr(&InputBox->InputText, (char *)s.c_str());
							}
							else if (curCommand < 0)
							{
								curCommand = -1;
								// Hit bottom.
								SetCXStr(&InputBox->InputText, "");
							}
						}
					}
				}
			}
			else
			{
				//DebugSpew("InputBox message %Xh, value: %Xh",Message,data);
			}
		}
		else if (pWnd==0)
		{
			if (Message==XWM_CLOSE)
			{
				Show=1;
				return 0;
			}
			else
			{
				DebugSpew("CMQ2EqbcWnd message %Xh, value: %Xh",Message,data);
			}
		}
		else
		{
			// DebugSpew("Wnd: 0x%X, Msg: 0x%X, value: %Xh",pWnd,Message,data);
		}
		return CSidlScreenWnd::WndNotification(pWnd,Message,data);
	};

	void SetFontSize(unsigned int size)
	{
		struct FONTDATA
		{
			DWORD NumFonts;
			PCHAR* Fonts; 
		};
		FONTDATA* Fonts;    // font array structure
		CXStr* str;         // contents of stml window
		DWORD* SelFont;     // selected font
		Fonts = (FONTDATA*)&(((char*)pWndMgr)[0xF4]);

		if (size < 0 || size >= (int) Fonts->NumFonts)
		{
			return;
		}
		if (Fonts->Fonts == NULL || chatWnd == NULL)
		{
			return;
		}

		SelFont = (DWORD*)Fonts->Fonts[size];
		((CStmlWnd*)chatWnd->OutWnd)->GetSTMLText(str);
		((CXWnd*)chatWnd->OutWnd)->SetFont(SelFont);
		((CStmlWnd*)chatWnd->OutWnd)->SetSTMLText(*str, 1, 0);
		((CStmlWnd*)chatWnd->OutWnd)->ForceParseNow();
		DebugTry(((CXWnd*)chatWnd->OutWnd)->SetVScrollPos(chatWnd->OutWnd->VScrollMax));
		chatWnd->FontSize = size;
	};

	DWORD FontSize;
};
// ------------------------

BOOL dataEQBC(PCHAR Index, MQ2TYPEVAR &Dest)
{
	Dest.DWord=1;
	Dest.Type=pEQBCType;
	return true;
}

// ------------------------

DWORD WINAPI EQBCConnectThread(LPVOID lpParam)
{
	EnterCriticalSection(&ConnectCS);
	bConnecting=true;
	nret = connect(theSocket, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));

	if (nret == SOCKET_ERROR)
	{
		bConnected=false;
	}
	else
	{
		// DebugSpew("MQ2Eqbc Connecting");
		unsigned long nonblocking = 1;
		ioctlsocket(theSocket, FIONBIO, &nonblocking);
		Sleep((clock_t)4 * CLOCKS_PER_SEC/2);

		send(theSocket, CONNECT_START, strlen(CONNECT_START), 0);
		if (*szPassword)
		{
			// DebugSpew("With Password");
			send(theSocket, CONNECT_PWSEP, strlen(CONNECT_PWSEP), 0);
			send(theSocket, szPassword, strlen(szPassword), 0);
		}
		send(theSocket, CONNECT_START2, strlen(CONNECT_START2), 0);
		send(theSocket, szToonName, strlen(szToonName), 0);
		send(theSocket, CONNECT_END, strlen(CONNECT_END), 0);
		// DebugSpew("MQ2Eqbc Connected");

		bConnected=true;
	}

	bTriedConnect=true;
	bConnecting=false;
	LeaveCriticalSection(&ConnectCS);
	return 0;
}

void CheckSocket(char *szFunc, int err)
{
	int werr = WSAGetLastError();
	if (werr == WSAECONNABORTED)
	{
		bSocketGone = true;   
	}
	WSASetLastError(0);
	// DebugSpewAlways("Sock Error-%s: %d / w%d", szFunc, err, werr);
}

char *getCurPlayerName()
{
	if (gGameState != GAMESTATE_INGAME) return NULL;
	PCHARINFO pChar = GetCharInfo();
	char *name = (pChar && pChar->Name) ? pChar->Name : NULL;
	return name;
}

VOID SetPlayer()
{
	char *pszName = getCurPlayerName();
	strcpy(szToonName, (pszName) ? pszName : "YouPlayer");
	sprintf(szToonCmdStart, "%s //", szToonName);
}

void CreateChatWindowIfNeeded()
{
	if (chatWnd == NULL && bUseWindow && gGameState == GAMESTATE_INGAME)
	{
		char szWindowText[MAX_STRING] = {0};
		sprintf(szWindowText, "%s", szServer);

		// Left for compatibility with previous versions. But is now de facto default.
		int chatWndTop = GetPrivateProfileInt("Window", "ChatTop", 10, INIFileName);
		int chatWndBot = GetPrivateProfileInt("Window", "ChatBottom", 210, INIFileName);
		int chatWndLeft = GetPrivateProfileInt("Window", "ChatLeft", 10, INIFileName);
		int chatWndRight = GetPrivateProfileInt("Window", "ChatRight", 410, INIFileName);
		int chatWndFades = GetPrivateProfileInt("Window", "Fades", 0, INIFileName);
		int chatWndAlpha = GetPrivateProfileInt("Window", "Alpha", 255, INIFileName);
		int chatWndFadeToAlpha = GetPrivateProfileInt("Window", "FadeToAlpha", 255, INIFileName);
		int chatWndFadeDuration = GetPrivateProfileInt("Window", "Duration", 500, INIFileName);
		int chatWndLocked = GetPrivateProfileInt("Window", "Locked", 0, INIFileName);
		int chatWndTimeMouseOver = GetPrivateProfileInt("Window", "Delay", 2000, INIFileName);
		int chatWndBGType = GetPrivateProfileInt("Window", "BGType", 1, INIFileName);
		int chatWndBGColorR = GetPrivateProfileInt("Window", "BGTint.red", 255, INIFileName);
		int chatWndBGColorG = GetPrivateProfileInt("Window", "BGTint.green", 255, INIFileName);
		int chatWndBGColorB = GetPrivateProfileInt("Window", "BGTint.blue", 255, INIFileName);
		unsigned int chatWndFontSize = GetPrivateProfileInt("Window", "FontSize", 4, INIFileName);
		bSetTitle = GetPrivateProfileInt("Window", "UseMyTitle", 0, INIFileName);
		if (bSetTitle)
		{
			GetPrivateProfileString("Window", "WindowTitle", szWindowText, szWindowText, MAX_STRING, INIFileName);
		}

		if (bSaveCharacter)
		{
			char *playerName = (getCurPlayerName()) ? getCurPlayerName() : "Settings";

			chatWndTop = GetPrivateProfileInt(playerName, "ChatTop", chatWndTop, INIFileName);
			chatWndBot = GetPrivateProfileInt(playerName, "ChatBottom", chatWndBot, INIFileName);
			chatWndLeft = GetPrivateProfileInt(playerName, "ChatLeft", chatWndLeft, INIFileName);
			chatWndRight = GetPrivateProfileInt(playerName, "ChatRight", chatWndRight, INIFileName);
			chatWndFades = GetPrivateProfileInt(playerName, "Fades", chatWndFades, INIFileName);
			chatWndAlpha = GetPrivateProfileInt(playerName, "Alpha", chatWndAlpha, INIFileName);
			chatWndFadeToAlpha = GetPrivateProfileInt(playerName, "Fades", chatWndFadeToAlpha, INIFileName);
			chatWndFadeDuration = GetPrivateProfileInt(playerName, "Duration", chatWndFadeDuration, INIFileName);
			chatWndLocked = GetPrivateProfileInt(playerName, "Locked", chatWndLocked, INIFileName);
			chatWndTimeMouseOver = GetPrivateProfileInt(playerName, "Delay", chatWndTimeMouseOver, INIFileName);
			chatWndBGType = GetPrivateProfileInt(playerName, "BGType", chatWndBGType, INIFileName);
			chatWndBGColorR = GetPrivateProfileInt(playerName, "BGTint.red", chatWndBGColorR, INIFileName);
			chatWndBGColorG = GetPrivateProfileInt(playerName, "BGTint.green", chatWndBGColorG, INIFileName);
			chatWndBGColorB = GetPrivateProfileInt(playerName, "BGTint.blue", chatWndBGColorB, INIFileName);
			chatWndFontSize = GetPrivateProfileInt(playerName, "FontSize", chatWndFontSize, INIFileName);
			bSetTitle = GetPrivateProfileInt(playerName, "UseMyTitle", 0, INIFileName);
			if (bSetTitle)
			{
				GetPrivateProfileString(playerName, "WindowTitle", szWindowText, szWindowText, MAX_STRING, INIFileName);
			}
		}

		class CXStr ChatWnd("ChatWindow");
		chatWnd= new CMQ2EqbcWnd(&ChatWnd);
		chatWnd->Location.top = chatWndTop;
		chatWnd->Location.bottom = chatWndBot;
		chatWnd->Location.left = chatWndLeft;
		chatWnd->Location.right = chatWndRight;
		chatWnd->Fades = chatWndFades;
		chatWnd->Alpha = chatWndAlpha;
		chatWnd->FadeToAlpha = chatWndFadeToAlpha;
		chatWnd->FadeDuration = chatWndFadeDuration;
		chatWnd->Locked = chatWndLocked;
		chatWnd->TimeMouseOver = chatWndTimeMouseOver;
		chatWnd->BGType = chatWndBGType;
		chatWnd->BGColor.R = chatWndBGColorR;
		chatWnd->BGColor.G = chatWndBGColorG;
		chatWnd->BGColor.B = chatWndBGColorB;
		chatWnd->SetFontSize(chatWndFontSize);
		SetCXStr(&chatWnd->WindowText, szWindowText);
		DebugTry(((CXWnd*)chatWnd)->Show(1, 1));
		DebugTry(BitOff(chatWnd->OutStruct->WindowStyle, CWS_CLOSE));
	}
}

void closeChatWindow(void)
{
	CHAR szTemp[20] = {0};

	if (chatWnd)
	{
		int chatWndTop = chatWnd->Location.top;
		int chatWndBot = chatWnd->Location.bottom;
		int chatWndLeft = chatWnd->Location.left;
		int chatWndRight = chatWnd->Location.right;
		int chatWndFades = chatWnd->Fades;
		int chatWndAlpha = chatWnd->Alpha;
		int chatWndFadeToAlpha = chatWnd->FadeToAlpha;
		int chatWndFadeDuration = chatWnd->FadeDuration;
		int chatWndLocked = chatWnd->Locked;
		int chatWndTimeMouseOver = chatWnd->TimeMouseOver;
		int chatWndBGType = chatWnd->BGType;
		int chatWndBGColorR = chatWnd->BGColor.R;
		int chatWndBGColorG = chatWnd->BGColor.G;
		int chatWndBGColorB = chatWnd->BGColor.B;
		unsigned int chatWndFontSize = chatWnd->FontSize;

		// Will be default for next new character
		WritePrivateProfileString("Window", "ChatTop", itoa(chatWndTop, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "ChatBottom", itoa(chatWndBot, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "ChatLeft", itoa(chatWndLeft, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "ChatRight", itoa(chatWndRight, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "Fades", itoa(chatWndFades, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "Alpha", itoa(chatWndAlpha, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "FadeToAlpha", itoa(chatWndFadeToAlpha, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "Duration", itoa(chatWndFadeDuration, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "Locked", itoa(chatWndLocked, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "Delay", itoa(chatWndTimeMouseOver, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "BGType", itoa(chatWndBGType, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "BGTint.red", itoa(chatWndBGColorR, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "BGTint.green", itoa(chatWndBGColorG, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "BGTint.blue", itoa(chatWndBGColorB, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "FontSize", itoa(chatWndFontSize, szTemp, 10), INIFileName);
		WritePrivateProfileString("Window", "UseMyTitle", bSetTitle ? "1" : "0", INIFileName);

		if (bSaveCharacter)
		{
			char *playerName = (getCurPlayerName()) ? getCurPlayerName() : "Settings";

			if (*playerName)
			{
				WritePrivateProfileString(playerName, "ChatTop", itoa(chatWndTop, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "ChatBottom", itoa(chatWndBot, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "ChatLeft", itoa(chatWndLeft, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "ChatRight", itoa(chatWndRight, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "Fades", itoa(chatWndFades, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "Alpha", itoa(chatWndAlpha, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "FadeToAlpha", itoa(chatWndFadeToAlpha, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "Duration", itoa(chatWndFadeDuration, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "Locked", itoa(chatWndLocked, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "Delay", itoa(chatWndTimeMouseOver, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "BGType", itoa(chatWndBGType, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "BGTint.red", itoa(chatWndBGColorR, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "BGTint.green", itoa(chatWndBGColorG, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "BGTint.blue", itoa(chatWndBGColorB, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "FontSize", itoa(chatWndFontSize, szTemp, 10), INIFileName);
				WritePrivateProfileString(playerName, "UseMyTitle", bSetTitle ? "1" : "0", INIFileName);
			}
		}

		delete chatWnd;
		chatWnd=0;
	}
}

void WriteOut(char *szText)
{
	typedef BOOL (__cdecl *fMQWriteBC)(char *szText);

	BOOL bWrite = true;
	PMQPLUGIN pPlugin = pPlugins;
	while(pPlugin)
	{
		fMQWriteBC WriteBC = (fMQWriteBC)GetProcAddress(pPlugin->hModule, "OnWriteBC");
		if (WriteBC)
		{
			if (!WriteBC(szText)) bWrite = false;
		}
		pPlugin = pPlugin->pNext;
	}

	if (!bWrite) return;

	// DebugSpew("WriteOut [%s]", szText);
	if (chatWnd && bUseWindow)
	{
		DebugTry(((CXWnd*)chatWnd)->Show(1, 1));
		char szProcessed[MAX_STRING];
		StripMQChat(szText, szProcessed);
		CheckChatForEvent(szProcessed);
		MQToSTML(szText, szProcessed,MAX_STRING);
		strcat(szProcessed,"<br>");
		CXStr NewText(szProcessed);
		CXSize Whatever;
		(chatWnd->StmlOut)->AppendSTML(&Whatever,NewText);
		(chatWnd->OutWnd)->SetVScrollPos(chatWnd->OutStruct->VScrollMax);
	}
	else
	{
		WriteChatColor(szText);
	}

	return;
}

int WriteStringGetCount(CHAR *dest, CHAR *src)
{
	int i=0;

	for(; dest && src && *src; src++)
	{
		dest[i++] = *src;
	}

	return i;
}

void transmit(bool handleDisconnect, PCHAR szMsg)
{
	if (bConnected)
	{
		int err = send(theSocket, szMsg, strlen(szMsg), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			if (handleDisconnect) CheckSocket("broadcast:send1", err);
		}

		err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			if (handleDisconnect) CheckSocket("broadcast:send2", err);
		}
	}
}

void echoTransmit(bool handleDisconnect, PCHAR szMsg)
{
	CHAR szTemp[MAX_STRING];
	sprintf(szTemp, "\ar#\ax %s", szMsg);
	WriteOut(szTemp);
	transmit(handleDisconnect, szMsg);
}

VOID HandleToggleRequest(int *piFlag, char *szProfName, char *szFlagDesc)
{
	char szTemp[MAX_STRING];
	*piFlag = (*piFlag) ? 0 : 1;
	sprintf(szTemp,"\ar#\ax Setting: %s turned %s", szFlagDesc, (*piFlag) ? "ON" : "OFF");
	WriteOut(szTemp);
	sprintf(szTemp, "%d", (int)(*piFlag));
	WritePrivateProfileString("Settings", szProfName, szTemp, INIFileName);
}

VOID HandleStatusRequest()
{
	CHAR szTemp[MAX_STRING];

	if (bConnected)
	{
		sprintf(szTemp,"\ar#\ax MQ2Eqbc Status: ONLINE - %s - %s", szServer, szPort);
		WriteOut(szTemp);
	}
	else
	{
		WriteOut("\ar#\ax MQ2Eqbc Status: OFFLINE");
	}
	sprintf(szTemp,"\ar#\ax Allow Control: %s, Auto Connect: %s", (bAllowControl) ? "ON" : "OFF", (bAutoConnect) ? "ON" : "OFF");
	WriteOut(szTemp);
	sprintf(szTemp,"\ar#\ax IRC Compat Mode: %s, Reconnect: %s: (every %d secs)", (bIrcCompatMode) ? "ON" : "OFF", (bReconnectMode) ? "ON" : "OFF", iReconnectSeconds);
	WriteOut(szTemp);
}

void HandleHelpRequest()
{
	HandleStatusRequest();
	WriteOut("\ar#\ax Commands Available");
	WriteOut("\ar#\ax \ay/bccmd connect <server> <port> <pw>\ax (defaults: 127.0.0.1 2112)");
	WriteOut("\ar#\ax \ay/bccmd quit\ax (disconnects)");
	WriteOut("\ar#\ax \ay/bccmd help\ax (show this screen)");
	WriteOut("\ar#\ax \ay/bccmd status\ax (show connected or not and settings)");
	WriteOut("\ar#\ax \ay/bccmd reconnect\ax (close current connection and connect again)");
	WriteOut("\ar#\ax \ay/bccmd names\ax (show who is connected)");
	WriteOut("\ar#\ax \ay/bccmd colordump\ax (Shows all available color codes)");
	WriteOut("\ar#\ax \ay/bccmd togglecontrol\ax (allow remote control)");
	WriteOut("\ar#\ax \ay/bccmd togglewindow\ax (toggles use of dedicated window)");
	WriteOut("\ar#\ax \ay/bccmd togglecompatmode\ax (toggle IRC Compatability mode)");
	WriteOut("\ar#\ax \ay/bccmd toggleautoconnect\ax (toggle auto connect)");
	WriteOut("\ar#\ax \ay/bccmd togglereconnect\ax (toggle auto-reconnect mode on server disconnect)");
	WriteOut("\ar#\ax \ay/bccmd toggletellwatch\ax (toggle relay of tells received to /bc)");
	WriteOut("\ar#\ax \ay/bccmd toggleguildwatch\ax (toggle relay of guild chat to /bc)");
	WriteOut("\ar#\ax \ay/bccmd togglegroupwatch\ax (toggle relay of group chat to /bc)");
	WriteOut("\ar#\ax \ay/bccmd togglefswatch\ax (toggle relay of fellowship chat to /bc)");
	WriteOut("\ar#\ax \ay/bccmd togglesilentcmd\ax (toggle display of 'CMD: [command]' echo)");
	WriteOut("\ar#\ax \ay/bccmd togglesavebychar\ax (toggle saving UI data to [CharName] in INI)");
	WriteOut("\ar#\ax \ay/bccmd setreconnectsecs n\ax (n is seconds to reconnect: default 15)");
	WriteOut("\ar#\ax \ay/bccmd stopreconnect\ax (stop trying to reconnect for now)");
	WriteOut("\ar#\ax \ay/bccmd relog <charname>\ax (relog in as charname if you camp < 60 seconds): No charname resets");
	WriteOut("\ar#\ax \ay/bccmd channels <channel list>\ax (set list of channels to receive tells from)");
	WriteOut("\ar#\ax \ay/bccmd togglelocalecho\ax (toggle echoing my commands back to me if I am in channel)");
	WriteOut("\ar#\ax \ay/bc your text\ax (send text)");
	WriteOut("\ar#\ax \ay/bct ToonName your text\ax (send your text to specific Toon)");
	WriteOut("\ar#\ax \ay/bct ToonName //command\ax (send Command to ToonName)");
	WriteOut("\ar#\ax \ay/bc ToonName //command\ax (send Command to ToonName - DEPRECATED)");
	WriteOut("\ar#\ax \ay/bca //command\ax (send Command to all connected names EXCLUDING yourself)");
	WriteOut("\ar#\ax \ay/bcaa //command\ax (send Command to all connected names INCLUDING yourself)");
}

void HandleConnectRequest(PCHAR szLine)
{
	CHAR szMsg[MAX_STRING];
	CHAR szIniServer[MAX_STRING] = {0};
	CHAR szIniPort[MAX_STRING] = {0};
	CHAR szIniPassword[MAX_PASSWORD] = {0};

	if (bConnected)
	{
		WriteOut("\ar#\ax Already connected. Use /bccmd quit to disconnect first.");
		return;
	}

	if (bConnecting)
	{
		WriteOut("\ar#\ax Already trying to connect! Hold on a minute there");
		return;
	}

	SetPlayer();

	CHAR szArg1[MAX_STRING] = {0};
	CHAR szArg2[MAX_STRING] = {0};
	CHAR szArg3[MAX_STRING] = {0};

	GetArg(szArg1, szLine, 2);  // 1 was the connect statement.
	GetArg(szArg2, szLine, 3);
	GetArg(szArg3, szLine, 4);

	GetPrivateProfileString("Last Connect", "Server", "127.0.0.1", szIniServer, MAX_STRING, INIFileName);
	GetPrivateProfileString("Last Connect", "Port", "2112", szIniPort, MAX_STRING, INIFileName);
	GetPrivateProfileString("Last Connect", "Password","", szIniPassword, MAX_STRING, INIFileName);

	strcpy(szServer, (*szArg1) ? szArg1 : szIniServer);
	strcpy(szPort, (*szArg2) ? szArg2 : szIniPort);
	strcpy(szPassword, (*szArg3) ? szArg3 : szIniPassword);

	sockVersion = MAKEWORD(1, 1);
	WSAStartup(sockVersion, &wsaData);
	hostEntry = gethostbyname(szServer);
	if (!hostEntry)
	{
		WriteOut("\ar#\ax gethostbyname error");
		WSACleanup();
		return;
	}

	theSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (theSocket == INVALID_SOCKET) {
		WriteOut("\ar#\ax Socket error");
		WSACleanup();
		return;
	}

	CreateChatWindowIfNeeded();
	// if not using custom title reset displayed IP if changed
	char szWindowText[MAX_STRING] = {0};
	if (chatWnd && !bSetTitle)
	{
		GetCXStr(chatWnd->WindowText, szWindowText);
		if (strnicmp(szWindowText, szServer, sizeof(szWindowText)))
		{
			SetCXStr(&chatWnd->WindowText, szServer);
		}
	}

	sprintf(szMsg, "\ar#\ax Connecting to %s %s...", szServer, szPort);
	WriteOut(szMsg);

	lastPingTime = 0;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	serverInfo.sin_port = htons(atoi(szPort));
	DWORD ThreadId;
	CreateThread(NULL, 0, &EQBCConnectThread, 0, 0, &ThreadId);

	return;
}

VOID HandleDisconnect(bool sendDisconnect)
{
	if (bConnected)
	{
		SendNetBotEvent("NBEXIT");
		bConnected = false;
		// Could set linger off here..
		if (sendDisconnect)
		{
			int err = send(theSocket, CMD_DISCONNECT, sizeof(CMD_DISCONNECT), 0);
			if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
			{
				CheckSocket("HandleDisconnect:Send", err);
			}
		}
		closesocket(theSocket);
		lastReadBufPos = 0;
	}
}

VOID HandleReconnectRequest()
{
	HandleDisconnect(true);
	HandleConnectRequest("");
}

VOID HandleVersionRequest()
{
	CHAR szTemp[MAX_STRING];

	sprintf(szTemp, "\ar#\ax %s", PROG_VERSION);
	WriteOut(szTemp);
}

VOID HandleNamesRequest()
{
	if (bConnected)
	{
		WriteOut("\ar#\ax Requesting names...");
		int err = send(theSocket, CMD_NAMES, strlen(CMD_NAMES), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			CheckSocket("HandleNamesRequest:send", err);
		}
	}
	else
	{
		WriteOut("You are not connected");
	}
}

VOID HandleChannels(PCHAR szLine)
{
	CHAR szTemp[MAX_STRING]={0};
	CHAR szTemp1[MAX_STRING]={0};
	CHAR *szArg;
	CHAR szCommand[] = CMD_CHANNELS;
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bccmd connect\ax to establish a connection.");
		return;
	}

	char *playerName = (getCurPlayerName()) ? getCurPlayerName() : "Settings";
  
	if (!szLine) {
		GetPrivateProfileString(playerName, "Channels", "", szTemp1, MAX_STRING, INIFileName);
		strlwr(szTemp1);
		if (!(szArg=strtok(szTemp1, " \n"))) return;
	} else {
		strlwr(szLine);
		szArg=strtok(szLine, " \n"); // first token will be command CHANNELS, skip it
		szArg=strtok(NULL, " \n");
	}

	while(szArg!=NULL){
		strncat(szTemp, szArg, MAX_STRING-1);
		if ((szArg=strtok(NULL, " \n"))) strcat(szTemp, " ");
	}

	if (*playerName) WritePrivateProfileString(playerName, "Channels", szTemp, INIFileName);

	err = send(theSocket, szCommand, strlen(szCommand), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleChannels:Send1", err);
	}

	err = send(theSocket, szTemp, strlen(szTemp), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleChannels:Send2", err);
	}

	err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleChannels:Send3", err);
	}
}
VOID SendCmdLocalEcho()
{
	CHAR szCommand[15];
	int err;

	sprintf(szCommand, "%s%i\n", CMD_LOCALECHO, bLocalEchoMode);

	err = send(theSocket, szCommand, strlen(szCommand), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("SendCmdLocalEcho:Send1", err);
	}
}

VOID BoxChatCommand(PSPAWNINFO pChar, PCHAR szLine)
{
	static CHAR szCommandConnect[] = "connect";
	static CHAR szCommandDisconnect[] = "quit";
	static CHAR szCommandHelp[] = "help";
	static CHAR szCommandStatus[] = "status";
	static CHAR szCommandReconnect[] = "reconnect";
	static CHAR szCommandNames[] = "names";
	static CHAR szCommandToggleAutoConnect[] = "toggleautoconnect";
	static CHAR szCommandToggleControl[] = "togglecontrol";
	static CHAR szCommandToggleCompatMode[] = "togglecompatmode";
	static CHAR szCommandToggleReconnect[] = "togglereconnect";
	static CHAR szCommandToggleWindow[] = "togglewindow";
	static CHAR szCommandStopReconnect[] = "stopreconnect";
	static CHAR szCommandSetReconnect[] = "setreconnectsecs";
	static CHAR szCommandRelog[] = "relog";
	static CHAR szCommandVersion[] = "version";
	static CHAR szCommandColorDump[] = "colordump";
	static CHAR szCommandChannels[] = "channels";
	static CHAR szCommandToggleLocalEcho[] = "togglelocalecho";
	static CHAR szCommandToggleTellWatch[] = "toggletellwatch";
	static CHAR szCommandToggleGuildWatch[] = "toggleguildwatch";
	static CHAR szCommandToggleGroupWatch[] = "togglegroupwatch";
	static CHAR szCommandToggleFSWatch[] = "togglefswatch";
	static CHAR szCommandToggleSilentCmd[] = "togglesilentcmd";
	static CHAR szCommandToggleSaveByChar[] = "togglesavebychar";

	CHAR szArg[MAX_STRING] = {0};
	CHAR szMsg[MAX_STRING]={0};

	//sprintf(szMsg, "BCCMD: %s", szLine);
	//WriteOut(szMsg);

	GetArg(szArg, szLine, 1);

	if (stricmp(szCommandConnect, szArg) == 0)
	{
		HandleConnectRequest(szLine);
	}
	else if (stricmp(szCommandDisconnect, szArg) == 0)
	{
		HandleDisconnect(true);
		WriteOut("\ar#\ax Connection Closed, you can unload MQ2Eqbc now.");
	}
	else if (stricmp(szCommandStatus, szArg) == 0)
	{
		HandleStatusRequest();
	}
	else if (stricmp(szCommandHelp, szArg) == 0)
	{
		HandleHelpRequest();
	}
	else if (stricmp(szCommandNames, szArg) == 0)
	{
		HandleNamesRequest();
	}
	else if (stricmp(szCommandReconnect, szArg) == 0)
	{
		HandleReconnectRequest();
	}
	else if (stricmp(szCommandToggleControl, szArg) == 0)
	{
		HandleToggleRequest(&bAllowControl, "AllowControl", "Allow Control");
	}
	else if (stricmp(szCommandToggleAutoConnect, szArg) == 0)
	{
		HandleToggleRequest(&bAutoConnect, "AutoConnect", "Auto Connect");
	}
	else if (stricmp(szCommandToggleCompatMode, szArg) == 0)
	{
		HandleToggleRequest(&bIrcCompatMode, "IRCCompatMode", "IRC Compat Mode");
	}
	else if (stricmp(szCommandToggleReconnect, szArg) == 0)
	{
		HandleToggleRequest(&bReconnectMode, "AutoReconnect", "Auto Reconnect (on remote disconnect)");
	}
	else if (stricmp(szCommandToggleWindow, szArg) == 0)
	{
		HandleToggleRequest(&bUseWindow, "UseWindow", "Use Dedicated Window");
		if (bUseWindow) CreateChatWindowIfNeeded();
		else closeChatWindow();
	}
	else if (stricmp(szCommandRelog, szArg) == 0)
	{
		GetArg(szArg, szLine, 2);
		if (reloginBeforeSecs && *szArg == 0)
		{
			sprintf(szMsg, "Aborting relog to [%s], you must cancel /camp.", szToonToRelog);
			echoTransmit(true, szMsg);
			reloginBeforeSecs = 0;
			*szToonToRelog = 0;
		}
		else if (*szArg != 0)
		{
			strcpy(szToonToRelog, szArg);
			reloginBeforeSecs = (GetTickCount() / 1000) + 60;
			sprintf(szMsg, "Now logging in as [%s], %s must /camp and reach Character Selection in 60 seconds.", szToonToRelog, szToonName);
			echoTransmit(true, szMsg);
		}
	}
	else if (stricmp(szCommandStopReconnect, szArg) == 0)
	{
		if (lastReconnectTimeSecs == 0)
		{
			WriteOut("\ar#\ax You are not trying to reconnect");
		}
		else
		{
			WriteOut("\ar#\ax Disabling reconnect mode for now.");
			lastReconnectTimeSecs = 0;
		}
	}
	else if (stricmp(szCommandSetReconnect, szArg) == 0)
	{
		GetArg(szArg, szLine, 2);
		if (*szArg && atoi(szArg) > 0)
		{
			iReconnectSeconds = atoi(szArg);

			sprintf(szMsg, "%d", iReconnectSeconds);
			WritePrivateProfileString("Settings", "ReconnectRetrySeconds", szMsg, INIFileName);

			sprintf(szMsg, "\ar#\ax Will now try to reconnect every %d seconds after server disconnect.", iReconnectSeconds);
		}
		else
		{
			sprintf(szMsg, "\ar#\ax Invalid value given - proper example: /bccmd setreconnectsecs 15");
		}
		WriteOut(szMsg);
	}
	else if (stricmp(szCommandColorDump, szArg) == 0)
	{
		CHAR ch;
		int i;
		strcpy(szMsg, "\ar#\ax Bright Colors:");
		for (i=0; i < (int)strlen(szColorChars)-1; i++)
		{
			ch = szColorChars[i];
			sprintf(szArg, " \a%c[+%c+]", ch, ch);
			strcat(szMsg, szArg);
		}
		WriteOut(szMsg);
		strcpy(szMsg, "\ar#\ax Dark Colors:");
		for (i=0; i < (int)strlen(szColorChars)-1; i++)
		{
			ch = szColorChars[i];
			sprintf(szArg, " \a-%c[+%c+]", ch, toupper(ch));
			strcat(szMsg, szArg);
		}
		WriteOut(szMsg);
		WriteOut("\ar#\ax [+x+] and [+X+] set back to default color.");
	}
	else if (stricmp(szCommandVersion, szArg) == 0)
	{
		sprintf(szMsg, "\ar#\ax %s", PROG_VERSION);
		WriteOut(szMsg);
	}
	else if (stricmp(szCommandChannels, szArg) == 0)
	{
		HandleChannels(szLine);
	}
	else if (stricmp(szCommandToggleLocalEcho, szArg) == 0)
	{
		HandleToggleRequest(&bLocalEchoMode, "LocalEcho", "Echo my channel commands back to me");
		SendCmdLocalEcho();
	}
	else if (stricmp(szCommandToggleTellWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoTellWatch, "TellWatch", "'Relay all tells to /bc'");
	}
	else if (stricmp(szCommandToggleGuildWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoGuildWatch, "GuildWatch", "'Relay guild chat to /bc'");
	}
	else if (stricmp(szCommandToggleGroupWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoGroupWatch, "GroupWatch", "'Relay group chat to /bc'");
	}
	else if (stricmp(szCommandToggleFSWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoFSWatch, "FSWatch", "'Relay fellowship chat to /bc'");
	}
	else if (stricmp(szCommandToggleSilentCmd, szArg) == 0)
	{
		HandleToggleRequest(&bSilentCmd, "SilentCmd", "Silence 'CMD: [command]' echo");
	}
	else if (stricmp(szCommandToggleSaveByChar, szArg) == 0)
	{
		HandleToggleRequest(&bSaveCharacter, "SaveByCharacter", "Save UI data by character name");
	}
	else
	{
		sprintf(szMsg, "\ar#\ax Unsupported command, supported commands are: %s", SUPPORTED_COMMANDS);
		WriteOut(szMsg);
	}
}

// BoxChatSay
VOID BoxChatSay(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szMsg[MAX_STRING]={0};
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bccmd connect\ax to establish a connection.");
		return;
	}

	if (szLine && strlen(szLine))
	{
		err = send(theSocket, szLine, strlen(szLine), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			CheckSocket("BoxChatSay:Send1", err);
		}

		err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			CheckSocket("BoxChatSay:Send2", err);
		}
	}
}

// BoxChatTell
VOID BoxChatTell(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szTemp[MAX_STRING]={0};
	int i_src=0;
	int i_dest=0;
	int i_len;
	CHAR szCommand[] = CMD_TELL;
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bccmd connect\ax to establish a connection.");
		return;
	}

	if (szLine && strlen(szLine)) {
		err = send(theSocket, szCommand, strlen(szCommand), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
			CheckSocket("BoxChatTell:Send1", err);
		}

		err = send(theSocket, szLine, strlen(szLine), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
			CheckSocket("BoxChatTell:Send2", err);
		}

		err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
			CheckSocket("BoxChatTell:Send3", err);
		}

		if (bIrcCompatMode) {
			i_len=strlen(szLine);
			i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_STELL1);
			while (szLine[i_src]!=' ' && szLine[i_src]!='\n' && i_src <= i_len) {
				szTemp[i_dest++]=szLine[i_src++];
			}
			i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_STELL2);
			i_src++;
			while (i_src <= i_len) {
				szTemp[i_dest++]=szLine[i_src++];
			}
			szTemp[i_dest]='\n';
			WriteOut(szTemp);
		}
	}
}

void BciTransmit(char *szLine, char *szCmd)
{
   CHAR szCommand[] = CMD_BCI;
   int err;

   strcat(szLine, " ");
   strcat(szLine, szCmd);

   if (szLine && strlen(szLine))
   {
      err = send(theSocket, szCommand, strlen(szCommand), 0);
      if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
         CheckSocket("BciTransmit:Send1", err);
      }

      err = send(theSocket, szLine, strlen(szLine), 0);
      if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
         CheckSocket("BciTransmit:Send2", err);
      }

      err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
      if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
         CheckSocket("BciTransmit:Send3", err);
      }
   }
}

VOID BoxChatAllButMe(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szCommand[] = CMD_MSGALL;
	CHAR szMsg[MAX_STRING]={0};
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bccmd connect\ax to establish a connection.");
		return;
	}

	if (szLine && strlen(szLine))
	{
		err = send(theSocket, szCommand, strlen(szCommand), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			CheckSocket("BoxChatSayAll:Send1", err);
		}

		err = send(theSocket, szLine, strlen(szLine), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			CheckSocket("BoxChatSayAll:Send2", err);
		}

		err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
		{
			CheckSocket("BoxChatSayAll:Send3", err);
		}
	}
}

VOID BoxChatAll(PSPAWNINFO pChar, PCHAR szLine)
{
	BoxChatAllButMe(pChar, szLine);
	CHAR szTemp[ MAX_STRING ];
	sprintf(szTemp, "<%s> %s %s", pChar->Name, pChar->Name, szLine);
	bForceCmd = true;
	HandleIncomingString(szTemp);
	bForceCmd = false;
}

void SendNetBotMsg(PCHAR szMess)
{
	CHAR *posi = NULL;
	CHAR *name = NULL;
	if (szMess != NULL && *szMess !=0) 
	{
		if (posi = strchr(szMess, ':'))
		{
			if (!strnicmp(posi, ":[NB]|", 6)) name="mq2netbots";
			else if (!strnicmp(posi, ":[NH]|", 6)) name="mq2netheal";
		}
	}
	if (name == NULL || *name == 0)
	{
		DebugSpew("SendNetBotMSG::Bad NBMSG");
		return;
	}

	fNetBotOnMsg sendf = NULL;
	PMQPLUGIN pFind = pPlugins;
	while (pFind && stricmp(pFind->szFilename,name)) pFind = pFind->pNext;
	if (pFind) sendf=(fNetBotOnMsg)GetProcAddress(pFind->hModule, "OnNetBotMSG");
	if (sendf == NULL)
	{
		DebugSpew("SendNetBotMSG::No Handler");
		return;
	}

	*posi = 0;
	sendf(szMess, posi+1);
	*posi = ':';
}

void SendNetBotEvent(PCHAR szMess)
{
	if (szMess == NULL || *szMess ==0)
	{
		DebugSpew("SendNetBotEVENT::Bad NBMSG");
		return;
	}

	fNetBotOnEvent sendf = NULL;
	PMQPLUGIN pFind = pPlugins;
	while (pFind && stricmp(pFind->szFilename, "mq2netbots")) pFind = pFind->pNext;
	if (pFind) sendf=(fNetBotOnEvent)GetProcAddress(pFind->hModule, "OnNetBotEVENT");
	if (sendf == NULL)
	{
		DebugSpew("SendNetBotEVENT::No Handler");
		return;
	}

	sendf(szMess);
}

char getColorCharFromCode(CHAR *test)
{
	// Colors From MQToSTML (already assigned here to szColorChars)
	// 'y'ellow, 'o'range, 'g'reen, bl'u'e, 'r'ed, 't'eal, 'b'lack (none),
	// 'm'agenta, 'p'urple, 'w'hite, 'x'=back to default

	// Color code format: "[+r+]"
	if (	test[0] == '[' &&
		test[1] == '+' &&
		test[2] != '\0' &&
		test[3] == '+' &&
		test[4] == ']')
	{
		if (strchr(szColorChars, (int)tolower(test[2])))
		{
			return test[2];
		}
	}

	return 0;
}


void strCleanEnd(PCHAR pszStr)
{
	// Remove trailing spaces and CR/LF's

	int len;

	if (pszStr && *pszStr)
	{
		for (len = strlen(pszStr)-1;
		len >= 0 && strchr(" \r\n", pszStr[len]);
		pszStr[len--]=0);
	}
}

void HandleIncomingCommand(PCHAR pszCmd)
{
	CHAR szTemp[MAX_STRING] = {0};
	CHAR *szAllowed = (bAllowControl || bForceCmd) ? "" : " - Not allowed (Control turned off)";
	sprintf(szTemp, "\ar#\ax CMD: [%s]%s", pszCmd, szAllowed);
	if (!bSilentCmd) WriteOut(szTemp);

	if (bAllowControl == false)
	{
		if (!bForceCmd)
		{
			return;
		}
	}

	sprintf(szTemp, pszCmd);
	strCleanEnd(szTemp);

	PCHARINFO pCharInfo=GetCharInfo();
	PSPAWNINFO pSpawn=(PSPAWNINFO)pCharSpawn;
	if (pCharInfo) pSpawn=pCharInfo->pSpawn;

	if (pSpawn)
	{
		DoCommand((PSPAWNINFO)pSpawn,szTemp);
	}
	bForceCmd = false;
}

void HandleBciMessage(char *szName, char *szMsg)
{
   char szBuff[1024] = {0};

   if(!strncmp(szMsg, "REQ", 3))
   {
      if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn)
      {

         sprintf(szBuff, "1|%d|%d|%d|%d|%d|%d|%d|%d|%s|%s|%d|%d|%d|%c|%c", GetCurHPS(), GetMaxHPS(), GetCharInfo2()->Mana, GetMaxMana(), GetCharInfo2()->Endurance, GetMaxEndurance(),
            GetCharInfo()->zoneId, GetCharInfo2()->Level, GetClassDesc(GetCharInfo2()->Class), pEverQuest->GetRaceDesc(GetCharInfo2()->Race), GetCharInfo()->Exp,
            GetCharInfo()->AAExp, GetCharInfo2()->AAPoints, ((PSPAWNINFO)pCharSpawn)->StandState, (((PSPAWNINFO)pCharSpawn)->PetID == 0xFFFFFFFF) ? '0' : '1');
         BciTransmit(szName, szBuff);
      }
   }
}

void HandleControlMessage(PCHAR rawmsg)
{
	// DebugSpew("HandleIncomingMessage Start()");

	if (!strncmp(rawmsg, "PING", 4)) 
	{
		lastPingTime = clock();
		transmit(true, CMD_PONG);
	}
	else if (!strncmp(rawmsg, "NBPKT:", 6))
	{
		SendNetBotMsg(rawmsg + 6);
	}
	else if (!strncmp(rawmsg, "NB", 2))
	{
		CHAR Output[64];
		if (!strncmp(rawmsg, "NBJOIN=", 7))
		{
			sprintf(Output, "\ar#\ax - %s has joined the server.", &rawmsg[7]);
			WriteOut(Output);
		}
		if (!strncmp(rawmsg, "NBQUIT=", 7))
		{
			sprintf(Output, "\ar#\ax - %s has left the server.", &rawmsg[7]);
			WriteOut(Output);
		}
		SendNetBotEvent(rawmsg);
	}

	// DebugSpew("HandleIncomingMessage End()");
}

void HandleIncomingString(PCHAR rawmsg)
{
	CHAR szTemp[MAX_STRING] = {0};
	CHAR lastChar = 0;
	int i_src = 0;
	int i_dest = 0;
	int msgCharCount = -1;
	int msgTell = 0;
	CHAR colorCh=0;
	boolean isSysMsg = false;
	PCHAR pszCmdStart = NULL;
	bool bBciCmd = false;

	// DebugSpew("HandleIncomingMsg: %s", rawmsg);

	if (!rawmsg) return;

	isSysMsg = (*rawmsg == '-');


	//Tim: Added 9/22/10
	//WriteChatColor("RAW:");
	//WriteChatColor(rawmsg);
	std::string msg = rawmsg;
	if(msg.find("<GVAR>") != std::string.npos) {
		HandleGlobalVar(msg);
		return;
	}
	//End Add

	if (*rawmsg == '\t')
	{
		HandleControlMessage(rawmsg + 1);
		return;
	}

	while (rawmsg[i_src] != 0 && i_dest < (MAX_STRING-1))
	{
		if (i_src == 0 && isSysMsg) // first char
		{
			i_dest += WriteStringGetCount(&szTemp[i_dest], "\ar#\ax ");
			lastChar = *rawmsg;
			msgCharCount=1;
		}
		else if (lastChar != ' ' || rawmsg[i_src] != ' ') // Do not add extra spaces
		{
			if (msgCharCount <= 1 && strnicmp(&rawmsg[i_src], szToonCmdStart, strlen(szToonCmdStart)) == 0)
			{
				pszCmdStart = &rawmsg[i_src + strlen(szToonCmdStart) - 1];
			}

			if (msgCharCount <=1 && msgTell==1 && strnicmp(&rawmsg[i_src], "//", 2) == 0)
			{
				pszCmdStart = &rawmsg[i_src + 1];
			}

			if (msgCharCount <=1 && bBciCmd)
			{
				pszCmdStart = &rawmsg[i_src];
			}

			if (msgCharCount >= 0)
			{
				msgCharCount++;
				// if not in cmdMode and have room, check for colorCode.
				if (pszCmdStart == NULL && (colorCh = getColorCharFromCode(&rawmsg[i_src])) != 0 && (i_dest + (isupper(colorCh) ? 5 : 4)) < MAX_STRING)
				{
					//DebugSpewAlways("Got Color: %c", colorCh);
					boolean bDark = isupper(colorCh);
					szTemp[i_dest++] = '\a';
					if (bDark) szTemp[i_dest++] = '-';
					szTemp[i_dest++] = tolower(colorCh);
					lastChar = '\a'; // Something that should not exist otherwise
					i_src += 4; // Color code format is [+x+] - 5 characters.
				}
				else
				{
					lastChar = szTemp[i_dest] = rawmsg[i_src];
					i_dest++;
				}
			}
			else if (isSysMsg == false)
			{
				switch (rawmsg[i_src])
				{
					case '<':
					{
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME_BRACKET);
						szTemp[i_dest++] = (bIrcCompatMode) ? '<' : '>';
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME);
						break;
					}
					case '>':
					{
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME_BRACKET);
						szTemp[i_dest++] = (bIrcCompatMode) ? '>' : '<';
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
						msgCharCount = 0;
						break;
					}
					case '[':
					{
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME_BRACKET);
						szTemp[i_dest++] = '[';
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME);
						break;
					}
					case ']':
					{
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME_BRACKET);
						if (bIrcCompatMode) {
							szTemp[i_dest++] = '(';
							i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
							i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME);
							i_dest += WriteStringGetCount(&szTemp[i_dest], "msg");
							i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
							i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_NAME_BRACKET);
							i_dest += WriteStringGetCount(&szTemp[i_dest], ")]");
						} else {
							szTemp[i_dest++] = ']';
						}
						i_dest += WriteStringGetCount(&szTemp[i_dest], COLOR_OFF);
						msgCharCount = 0;
						msgTell=1;
						break;
					}
					case '{':
						break;
					case '}':
						i_dest++;
						msgCharCount = 0;
						bBciCmd = true;
						break;
					default:
					{
						lastChar = szTemp[i_dest] = rawmsg[i_src];
						i_dest++;
						break;
					}
				}
			}
		}

		i_src++;
	}

	if (pszCmdStart)
	{
		if(bBciCmd)
		{
			HandleBciMessage(szTemp, pszCmdStart);
			return;
		}
		HandleIncomingCommand(pszCmdStart);
	}

	// DebugSpew("Writing incoming");

	WriteOut(szTemp);

	// DebugSpew("HandleIncomingMessage finish");
}

PCSIDLWND GetCharacterSelectWindow(VOID)
{
	// Could put name into .ini
	return (PCSIDLWND)FindMQ2Window("CharacterSelectWindow");
}

BOOL LoginReady(VOID)
{
	return (pWndMgr && GetCharacterSelectWindow());
}

BOOL GetWindowItemForLogin(PCHAR szLoginToonName, PCHAR szBuf, int maxBufLen)
{
	// LoginReady() should be checked before this call.
	// It's here to avoid bad things.
	// This returns false on Could Not Find Character.

	if (LoginReady() == false || szLoginToonName == NULL || *szLoginToonName == 0 || maxBufLen < 1 || szBuf == NULL) return false;

	*szBuf = 0;

	PCSIDLWND pWnd = GetCharacterSelectWindow();
	PCSIDLWND pChildWnd;

	CHAR szLowerToonName[MAX_STRING];
	CHAR szText[MAX_STRING];
	CHAR *szValidate = "CSW_Char"; // First letters of button. So you can't login Quit.

	strcpy(szLowerToonName, szLoginToonName);
	strlwr(szLowerToonName);

	pChildWnd=(PCSIDLWND)pWnd->pFirstChildWnd;


	while(pChildWnd && *szBuf == 0)
	{
		if (CXMLData *pXMLData=((CXWnd*)pChildWnd)->GetXMLData())
		{
			if (pChildWnd->WindowText)
			{
				GetCXStr(pChildWnd->WindowText, szText, MAX_STRING);
				strlwr(szText);
				if (strncmp(szText, szLowerToonName, strlen(szLowerToonName)) == 0)
				{
					GetCXStr(pXMLData->Name.Ptr, szBuf, maxBufLen);
					if (strncmp(szValidate, szBuf, strlen(szValidate)) != 0)
					{
						*szBuf = 0;
					}
				}
			}
		}
		pChildWnd=(PCSIDLWND)(pChildWnd->pNextSiblingWnd);
	}

	return (*szBuf);
}

BOOL LoginCharacter(PCHAR ToonName)
{
	CHAR szTemp[MAX_STRING];
	CHAR szWindowItem[MAX_STRING];
	// LoginReady() should be checked before this call.

	if (GetWindowItemForLogin(ToonName, szWindowItem, MAX_STRING) == false)
	{
		sprintf(szTemp, "\ar#\ax Character [%s] not found, or not at CharacterSelectionWindow", ToonName);
		WriteOut(szTemp);
		return false;
	}

	sprintf(szTemp, "\ar#\ax Logging in [%s]", ToonName);
	WriteOut(szTemp);

	sprintf(szTemp, "/notify CharacterSelectWindow %s leftmouseup", szWindowItem);
	DoCommand((PSPAWNINFO)pLocalPlayer, szTemp);
	DoCommand((PSPAWNINFO)pLocalPlayer, "/notify CharacterSelectWindow CSW_Enter_World_Button leftmouseup");

	return true;
}

void TryRelogin()
{
	CHAR szMsg[MAX_STRING];

	if (reloginBeforeSecs == 0) return;

	if (reloginBeforeSecs < (GetTickCount() / 1000))
	{
		// timed out
		sprintf(szMsg, "Failed to reach character select in time to relog as [%s]", szToonToRelog);
		echoTransmit(false, szMsg);
		reloginBeforeSecs = 0;
		*szToonToRelog = 0;
		return;
	}

	if (*szToonToRelog == 0)
	{
		reloginBeforeSecs = 0;
		return;
	}

	if (LoginReady())
	{
		reloginBeforeSecs = 0;
		sprintf(szMsg, "Relog as %s: ", szToonToRelog, LoginCharacter(szToonToRelog) ? "NOW" : "FAILED");
		echoTransmit(false, szMsg);
		HandleDisconnect(true);
	}
}

VOID EQBCFont(PSPAWNINFO pChar, PCHAR Line)
{
	if (chatWnd)
	{
		if (Line[0])
		{
			int size = atoi(Line);
			if (size < 0 || size > 10) {
				WriteChatf("Usage: /bcfont 0-10");
				return;
			}
			chatWnd->SetFontSize(size);
		}
	}
}

VOID EQBCMin(PSPAWNINFO pChar, PCHAR Line)
{
	if (chatWnd)
	{
		((CXWnd*)chatWnd)->OnMinimizeBox();
	}
}
//Tim: 9/22/10

/********************************************************************************
*********************************************************************************
BEGIN TYPE VERIFICATION
*********************************************************************************
********************************************************************************/
bool StringIsInt(std::string value) {
	std::istringstream iss(value);
	int integer = 0;

	if(!(iss >> integer))
		return false;
	else
		return true;
}

int StringToInt(std::string value) {
	std::istringstream iss(value);
	int integer = 0;

	if(!(iss >> integer))
		return false;
	else
		return integer;
}

bool StringIsFloat(std::string value) {
	std::istringstream iss(value);
	float floating = 0;

	if(!(iss >> floating))
		return false;
	else
		return true;
}

float StringToFloat(std::string value) {
	std::istringstream iss(value);
	float floating = 0;

	if(!(iss >> floating))
		return false;
	else
		return floating;
}

bool StringIsBool(std::string value) {
	if(value == "true" || value == "TRUE" || value == "false" || value == "FALSE" || value == "null" || value == "NULL")
		return true;

	return false;
}

bool StringToBool(std::string value) {
	if(value == "true" || value == "TRUE") 
		return true;

	return false;
}
/*****************************
******************************
END TYPE VERIFICATION
******************************
*****************************/

class MQ2GlobalValType *pGlobalValType = 0;
class MQ2GlobalVarType *pGlobalVarType = 0;
/* variables used */
map<std::string, std::string> globals;
const std::string DELETE_STR = "delete";
const std::string SET_STR = "set";

typedef pair<std::string, std::string> value_type;
typedef map<std::string, std::string>::iterator iter_type;

void sendGlobalVarPacket(std::string variable, std::string value, std::string action);

/* function to get a val for a variable */
std::string getValByVarName(std::string variable) {
	/*
	char szTemp[1024];
	sprintf(szTemp, "GET:[%s]", variable.c_str());
	WriteChatColor(szTemp);
	*/
	iter_type it = globals.find(variable);

	if(it == globals.end())
		return "";

	return it->second;
}

/* function to set a val for a variable */
void setValByVarName(std::string variable, std::string value, bool sendUpdate = true) {
	/*
	char szTemp[1024];
	sprintf(szTemp, "SET:[%s][%s]", variable.c_str(), value.c_str());
	WriteChatColor(szTemp);
	*/
	bool bFound = false;
	iter_type it = globals.begin();
	while(it != globals.end()) {
		if(it->first == variable) {
			it->second = value;
			bFound = true;
			break;
		} else {
			++it;
		}
	}

	if(!bFound) {
		value_type global;
		global.first = variable;
		global.second = value;
		globals.insert(global);
	}

	if(sendUpdate)
		sendGlobalVarPacket(variable, value, SET_STR);
}

/* function to declare a variable */
void declareVar(std::string variable) {
	/*
	char szTemp[1024];
	sprintf(szTemp, "DECLARE:[%s]", variable.c_str());
	WriteChatColor(szTemp);
	*/
	std::string value = "null";
	value_type global;
	global.first = variable;
	global.second = value;
	globals.insert(global);

	sendGlobalVarPacket(variable, value, SET_STR);
}

/* function to delete a variable */
bool deleteVarByName(std::string variable, bool sendUpdate = true) {
	/*
	char szTemp[1024];
	sprintf(szTemp, "DELETE:[%s]", variable.c_str());
	WriteChatColor(szTemp);
	*/
	bool bFound = false;
	iter_type it = globals.begin();
	value_type deleted;
	while(it != globals.end()) {
		if(it->first == variable) {
			deleted.first = it->first;
			deleted.second = it->second;
			globals.erase(it);
			bFound = true;
			break;
		} else {
			++it;
		}
	}

	if(bFound) {
		if(sendUpdate)
			sendGlobalVarPacket(deleted.first, deleted.second, DELETE_STR);

		return true;
	}
	
	return false;
}

/*
{GlobalVarName}{GlobalVarVal}{Action}
Action can be:set, delete
*/
void HandleGlobalVar(std::string line) {
	/*
	char szTemp[1024];
	sprintf(szTemp, "GLOBAL:%s", line.c_str());
	WriteChatColor(szTemp);
	*/
	std::string variable, value, action;
	value_type varSet;
	bool bCopy = false;
	int varCount = 1;
	for(int i = 0; i < line.length(); ++i) {
		if(line[i] == '{')
			bCopy = true;
		else if(line[i] == '}') {
			bCopy = false;
			++varCount;
		} else if(bCopy) {
			if(varCount == 1)
				variable += line[i];
			else if(varCount == 2)
				value += line[i];
			else if(varCount == 3)
				action += line[i];
		}
	}

	varSet.first = variable;
	varSet.second = value;	

	/*
	sprintf(szTemp, "VAR:[%s]", variable.c_str());
	WriteChatColor(szTemp);
	sprintf(szTemp, "VALUE:[%s]", value.c_str());
	WriteChatColor(szTemp);
	sprintf(szTemp, "ACTION:[%s]", action.c_str());
	WriteChatColor(szTemp);
	*/

	if(action == SET_STR)
		setValByVarName(varSet.first, varSet.second, false);
	else if(action == DELETE_STR)
		deleteVarByName(varSet.first, false);
}

bool GlobalVarExists(PCHAR szName) {
	iter_type it;
	std::string name = szName;
	for(it = globals.begin(); it != globals.end(); ++it) {
		if(name == it->first)
			return true;
	}

	return false;
}

void DeleteGlobalVars() {
	globals.clear();
}

/*
Action can be:set, delete
*/
void sendGlobalVarPacket(std::string variable, std::string value, std::string action) {
	/*
	char szTemp[1024];
	sprintf(szTemp, "SENDING:[%s][%s][%s]", variable.c_str(), value.c_str(), action.c_str());
	WriteChatColor(szTemp);
	*/
	char szBuff[1024] = {0};
	CHAR szCommand[] = CMD_GVAR;
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "{%s}{%s}{%s}", variable.c_str(), value.c_str(), action.c_str());

		int err;
		err = send(theSocket, szCommand, strlen(szCommand), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
			CheckSocket("GlobalVar:Send1", err);
		}

		err = send(theSocket, szBuff, strlen(szBuff), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
			CheckSocket("GlobalVar:Send2", err);
		}

		err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
		if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
			CheckSocket("GlobalVar:Send3", err);
		}
	}
}


class MQ2GlobalVarType : public MQ2Type {
private:
	std::string variable, value;

public:
	enum GlobalVarMembers {
		GetVal=1,
		SetVal=2,
		Delete=3
	};

	MQ2GlobalVarType(std::string var, std::string val):MQ2Type("GlobalVar") {
		TypeMember(GetVal);
		TypeMember(SetVal);
		TypeMember(Delete);
		variable = var;
		value = val;
	}

	MQ2GlobalVarType():MQ2Type("GlobalVar") {
		TypeMember(GetVal);
		TypeMember(SetVal);
		TypeMember(Delete);
	}

	~MQ2GlobalVarType() {}

	void setVar(std::string var) {
		variable = var;
	}

	void setVal(std::string val) {
		value = val;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2GlobalVarType::FindMember(Member);
		if (!pMember)
			return false;
		switch((GlobalVarMembers)pMember->ID) {
			case GetVal:
				if (StringIsInt(value)) {
					Dest.Int = StringToInt(value);
					Dest.Type = pIntType;
				} else if(StringIsFloat(value)) {
					Dest.Float = StringToFloat(value);
					Dest.Type = pFloatType;
				} else if (StringIsBool(value)) {
					Dest.DWord = StringToBool(value);
					Dest.Type = pBoolType;
				} else {
					strcpy(DataTypeTemp, value.c_str());
					Dest.Ptr = DataTypeTemp;
					Dest.Type = pStringType;
				}

				return true;
			case SetVal:
				setValByVarName(variable, Index);
				strcpy(DataTypeTemp, Index);

				value = Index;
				if (StringIsInt(value)) {
					Dest.Int = StringToInt(value);
					Dest.Type = pIntType;
				} else if(StringIsFloat(value)) {
					Dest.Float = StringToFloat(value);
					Dest.Type = pFloatType;
				} else if (StringIsBool(value)) {
					Dest.DWord = StringToBool(value);
					Dest.Type = pBoolType;
				} else {
					strcpy(DataTypeTemp, value.c_str());
					Dest.Ptr = DataTypeTemp;
					Dest.Type = pStringType;
				}

				return true;

			case Delete:
				if(deleteVarByName(variable))
					Dest.DWord = true;
				else
					Dest.DWord = false;

				Dest.Type = pBoolType;
				return true;
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		if(value.length() > 0)
			strcpy(Destination, value.c_str());
		else
			strcpy(Destination, "NULL");

		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

BOOL dataGlobalVar(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pGlobalVarType;

	if(!GlobalVarExists(szName)) {
		//create the variable
		declareVar(szName);
	}

	pGlobalVarType->setVal(getValByVarName(szName));
	pGlobalVarType->setVar(szName);	

	return true;
}

//End Tim 9/22/10

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	CHAR szTemp[MAX_STRING] = {0};

	DebugSpewAlways("Initializing MQ2Eqbc");

	AddCommand("/bc", BoxChatSay);
	AddCommand("/bct", BoxChatTell);
	AddCommand("/bca", BoxChatAllButMe);
	AddCommand("/bcaa", BoxChatAll);
	AddCommand("/bccmd", BoxChatCommand);
	AddCommand("/bcfont", EQBCFont);
	AddCommand("/bcmin", EQBCMin);

	//Tim: 9/22/10
	pGlobalVarType = new MQ2GlobalVarType;
	AddMQ2Data("Globals", dataGlobalVar);
	//End Tim

	pEQBCType= new EQBCType;
	AddMQ2Data("EQBC",dataEQBC);

	GetPrivateProfileString("Last Connect", "Server", "127.0.0.1", szServer, MAX_STRING, INIFileName);
	GetPrivateProfileString("Last Connect", "Port", "2112", szPort, MAX_STRING, INIFileName);
	bAutoConnect = GetPrivateProfileInt("Settings", "AutoConnect", 0, INIFileName);
	bAllowControl = GetPrivateProfileInt("Settings", "AllowControl", 1, INIFileName);
	bSilentCmd = GetPrivateProfileInt("Settings", "SilentCmd", 0, INIFileName);
	bDoTellWatch = GetPrivateProfileInt("Settings", "TellWatch", 0, INIFileName);
	bDoGuildWatch = GetPrivateProfileInt("Settings", "GuildWatch", 0, INIFileName);
	bDoGroupWatch = GetPrivateProfileInt("Settings", "GroupWatch", 0, INIFileName);
	bDoFSWatch = GetPrivateProfileInt("Settings", "FSWatch", 0, INIFileName);
	bIrcCompatMode = GetPrivateProfileInt("Settings", "IRCCompatMode", 1, INIFileName);
	bReconnectMode = GetPrivateProfileInt("Settings","AutoReconnect",1,INIFileName);
	iReconnectSeconds = GetPrivateProfileInt("Settings", "ReconnectRetrySeconds", 15, INIFileName);
	bUseWindow = GetPrivateProfileInt("Settings", "UseWindow", 0, INIFileName);
	bLocalEchoMode = GetPrivateProfileInt("Settings", "LocalEcho", 1, INIFileName);
	bSaveCharacter = GetPrivateProfileInt("Settings", "SaveByCharacter", 1, INIFileName);

	SetPlayer();
	CreateChatWindowIfNeeded();

	sprintf(szTemp, "\ar#\ax Welcome to \ayMQ2Eqbc\ax, %s: Use \ar/bccmd help\ax to see help.", szToonName);
	WriteChatColor(szTemp);
	//WriteOut(szTemp);

	InitializeCriticalSection(&ConnectCS);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Eqbc");

	if (bConnected)
	{
		WriteChatColor("\ar#\ax You are still connected! Attempting to disconnect.");
		DebugSpewAlways("MQ2Eqbc::Still Connected::Attempting disconnect.");
		HandleDisconnect(false);
	}

	closeChatWindow();

	RemoveCommand("/bc");
	RemoveCommand("/bct");
	RemoveCommand("/bca");
	RemoveCommand("/bcaa");
	RemoveCommand("/bccmd");
	RemoveCommand("/bcfont");
	RemoveCommand("/bcmin");

	RemoveMQ2Data("EQBC");
	delete pEQBCType;

	//Tim: 9/22/10
	RemoveMQ2Data("Globals");
	delete pGlobalVarType;

	if(globals.size() > 0)
		globals.clear();
	//End Tim

	// make sure we're not trying to connect...
	EnterCriticalSection(&ConnectCS);
	LeaveCriticalSection(&ConnectCS);
	DeleteCriticalSection(&ConnectCS);
}

// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	int err;

	if (bTriedConnect)
	{
		bTriedConnect=false;
		if (bConnected)
		{
			WriteOut("\ar#\ax Connected!");
			WritePrivateProfileString("Last Connect", "Server", szServer, INIFileName);
			WritePrivateProfileString("Last Connect", "Port", szPort, INIFileName);
			lastReadBufPos = 0;
			lastReconnectTimeSecs = 0;
			HandleChannels(NULL);
			SendCmdLocalEcho();
		}
		else
		{
			WriteOut("\ar#\ax Could not connect.");
		}
	}

	// Fill the input buffer with new data, if any.

	if (bConnected)
	{
		for (;lastReadBufPos<(MAX_READBUF-1);lastReadBufPos++)
		{
			err = recv(theSocket, &ireadbuf[lastReadBufPos], 1, 0);
			if ((ireadbuf[lastReadBufPos] == '\n') || (err == 0) || (err == SOCKET_ERROR))
			{
				if (ireadbuf[lastReadBufPos] == '\n')
				{
					ireadbuf[lastReadBufPos] = '\0';
					HandleIncomingString(ireadbuf);
					lastReadBufPos = -1;
				}
				//break;
			}
			if (err == 0 || err == SOCKET_ERROR) break;
		}

		if (lastReadBufPos < 0) lastReadBufPos = 0;
		if (err == 0 && WSAGetLastError() == 0)
		{
			// Should be giving WSAWOULDBLOCK
			bSocketGone = true;
		}

		if (bSocketGone)
		{
			bSocketGone = false;
			HandleIncomingString("-- Remote connection closed, you are no longer connected");
			HandleDisconnect(false);
			if (bReconnectMode && iReconnectSeconds > 0)
			{
				lastReconnectTimeSecs = GetTickCount()/1000;
			}
		}
		if ( lastPingTime > 0 && lastPingTime + 120000 < clock() )
		{
			WriteChatf( "\arMQ2EQBC: did not recieve expected ping from server, pinging..." );
			transmit( true, CMD_PONG );
			lastPingTime = 0;
		}
	}
	else if (lastReconnectTimeSecs > 0 && bConnecting == false)
	{
		if (lastReconnectTimeSecs + iReconnectSeconds < GetTickCount()/1000)
		{
			lastReconnectTimeSecs = GetTickCount()/1000;
			HandleConnectRequest("");
		}
	}

	if (reloginBeforeSecs)
	{
		TryRelogin();
	}
}

PLUGIN_API VOID SetGameState(DWORD GameState)
{
	if (GameState==GAMESTATE_INGAME)
	{
		CreateChatWindowIfNeeded();
	}
	else
	{
		closeChatWindow();
	}

	if (GameState==GAMESTATE_INGAME && !bConnected)
	{
		if (bAutoConnect && !bConnected)
		{
			SetPlayer();
			HandleConnectRequest("");
		}
	}
	else if (bConnected && GameState==GAMESTATE_CHARSELECT)
	{
		if (reloginBeforeSecs == 0)
		{
			HandleDisconnect(true);
		}
	}
}

PLUGIN_API VOID OnCleanUI(VOID)
{

	DebugSpewAlways("MQ2WndTest::OnCleanUI()");
	closeChatWindow();
}

PLUGIN_API VOID OnReloadUI()
{
	CreateChatWindowIfNeeded();
	if (bUseWindow == false) closeChatWindow();
}

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	PSPAWNINFO pChar = (PSPAWNINFO)pCharSpawn;
	if (GetGameState() != GAMESTATE_INGAME || !pChar)
	{
		return 0;
	}

	if (bConnected)
	{
		CHAR szSender[MAX_STRING];
		CHAR szTell[MAX_STRING];
		CHAR szBCMSG[MAX_STRING];
		PSTR Text;

		if (bDoTellWatch && Color == USERCOLOR_TELL)
		{
			GetArg(szSender, Line, 1);
			Text = GetNextArg(Line, 1, FALSE, '\'');
			strcpy(szTell, Text);
			szTell[strlen(Text)-1] = '\0';

			sprintf(szBCMSG, "Tell received from %s: %s", szSender, szTell);
			BoxChatSay(pChar, szBCMSG);
			return 0;
		}
		if (bDoGuildWatch && Color == USERCOLOR_GUILD)
		{
			GetArg(szSender, Line, 1);
			Text = GetNextArg(Line, 1, FALSE, '\'');
			strcpy(szTell, Text);
			szTell[strlen(Text)-1] = '\0';

			sprintf(szBCMSG, "Guild chat from %s: %s", szSender, szTell);
			BoxChatSay(pChar, szBCMSG);
			return 0;
		}
		if (bDoGroupWatch && Color == USERCOLOR_GROUP)
		{
			GetArg(szSender, Line, 1);
			Text = GetNextArg(Line, 1, FALSE, '\'');
			strcpy(szTell, Text);
			szTell[strlen(Text)-1] = '\0';

			sprintf(szBCMSG, "Group chat from %s: %s", szSender, szTell);
			BoxChatSay(pChar, szBCMSG);
			return 0;
		}
		if (bDoFSWatch && Color == USERCOLOR_FELLOWSHIP)
		{
			GetArg(szSender, Line, 1);
			Text = GetNextArg(Line, 1, FALSE, '\'');
			strcpy(szTell, Text);
			szTell[strlen(Text)-1] = '\0';

			sprintf(szBCMSG, "Fellowship chat from %s: %s", szSender, szTell);
			BoxChatSay(pChar, szBCMSG);
			return 0;
		}
	}

	return 0;
}

// ---------- EQBCType Methods
EQBCType::EQBCType():MQ2Type("EQBC")
{
	TypeMember(Connected);
}

EQBCType::~EQBCType() { }

bool EQBCType::GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
{
	PMQ2TYPEMEMBER pMember=EQBCType::FindMember(Member);
	if(!pMember) return false;
	switch((VarMembers)pMember->ID)
	{
		case Connected:
			Dest.DWord=bConnected;
			Dest.Type=pBoolType;
			return true;
	}
	return false;
}

bool EQBCType::ToString(MQ2VARPTR VarPtr, PCHAR Destination)
{
	strcpy(Destination,"EQBC");
	return true;
}

bool EQBCType::FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source)
{
	return false;
}

bool EQBCType::FromString(MQ2VARPTR &VarPtr, PCHAR Source)
{
	return false;
}

// NetBot Support
PLUGIN_API VOID NetBotSendMsg(PCHAR szMsg)
{
	if (szMsg && *szMsg)
	{
		transmit(true, "\tNBMSG");
		transmit(true, szMsg);
		// DebugSpew("Sending NBMSG: %s", szMsg);
	}
}

PLUGIN_API VOID NetBotRequest(PCHAR szMsg)
{
	if (szMsg && *szMsg)
	{
		if (strcmp(szMsg, "NAMES") == 0)
		{
			transmit(true, "\tNBNAMES");
		}
	}
}

PLUGIN_API WORD isConnected(void)
{
	return (WORD)bConnected;
} 
