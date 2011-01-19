/*
MQ2EQBH
Clone of EQBC that has been hacked down to suit our needs.
By: Tim Hansen
*/

// MQ2EQBC.cpp - Much of the code here came from MQ2Irc and MQ2Telnet.
// Author: Omnictrl (o)
// Contributor: ascii (a)
// Contributor: Vladus2000 (v)
// Contributor: pms (p)
// Contributor: ieatacid (i)
/*
* Version 1.0.o2 - 20050926
*/

#define PROG_VERSION "MQ2EQBH v1.3.p5"

#pragma comment(lib,"wsock32.lib")
#include "../MQ2Plugin.h"

#include <vector>

//Tim: 9/22/10
#include <sstream>
//End Tim

PreSetup("MQ2Eqbh");

// Shared Info
class EQBHType : public MQ2Type
{
public:
	enum VarMembers { Connected=1 };
	EQBHType();
	~EQBHType();
	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest);
	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination);
	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source);
	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source);
};

class EQBHType *pEQBHType=0;

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
//Tim
//10/14/10
#define CMD_START "\tSTARTROTATION\n"
#define CMD_STOP "\tSTOPROTATION\n"
#define CMD_DELAY "\tDELAY\n"
#define CMD_TANK "\tTANK\n"
//End Tim

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

void WriteOut(char *szText);
void transmit(bool handleDisconnect, PCHAR szMsg);
void HandleIncomingString(PCHAR rawmsg);
VOID BoxHChatCommand(PSPAWNINFO pChar, PCHAR szLine);
VOID BoxHChatTell(PSPAWNINFO pChar, PCHAR szLine);
VOID BoxHChatAll(PSPAWNINFO pChar, PCHAR szLine);
VOID BoxHChatAllButMe(PSPAWNINFO pChar, PCHAR szLine);

BOOL dataEQBH(PCHAR Index, MQ2TYPEVAR &Dest)
{
	Dest.DWord=1;
	Dest.Type=pEQBHType;
	return true;
}

// ------------------------

DWORD WINAPI EQBHConnectThread(LPVOID lpParam)
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
		// DebugSpew("MQ2Eqbh Connecting");
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
		// DebugSpew("MQ2Eqbh Connected");

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

void WriteOut(char *szText)
{
	WriteChatColor(szText);

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
		sprintf(szTemp,"\ar#\ax MQ2Eqbh Status: ONLINE - %s - %s", szServer, szPort);
		WriteOut(szTemp);
	}
	else
	{
		WriteOut("\ar#\ax MQ2Eqbh Status: OFFLINE");
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
	WriteOut("\ar#\ax \ay/bhcmd connect <server> <port> <pw>\ax (defaults: 127.0.0.1 2112)");
	WriteOut("\ar#\ax \ay/bhcmd quit\ax (disconnects)");
	WriteOut("\ar#\ax \ay/bhcmd help\ax (show this screen)");
	WriteOut("\ar#\ax \ay/bhcmd status\ax (show connected or not and settings)");
	WriteOut("\ar#\ax \ay/bhcmd reconnect\ax (close current connection and connect again)");
	WriteOut("\ar#\ax \ay/bhcmd colordump\ax (Shows all available color codes)");
	WriteOut("\ar#\ax \ay/bhcmd togglecontrol\ax (allow remote control)");
	WriteOut("\ar#\ax \ay/bhcmd togglewindow\ax (toggles use of dedicated window)");
	WriteOut("\ar#\ax \ay/bhcmd togglecompatmode\ax (toggle IRC Compatability mode)");
	WriteOut("\ar#\ax \ay/bhcmd toggleautoconnect\ax (toggle auto connect)");
	WriteOut("\ar#\ax \ay/bhcmd togglereconnect\ax (toggle auto-reconnect mode on server disconnect)");
	WriteOut("\ar#\ax \ay/bhcmd toggletellwatch\ax (toggle relay of tells received to /bh)");
	WriteOut("\ar#\ax \ay/bhcmd toggleguildwatch\ax (toggle relay of guild chat to /bh)");
	WriteOut("\ar#\ax \ay/bhcmd togglegroupwatch\ax (toggle relay of group chat to /bh)");
	WriteOut("\ar#\ax \ay/bhcmd togglefswatch\ax (toggle relay of fellowship chat to /bh)");
	WriteOut("\ar#\ax \ay/bhcmd togglesilentcmd\ax (toggle display of 'CMD: [command]' echo)");
	WriteOut("\ar#\ax \ay/bhcmd togglesavebychar\ax (toggle saving UI data to [CharName] in INI)");
	WriteOut("\ar#\ax \ay/bhcmd setreconnectsecs n\ax (n is seconds to reconnect: default 15)");
	WriteOut("\ar#\ax \ay/bhcmd stopreconnect\ax (stop trying to reconnect for now)");
	WriteOut("\ar#\ax \ay/bhcmd relog <charname>\ax (relog in as charname if you camp < 60 seconds): No charname resets");
	WriteOut("\ar#\ax \ay/bhcmd channels <channel list>\ax (set list of channels to receive tells from)");
	WriteOut("\ar#\ax \ay/bhcmd togglelocalecho\ax (toggle echoing my commands back to me if I am in channel)");
	WriteOut("\ar#\ax \ay/bh your text\ax (send text)");
	WriteOut("\ar#\ax \ay/bht ToonName your text\ax (send your text to specific Toon)");
	WriteOut("\ar#\ax \ay/bht ToonName //command\ax (send Command to ToonName)");
	WriteOut("\ar#\ax \ay/bha //command\ax (send Command to all connected names EXCLUDING yourself)");
	WriteOut("\ar#\ax \ay/bhaa //command\ax (send Command to all connected names INCLUDING yourself)");
	//Tim
	//10/14/10
	WriteOut("\ar#\ax \ay/bhcmd start (Starts the CH Rotation)");
	WriteOut("\ar#\ax \ay/bhcmd start (Starts the CH Rotation)");
	WriteOut("\ar#\ax \ay/bhcmd start (Starts the CH Rotation)");
	WriteOut("\ar#\ax \ay/bhcmd start (Starts the CH Rotation)");
	//End
}

void HandleConnectRequest(PCHAR szLine)
{
	CHAR szMsg[MAX_STRING];
	CHAR szIniServer[MAX_STRING] = {0};
	CHAR szIniPort[MAX_STRING] = {0};
	CHAR szIniPassword[MAX_PASSWORD] = {0};

	if (bConnected)
	{
		WriteOut("\ar#\ax Already connected. Use /bhcmd quit to disconnect first.");
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

	sprintf(szMsg, "\ar#\ax Connecting to %s %s...", szServer, szPort);
	WriteOut(szMsg);

	lastPingTime = 0;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	serverInfo.sin_port = htons(atoi(szPort));
	DWORD ThreadId;
	CreateThread(NULL, 0, &EQBHConnectThread, 0, 0, &ThreadId);

	return;
}

VOID HandleDisconnect(bool sendDisconnect)
{
	if (bConnected)
	{
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
		WriteOut("\ar#\ax You are not connected. Please use \ag/bhcmd connect\ax to establish a connection.");
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
//Tim
//10/14/10
VOID HandleStartRotation() {
	int err = send(theSocket, CMD_START, strlen(CMD_START), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleStartRotation:Send1", err);
	}

	err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleStartRotation:Send3", err);
	}
}

VOID HandleStopRotation() {
	int err = send(theSocket, CMD_STOP, strlen(CMD_STOP), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleStopRotation:Send1", err);
	}

	err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleStopRotation:Send3", err);
	}
}

VOID HandleSetDelay(std::string newDelay) {
	int err = send(theSocket, CMD_DELAY, strlen(CMD_DELAY), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleDelay:Send1", err);
	}

	err = send(theSocket, newDelay.c_str(), strlen(newDelay.c_str()), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleDelay:Send1", err);
	}

	err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleDelay:Send3", err);
	}
}

VOID HandleSetTank(std::string newTank) {
	int err = send(theSocket, CMD_TANK, strlen(CMD_TANK), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleTank:Send1", err);
	}

	err = send(theSocket, newTank.c_str(), strlen(newTank.c_str()), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleTank:Send1", err);
	}

	err = send(theSocket, SEND_LINE_TERM, strlen(SEND_LINE_TERM), 0);
	if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket("HandleTank:Send3", err);
	}
}
//End Tim
VOID BoxHChatCommand(PSPAWNINFO pChar, PCHAR szLine)
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
	//Tim
	//10/14/10
	static CHAR szCommandStartRotation[] = "start";
	static CHAR szCommandStopRotation[] = "stop";
	static CHAR szCommandSetDelay[] = "delay";
	static CHAR szCommandSetTank[] = "tank";
	//End Tim
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
		WriteOut("\ar#\ax Connection Closed, you can unload MQ2Eqbh now.");
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
			sprintf(szMsg, "\ar#\ax Invalid value given - proper example: /bhcmd setreconnectsecs 15");
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
		HandleToggleRequest(&bDoTellWatch, "TellWatch", "'Relay all tells to /bh'");
	}
	else if (stricmp(szCommandToggleGuildWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoGuildWatch, "GuildWatch", "'Relay guild chat to /bh'");
	}
	else if (stricmp(szCommandToggleGroupWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoGroupWatch, "GroupWatch", "'Relay group chat to /bh'");
	}
	else if (stricmp(szCommandToggleFSWatch, szArg) == 0)
	{
		HandleToggleRequest(&bDoFSWatch, "FSWatch", "'Relay fellowship chat to /bh'");
	}
	else if (stricmp(szCommandToggleSilentCmd, szArg) == 0)
	{
		HandleToggleRequest(&bSilentCmd, "SilentCmd", "Silence 'CMD: [command]' echo");
	}
	else if (stricmp(szCommandToggleSaveByChar, szArg) == 0)
	{
		HandleToggleRequest(&bSaveCharacter, "SaveByCharacter", "Save UI data by character name");
	}
	//Tim
	//10/14/10
	else if (stricmp(szCommandStartRotation, szArg) == 0) {
		HandleStartRotation();
	} else if (stricmp(szCommandStopRotation, szArg) == 0) {
		HandleStopRotation();
	} else if (stricmp(szCommandSetDelay, szArg) == 0) {
		GetArg(szArg, szLine, 2);
		WriteChatColor(szArg);
		std::string delay = szArg;
		HandleSetDelay(delay);
	} else if (stricmp(szCommandSetTank, szArg) == 0) {
		GetArg(szArg, szLine, 2);
		std::string tank = szArg;
		HandleSetTank(tank);
	}
	//End Tim
	else
	{
		sprintf(szMsg, "\ar#\ax Unsupported command, supported commands are: %s", SUPPORTED_COMMANDS);
		WriteOut(szMsg);
	}
}

// BoxChatSay
VOID BoxHChatSay(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szMsg[MAX_STRING]={0};
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bhcmd connect\ax to establish a connection.");
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
VOID BoxHChatTell(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szTemp[MAX_STRING]={0};
	int i_src=0;
	int i_dest=0;
	int i_len;
	CHAR szCommand[] = CMD_TELL;
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bhcmd connect\ax to establish a connection.");
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

VOID BoxHChatAllButMe(PSPAWNINFO pChar, PCHAR szLine)
{
	CHAR szCommand[] = CMD_MSGALL;
	CHAR szMsg[MAX_STRING]={0};
	int err;

	if (!bConnected) {
		WriteOut("\ar#\ax You are not connected. Please use \ag/bhcmd connect\ax to establish a connection.");
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

VOID BoxHChatAll(PSPAWNINFO pChar, PCHAR szLine)
{
	BoxHChatAllButMe(pChar, szLine);
	CHAR szTemp[ MAX_STRING ];
	sprintf(szTemp, "<%s> %s %s", pChar->Name, pChar->Name, szLine);
	bForceCmd = true;
	HandleIncomingString(szTemp);
	bForceCmd = false;
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

void HandleControlMessage(PCHAR rawmsg)
{
	// DebugSpew("HandleIncomingMessage Start()");

	if (!strncmp(rawmsg, "PING", 4)) 
	{
		lastPingTime = clock();
		transmit(true, CMD_PONG);
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
		WriteChatColor("GLOBAL VAR RECIEVED!");
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
			return;
		}
		HandleIncomingCommand(pszCmdStart);
	}

	// DebugSpew("Writing incoming");

	WriteOut(szTemp);

	// DebugSpew("HandleIncomingMessage finish");
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

//class MQ2GlobalBHValType *pGlobalBHValType = 0;
class MQ2GlobalBHVarType *pGlobalBHVarType = 0;
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

	if(GetCharInfo()->pSpawn) {
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


class MQ2GlobalBHVarType : public MQ2Type {
private:
	std::string variable, value;

public:
	enum GlobalBHVarMembers {
		GetVal=1,
		SetVal=2,
		Delete=3
	};

	MQ2GlobalBHVarType(std::string var, std::string val):MQ2Type("GlobalBHVar") {
		TypeMember(GetVal);
		TypeMember(SetVal);
		TypeMember(Delete);
		variable = var;
		value = val;
	}

	MQ2GlobalBHVarType():MQ2Type("GlobalBHVar") {
		TypeMember(GetVal);
		TypeMember(SetVal);
		TypeMember(Delete);
	}

	~MQ2GlobalBHVarType() {}

	void setVar(std::string var) {
		variable = var;
	}

	void setVal(std::string val) {
		value = val;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2GlobalBHVarType::FindMember(Member);
		if (!pMember)
			return false;
		switch((GlobalBHVarMembers)pMember->ID) {
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
	Ret.Type = pGlobalBHVarType;

	if(!GlobalVarExists(szName)) {
		//create the variable
		declareVar(szName);
	}

	pGlobalBHVarType->setVal(getValByVarName(szName));
	pGlobalBHVarType->setVar(szName);	

	return true;
}

//End Tim 9/22/10

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	CHAR szTemp[MAX_STRING] = {0};

	DebugSpewAlways("Initializing MQ2Eqbh");

	AddCommand("/bh", BoxHChatSay);
	AddCommand("/bht", BoxHChatTell);
	AddCommand("/bha", BoxHChatAllButMe);
	AddCommand("/bhaa", BoxHChatAll);
	AddCommand("/bhcmd", BoxHChatCommand);

	//Tim: 9/22/10
	pGlobalBHVarType = new MQ2GlobalBHVarType;
	AddMQ2Data("GlobalsBH", dataGlobalVar);
	//End Tim

	pEQBHType= new EQBHType;
	AddMQ2Data("EQBH",dataEQBH);

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

	sprintf(szTemp, "\ar#\ax Welcome to \ayMQ2Eqbh\ax, %s: Use \ar/bhcmd help\ax to see help.", szToonName);
	WriteChatColor(szTemp);
	//WriteOut(szTemp);

	InitializeCriticalSection(&ConnectCS);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Eqbh");

	if (bConnected)
	{
		WriteChatColor("\ar#\ax You are still connected! Attempting to disconnect.");
		DebugSpewAlways("MQ2Eqbh::Still Connected::Attempting disconnect.");
		HandleDisconnect(false);
	}

	RemoveCommand("/bh");
	RemoveCommand("/bht");
	RemoveCommand("/bha");
	RemoveCommand("/bhaa");
	RemoveCommand("/bhcmd");

	RemoveMQ2Data("EQBH");
	delete pEQBHType;

	//Tim: 9/22/10
	RemoveMQ2Data("GlobalsBH");
	delete pGlobalBHVarType;

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
			WriteChatf( "\arMQ2EQBH: did not recieve expected ping from server, pinging..." );
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

PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	PSPAWNINFO pChar = (PSPAWNINFO)pCharSpawn;
	if (!pChar)
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
			BoxHChatSay(pChar, szBCMSG);
			return 0;
		}
		if (bDoGuildWatch && Color == USERCOLOR_GUILD)
		{
			GetArg(szSender, Line, 1);
			Text = GetNextArg(Line, 1, FALSE, '\'');
			strcpy(szTell, Text);
			szTell[strlen(Text)-1] = '\0';

			sprintf(szBCMSG, "Guild chat from %s: %s", szSender, szTell);
			BoxHChatSay(pChar, szBCMSG);
			return 0;
		}
		if (bDoGroupWatch && Color == USERCOLOR_GROUP)
		{
			GetArg(szSender, Line, 1);
			Text = GetNextArg(Line, 1, FALSE, '\'');
			strcpy(szTell, Text);
			szTell[strlen(Text)-1] = '\0';

			sprintf(szBCMSG, "Group chat from %s: %s", szSender, szTell);
			BoxHChatSay(pChar, szBCMSG);
			return 0;
		}
	}

	return 0;
}

// ---------- EQBHType Methods
EQBHType::EQBHType():MQ2Type("EQBH")
{
	TypeMember(Connected);
}

EQBHType::~EQBHType() { }

bool EQBHType::GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
{
	PMQ2TYPEMEMBER pMember=EQBHType::FindMember(Member);
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

bool EQBHType::ToString(MQ2VARPTR VarPtr, PCHAR Destination)
{
	strcpy(Destination,"EQBH");
	return true;
}

bool EQBHType::FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source)
{
	return false;
}

bool EQBHType::FromString(MQ2VARPTR &VarPtr, PCHAR Source)
{
	return false;
}

PLUGIN_API WORD isConnected(void)
{
	return (WORD)bConnected;
} 
