// MQ2Center.cpp : Defines the entry point for the DLL application.
//

// PLUGIN_API is only to be used for callbacks.  All existing callbacks at this time
// are shown below. Remove the ones your plugin does not use.  Always use Initialize
// and Shutdown for setup and cleanup, do NOT do it in DllMain.
#include <winsock2.h>
#include <iostream>
#include <conio.h>
#include <iostream>
#include <sstream>

#include <string>
#include <map>
#include <vector>
#include "../MQ2Plugin.h"

void sendGlobalVarPacket(std::string variable, std::string value, std::string action);

PreSetup("MQ2Center");

SOCKET conn;
int cycle_count = 0;
int MAX_CYCLE_COUNT = 5;

int iLastConnectTry = 0;
int iMinReconnect = 15;

bool bConnected = false;
bool bInitialized = false;
bool bSocketGone = false;
bool bTriedConnect = false;
CRITICAL_SECTION ConnectCS;
bool bConnecting=false;
SOCKADDR_IN serverInfo;
LPHOSTENT hostEntry;
WORD sockVersion;
WSADATA wsaData;
int nret;
CHAR szServer[MAX_STRING] = {"127.0.0.1"};
CHAR szPort[64] = {"20248"};

void sendLoginPacket();
void closeChatWindow(void);
void CreateChatWindowIfNeeded();


DWORD WINAPI EQConnectThread(LPVOID lpParam) {
	EnterCriticalSection(&ConnectCS);
	bConnecting = true;
	nret = connect(conn, (LPSOCKADDR)&serverInfo, sizeof(struct sockaddr));

	if (nret == SOCKET_ERROR)
	{
		bConnected = false;
	}
	else
	{
		// DebugSpew("MQ2Eqbc Connecting");
		unsigned long nonblocking = 1;
		ioctlsocket(conn, FIONBIO, &nonblocking);
		Sleep((clock_t)4 * CLOCKS_PER_SEC/2);
		bConnected = true;
		WriteChatColor("MQ2Center Reconnected!");
	}

	bTriedConnect = true;
	bConnecting = false;
	sendLoginPacket();
	LeaveCriticalSection(&ConnectCS);
	return 0;
}

void HandleConnectRequest() {

	if (bConnected)
	{
		WriteChatColor("You are already connected");
		return;
	}

	if (bConnecting)
	{
		WriteChatColor("\ar#\ax Already trying to connect! Hold on a minute there");
		return;
	}

	sockVersion = MAKEWORD(1, 1);
	WSAStartup(sockVersion, &wsaData);
	hostEntry = gethostbyname(szServer);
	if (!hostEntry)
	{
		WSACleanup();
		return;
	}

	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET) {
		WSACleanup();
		return;
	}

	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	serverInfo.sin_port = htons(atoi(szPort));
	DWORD ThreadId;
	CreateThread(NULL, 0, &EQConnectThread, 0, 0, &ThreadId);

	return;
}

void CheckSocket(int err) {
	int werr = WSAGetLastError();
	//char temp[64] = {0};
	//sprintf(temp, "The last error being sent was %d", werr);
	//WriteChatColor(temp);
	if (werr == WSAECONNABORTED) {
		bSocketGone = true;
	}

	if(werr == WSAECONNRESET) {
		bConnected = false;
		HandleConnectRequest();
	}
	WSASetLastError(0);
	// DebugSpewAlways("Sock Error-%s: %d / w%d", szFunc, err, werr);
}

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
	if(value == "true" || value == "TRUE" || value == "false" || value == "FALSE" || value == "null")
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

std::string readSocket() {
	int MAX_READ = 512;
	std::string message;
	char temp = '\n';
	int err = 0;

	for(int i = 0; i < MAX_READ; ++i) {
		err = recv(conn, &temp, 1, 0);
		if(temp == '\n' || err == 0 || err == SOCKET_ERROR) {
			break;
		} 

		message += temp;

		if ((err == 0 && WSAGetLastError() == 0) || err == SOCKET_ERROR) {
			// Should be giving WSAWOULDBLOCK
			bSocketGone = true;
		}
	}

	return message;
}

bool sendPacket(std::string message) {
	int err = send(conn, message.c_str(), message.length(), 0);
	//char temp[64] = {0};
	//sprintf(temp, "The packet return after being sent was %d", err);
	//WriteChatColor(temp);
	if(err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
		CheckSocket(err);
	}

	return true;
}

/*
Packet #1
Name|Level|Race|Class|CurHP|MaxHP|CurMana|MaxMana|CurEnd|MaxEnd|str|sta|agi|dex|wis|int|cha|fire|cold|magic|dis|pos|
target|targetx|targety|targetz||zoneID|x|y|z|combat|
*/
void sendUpdatePacket() {
	char szBuff[1024] = {0};
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "1|%s|%d|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%s|%f|%f|%f|%u|%f|%f|%f|%d|\n",
			GetCharInfo()->Name, GetCharInfo2()->Level, pEverQuest->GetRaceDesc(GetCharInfo2()->Race), GetClassDesc(GetCharInfo2()->Class),
			GetCurHPS(), GetMaxHPS(), GetCharInfo2()->Mana, GetMaxMana(), GetCharInfo2()->Endurance, GetMaxEndurance(), 
			GetCharInfo()->STR, GetCharInfo()->STA, GetCharInfo()->AGI, GetCharInfo()->DEX, GetCharInfo()->WIS, GetCharInfo()->INT, GetCharInfo()->CHA,
			GetCharInfo()->SaveFire, GetCharInfo()->SaveCold, GetCharInfo()->SaveMagic, GetCharInfo()->SaveDisease, GetCharInfo()->SavePoison,
			(((PSPAWNINFO) pTarget)) ? ((PSPAWNINFO) pTarget)->Name : "null", (((PSPAWNINFO) pTarget)) ? ((PSPAWNINFO)pTarget)->X : 0, (((PSPAWNINFO) pTarget)) ? ((PSPAWNINFO)pTarget)->Y : 0, (((PSPAWNINFO) pTarget)) ? ((PSPAWNINFO)pTarget)->Z : 0, 
			GetCharInfo()->zoneId, ((PSPAWNINFO)pCharSpawn)->X, ((PSPAWNINFO)pCharSpawn)->Y, ((PSPAWNINFO)pCharSpawn)->Z, *EQADDR_ATTACK);

		packet.append(szBuff);
		
		sendPacket(packet);
	}

	//TODO: add in a new packet for when we have a pet. That way we can only send this information when there is in fact a pet involved, otherwise..fuck him
}

/*
Packet #2
*/
void sendZonePacket() {
	char szBuff[1024] = {0};
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "2|%s|%d|%s|%s|\n",
			GetCharInfo()->Name, GetCharInfo2()->Level, pEverQuest->GetRaceDesc(GetCharInfo2()->Race), GetClassDesc(GetCharInfo2()->Class));

		packet.append(szBuff);
		
		sendPacket(packet);
	}
}

/*
Packet #5
*/
void sendMessagePacket(std::string line, std::string msgType, bool toSelf = false) {
	char szBuff[1024] = {0};
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "5|%s|%s|%d|\n", line.c_str(), msgType.c_str(), (toSelf) ? 1 : 0);

		packet.append(szBuff);
		
		sendPacket(packet);
	}	
}

/*
Packet #0
0|name|
*/
void sendLoginPacket() {
	char szBuff[1024] = {0};
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "0|%s|\n", GetCharInfo()->Name);

		packet.append(szBuff);
		
		sendPacket(packet);
	}	
}

/*
Packet #6
6|names|
*/
void sendNamesPacket() {
	char szBuff[1024] = {0};
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "6|%s|\n", "names");

		packet.append(szBuff);
		
		sendPacket(packet);
	}
}

VOID HandleDisconnect(bool sendDisconnect)
{
	if (bConnected)	{
		bConnected = false;
		// Could set linger off here..
		if (sendDisconnect)
		{
			/*
			int err = send(theSocket, CMD_DISCONNECT, sizeof(CMD_DISCONNECT), 0);
			if (err == SOCKET_ERROR || (err == 0 && WSAGetLastError() != WSAEWOULDBLOCK))
			{
				CheckSocket("HandleDisconnect:Send", err);
			}
			*/
		}
		closesocket(conn);
	}
}

void strCleanEnd(PCHAR pszStr) {
	// Remove trailing spaces and CR/LF's

	int len;

	if (pszStr && *pszStr)
	{
		for (len = strlen(pszStr)-1;
		len >= 0 && strchr(" \r\n", pszStr[len]);
		pszStr[len--]=0);
	}
}

std::string removeDispatchTag(std::string line, char start = '[', char end = ']') {
	std::string value;
	bool bCopy = true;

	for(int i = 0; i < line.length(); ++i) {
		if(line[i] == start && bCopy)
			bCopy = false;

		if(bCopy)
			value += line[i];
		else if(line[i] == end && !bCopy)
			bCopy = true;
	}

	return value;
}

void HandleCommand(std::string command) {
	PCHARINFO pCharInfo=GetCharInfo();
	PSPAWNINFO pSpawn = (PSPAWNINFO)pCharSpawn;
	if (pCharInfo) 
		pSpawn=pCharInfo->pSpawn;

	CHAR szTemp[MAX_STRING] = {0};	
	sprintf(szTemp, "%s", command.c_str());
	strCleanEnd(szTemp);

	if (pSpawn)
		DoCommand((PSPAWNINFO)pSpawn, szTemp);

	//sprintf(szTemp, "CMD:%s", command.c_str());
	//WriteChatColor(szTemp);
}
/************************************************************************************
*************************************************************************************
BEGIN CHATWINDOW FUNCTIONS
*************************************************************************************
************************************************************************************/
//Will I ever even use these?????
const std::string COLOR_NAME = "\ay";
const std::string COLOR_NAME_BRACKET = "\ar";
const std::string COLOR_NORMAL = "\ax";

CHAR szColorChars[] = "yogurtbmpwx";

void WriteMessageToChat(std::string line) {
	char szTemp[MAX_STRING] = {0};
	sprintf(szTemp, "%s", line.c_str());
	WriteChatColor(szTemp);
}

void WriteMessageToChat(char * line) {
	WriteChatColor(line);
}

void WriteAllMessage(std::string sender, std::string message) {
	char szTemp[MAX_STRING] = {0};
	sprintf(szTemp,  "\ar<\ay%s\ar> \aw%s %s", sender.c_str(), GetCharInfo()->Name, message.c_str());
	WriteMessageToChat(szTemp);
}

void WriteTellMessage(std::string sender, std::string message) {
	char szTemp[MAX_STRING] = {0};
	sprintf(szTemp, "\ar[\ay%s\ar(\aymsg\ar)] \aw%s", sender.c_str(), message.c_str());
	WriteMessageToChat(szTemp);
}

void HandleMessageAll(std::string line) {
	WriteMessageToChat("LINE IN:[" + line + "]");
	std::string sender, message;
	int varCount = 1;
	bool bCopy = false;

	for(int i = 0; i < line.length(); ++i) {
		if(line[i] == '{')
			bCopy = true;
		else if(line[i] == '}') {
			bCopy = false;
			++varCount;
		} else if(bCopy) {
			if(varCount == 1)
				sender += line[i];
			else if(varCount == 2)
				message += line[i];
		}
	}
	WriteMessageToChat("SENDER:[" + sender + "]");
	WriteMessageToChat("MESSAGE:[" + message + "]");
	WriteAllMessage(sender, message);
}

void HandleMessageTell(std::string line) {
	WriteMessageToChat("LINE IN:[" + line + "]");
	//{sender}{message}
	std::string sender = "";
	std::string message = "";
	int varCount = 1;
	bool bCopy = false;

	for(int i = 0; i < line.length(); ++i) {
		if(line[i] == '{')
			bCopy = true;
		else if(line[i] == '}') {
			bCopy = false;
			++varCount;
		} else if(bCopy) {
			if(varCount == 1)
				sender += line[i];
			else if(varCount == 2)
				message += line[i];
		}
	}
	WriteMessageToChat("SENDER:[" + sender + "]");
	WriteMessageToChat("MESSAGE:[" + message + "]");
	WriteTellMessage(sender, message);
}

void HandleNames(std::string line) {
	WriteMessageToChat("\ar#\aw- Names: " + line + ".");
}

//sprintf(szTemp, "<%s> %s %s", pChar->Name, pChar->Name, szLine);

/*****************************************************
******************************************************
END CHATWINDOW FUNCTIONS
******************************************************
*****************************************************/

/************************************************************************************
*************************************************************************************
BEGIN SLASH COMMANDS
*************************************************************************************
************************************************************************************/
//This is the string that tells the server to send it to everyone
const std::string TOALL_STR = "all";

//These are the commands that can be issued
const std::string TELL_CMD_STR = "/mqct";
const std::string ALL_CMD_STR = "/mqca";
const std::string ALLBUT_CMD_STR = "/mqc";

/*
Removes the slash command and the space after it from the incoming string
*/
std::string removeSlashCommand(std::string line, std::string toRemove) {
	std::string data = "";

	if(line.length() > toRemove.length()) {
		std::string temp;
		for(int i = 0; i < (toRemove.length() + 1); ++i)
			temp.push_back(line[i]);

		data = temp;

		while(data[0] == ' ')
			data = data.substr(1);
	}

	return data;
}

/*
Connects to the server. This was used during testing.
*/
VOID MQ2CenterConnect(PSPAWNINFO pChar, PCHAR szLine) {
	HandleDisconnect(false);
	HandleConnectRequest();
}

/*
Sends a message to one user
*/
VOID SendTell(PSPAWNINFO pChar, PCHAR szLine) {
	std::string packet = szLine;
	sendMessagePacket(packet, TELL_CMD_STR);
}

/*
Sends a message to all users BUT this one
*/
VOID SendMessageAllButMe(PSPAWNINFO pChar, PCHAR szLine) {
	std::string packet = szLine;
	sendMessagePacket(packet, ALL_CMD_STR);
}

/*
Sends a message to every user
*/
VOID SendMessageAll(PSPAWNINFO pChar, PCHAR szLine) {
	std::string packet = szLine;
	sendMessagePacket(packet, ALL_CMD_STR);

	//need to handle the command for myself here
}

VOID MQ2CenterCommand(PSPAWNINFO pChar, PCHAR szLine) {
	CHAR currentArg[MAX_STRING];
	int argn=1;
	GetArg(currentArg,szLine,argn++);

	if (!stricmp(currentArg,"names") || szLine[0]==0) {
		sendNamesPacket();
		return;
	}


}
/******************************
*******************************
END SLASH COMMANDS
*******************************
******************************/

/************************************************************************************
*************************************************************************************
BEGIN GLOBALVARS
*************************************************************************************
*************************************************************************************
*/
class MQ2GlobalValType *pGlobalValType = 0;
class MQ2GlobalVarType *pGlobalVarType = 0;
/* variables used */
map<std::string, std::string> globals;
const std::string DELETE_STR = "delete";
const std::string SET_STR = "set";

typedef pair<std::string, std::string> value_type;
typedef map<std::string, std::string>::iterator iter_type;

/* function to get a val for a variable */
std::string getValByVarName(std::string variable) {
	char szTemp[1024];
	sprintf(szTemp, "GET:[%s]", variable.c_str());
	WriteChatColor(szTemp);

	iter_type it = globals.find(variable);

	if(it == globals.end())
		return "";

	return it->second;
}

/* function to set a val for a variable */
void setValByVarName(std::string variable, std::string value, bool sendUpdate = true) {
	char szTemp[1024];
	sprintf(szTemp, "SET:[%s][%s]", variable.c_str(), value.c_str());
	WriteChatColor(szTemp);

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
	char szTemp[1024];
	sprintf(szTemp, "DECLARE:[%s]", variable.c_str());
	WriteChatColor(szTemp);

	std::string value = "null";
	value_type global;
	global.first = variable;
	global.second = value;
	globals.insert(global);

	sendGlobalVarPacket(variable, value, SET_STR);
}

/* function to delete a variable */
bool deleteVarByName(std::string variable, bool sendUpdate = true) {
	char szTemp[1024];
	sprintf(szTemp, "DELETE:[%s]", variable.c_str());
	WriteChatColor(szTemp);

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
	char szTemp[1024];
	sprintf(szTemp, "GLOBAL:%s", line.c_str());
	WriteChatColor(szTemp);

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

	sprintf(szTemp, "VAR:[%s]", variable.c_str());
	WriteChatColor(szTemp);
	sprintf(szTemp, "VALUE:[%s]", value.c_str());
	WriteChatColor(szTemp);
	sprintf(szTemp, "ACTION:[%s]", action.c_str());
	WriteChatColor(szTemp);

	if(action == SET_STR)
		setValByVarName(varSet.first, varSet.second, false);
	else if(action == DELETE_STR)
		deleteVarByName(varSet.first, false);
}

void DeleteGlobalVars() {
	globals.clear();
}

void GetGlobalVars() {
	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		std::string packet = "4|";
		sendPacket(packet);
	}
}

/*
Action can be:set, delete
*/
void sendGlobalVarPacket(std::string variable, std::string value, std::string action) {
	char szTemp[1024];
	sprintf(szTemp, "SENDING:[%s][%s][%s]", variable.c_str(), value.c_str(), action.c_str());
	WriteChatColor(szTemp);

	char szBuff[1024] = {0};
	std::string packet;

	if(GetGameState() == GAMESTATE_INGAME && GetCharInfo()->pSpawn) {
		sprintf(szBuff, "3|%s|%s|%s|\n", variable.c_str(), value.c_str(), action.c_str());

		packet.append(szBuff);
		
		sendPacket(packet);
	}
}

class MQ2GlobalValType : public MQ2Type {
private:
	std::string variable, value;

public:
	enum GlobalValMembers {
		GetVal=1,
		SetVal=2
	};

	MQ2GlobalValType(std::string var, std::string val):MQ2Type("GlobalVal") {
		TypeMember(GetVal);
		TypeMember(SetVal);
		variable = var;
		value = val;
	}

	MQ2GlobalValType():MQ2Type("GlobalVal") {
		TypeMember(GetVal);
		TypeMember(SetVal);
	}

	~MQ2GlobalValType() {}

	void setVar(std::string var) {
		variable = var;
	}

	void setVal(std::string val) {
		value = val;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest) {
		PMQ2TYPEMEMBER pMember=MQ2GlobalValType::FindMember(Member);
		if (!pMember)
			return false;
		switch((GlobalValMembers)pMember->ID) {
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

class MQ2GlobalVarType : public MQ2Type {
public:
	enum GlobalVarMembers {
		DeclareVar=1,
		GetVar=2,
		DeleteVar=3,
	};

	MQ2GlobalVarType():MQ2Type("GlobalVar")	{
		TypeMember(DeclareVar);
		TypeMember(GetVar);
		TypeMember(DeleteVar);
	}

	~MQ2GlobalVarType()	{}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
	{
		PMQ2TYPEMEMBER pMember=MQ2GlobalVarType::FindMember(Member);
		if (!pMember)
			return false;
		switch((GlobalVarMembers)pMember->ID) {
		case DeclareVar:
			declareVar(Index);
			Dest.DWord = true;
			Dest.Type = pBoolType;
			return true;
		case DeleteVar:
			if(deleteVarByName(Index))
				Dest.DWord = true;
			else
				Dest.DWord = false;
					
			Dest.Type = pBoolType;
			return true;
		case GetVar:			
			std::string val = getValByVarName(Index);

			if(val.length() > 0) {
				pGlobalValType->setVal(val);
				pGlobalValType->setVar(Index);

				Dest.Type = pGlobalValType;
			} else {
				Dest.DWord = false;
				Dest.Type = pBoolType;
			}
			
			return true;
			
		}

		return false;
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		strcpy(Destination,"GlobalVar");
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};

BOOL dataGlobalVal(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pGlobalValType;
	return true;
}

BOOL dataGlobalVar(PCHAR szName, MQ2TYPEVAR &Ret) {
	Ret.DWord = true;
	Ret.Type = pGlobalVarType;
	return true;
}

/**************************
***************************
END GLOBAL VAR
***************************
**************************/

FLOAT CalculateDistance(FLOAT x1, FLOAT y1, FLOAT z1, FLOAT x2, FLOAT y2, FLOAT z2) {
	return abs( sqrt(pow(x1, 2) + pow(y1, 2) + pow(z1, 2)) - sqrt(pow(x2, 2) + pow(y2, 2) + pow(z2, 2)));
}

// Called once, when the plugin is to initialize
PLUGIN_API VOID InitializePlugin(VOID)
{
	DebugSpewAlways("Initializing MQ2Center");

	//bConnected = ConnectClient();
	//if(!bConnected)
	//	HandleConnectRequest();
	
	bInitialized = true;

	AddCommand("/MQ2CenterConnect", MQ2CenterConnect);

	AddMQ2Data("GlobalVar",dataGlobalVar);
	pGlobalVarType = new MQ2GlobalVarType;

	AddMQ2Data("GlobalVal",dataGlobalVal);
	pGlobalValType = new MQ2GlobalValType;

	AddCommand("/mqc", SendMessageAllButMe);
	AddCommand("/bc", SendMessageAllButMe);
	AddCommand("/bca", SendMessageAllButMe);

	AddCommand("/mqca", SendMessageAll);
	AddCommand("/bcaa", SendMessageAll);

	AddCommand("/mqct", SendTell);
	AddCommand("/bct", SendTell);

	AddCommand("/mqccmd", MQ2CenterCommand);

	// Add commands, MQ2Data items, hooks, etc.
	// AddCommand("/mycommand",MyCommand);
	// AddXMLFile("MQUI_MyXMLFile.xml");
	// bmMyBenchmark=AddMQ2Benchmark("My Benchmark Name");

	InitializeCriticalSection(&ConnectCS);
}

// Called once, when the plugin is to shutdown
PLUGIN_API VOID ShutdownPlugin(VOID)
{
	DebugSpewAlways("Shutting down MQ2Center");
	if(bConnected)
		HandleDisconnect(false);

	RemoveCommand("/MQ2CenterConnect");

	RemoveMQ2Data("GlobalVal");
	RemoveMQ2Data("GlobalVar");

	RemoveCommand("/mqc");
	RemoveCommand("/mqca");
	RemoveCommand("/mqct");
	RemoveCommand("/mqccmd");
	RemoveCommand("/bc");
	RemoveCommand("/bca");
	RemoveCommand("/bcaa");
	RemoveCommand("/bct");

	// make sure we're not trying to connect...
	EnterCriticalSection(&ConnectCS);
	LeaveCriticalSection(&ConnectCS);
	DeleteCriticalSection(&ConnectCS);
	// Remove commands, MQ2Data items, hooks, etc.
	// RemoveMQ2Benchmark(bmMyBenchmark);
	// RemoveCommand("/mycommand");
	// RemoveXMLFile("MQUI_MyXMLFile.xml");
}

// Called after entering a new zone
PLUGIN_API VOID OnZoned(VOID)
{
	DebugSpewAlways("MQ2Center::OnZoned()");
	//if(bConnected)
		//sendZonePacket();
}

// Called once directly before shutdown of the new ui system, and also
// every time the game calls CDisplay::CleanGameUI()
PLUGIN_API VOID OnCleanUI(VOID)
{
	DebugSpewAlways("MQ2Center::OnCleanUI()");
	// destroy custom windows, etc
}

// Called once directly after the game ui is reloaded, after issuing /loadskin
PLUGIN_API VOID OnReloadUI(VOID)
{
	DebugSpewAlways("MQ2Center::OnReloadUI()");
	// recreate custom windows, etc
}

// Called every frame that the "HUD" is drawn -- e.g. net status / packet loss bar
PLUGIN_API VOID OnDrawHUD(VOID)
{
	// DONT leave in this debugspew, even if you leave in all the others
//	DebugSpewAlways("MQ2Center::OnDrawHUD()");
}

// Called once directly after initialization, and then every time the gamestate changes
PLUGIN_API VOID SetGameState(DWORD GameState)
{
	DebugSpewAlways("MQ2Center::SetGameState()");

	if (GameState==GAMESTATE_INGAME) {
		//CreateChatWindowIfNeeded();
	} else {
		//closeChatWindow();
	}
	// if (GameState==GAMESTATE_INGAME)
	// create custom windows if theyre not set up, etc
}


// This is called every time MQ pulses
PLUGIN_API VOID OnPulse(VOID)
{
	if(bConnected) {
		std::string msg = readSocket();
		CHAR szTemp[1024];
		if(msg.length() > 0) {
			if(msg.find("[PING]") == std::string::npos) {
				sprintf(szTemp, "DATA: ==>%s<==", msg.c_str());
				WriteChatColor(szTemp);
			}

			//handle the usage of the incoming string here
			if(msg.find("[CMD]") != std::string::npos) {
				HandleCommand(removeDispatchTag(msg));
				WriteChatColor("COMMAND FOUND");
			} else if(msg.find("[GVAR]") != std::string::npos) {
				HandleGlobalVar(removeDispatchTag(msg));
				WriteChatColor("GLOBAL FOUND");
			} else if(msg.find("[MSGALL]") != std::string::npos) {
				WriteChatColor("MSGALL FOUND");
				HandleMessageAll(removeDispatchTag(msg));
			} else if(msg.find("[MSGONE]") != std::string::npos) {
				WriteChatColor("MSGONE FOUND");
				HandleMessageTell(removeDispatchTag(msg));
			} else if(msg.find("[NAMES]") != std::string::npos) {
				WriteChatColor("NAMES FOUND");
				HandleNames(removeDispatchTag(msg));
			}
		}

		if(cycle_count >= MAX_CYCLE_COUNT) {
			//WriteChatColor("Sending a packet");
			sendUpdatePacket();
			cycle_count = 0;
		} else {
			++cycle_count;
		}
	} else if(bSocketGone) {
		//WriteChatColor("Socket is gone");
		bSocketGone = false;
		HandleDisconnect(false);
		HandleConnectRequest();
	} else if(!bConnecting) {
		if((iLastConnectTry + iMinReconnect) < GetTickCount()/1000) {	
			WriteChatColor("Reconnecting to MQ2Center");
			HandleConnectRequest();
			iLastConnectTry = GetTickCount()/1000;
		}
	}

	if((bTriedConnect && bConnected && !bConnecting) || bInitialized) {
		bTriedConnect = false;
		bInitialized = false;
		DeleteGlobalVars();
		GetGlobalVars();
	}
}

// This is called every time WriteChatColor is called by MQ2Main or any plugin,
// IGNORING FILTERS, IF YOU NEED THEM MAKE SURE TO IMPLEMENT THEM. IF YOU DONT
// CALL CEverQuest::dsp_chat MAKE SURE TO IMPLEMENT EVENTS HERE (for chat plugins)
PLUGIN_API DWORD OnWriteChatColor(PCHAR Line, DWORD Color, DWORD Filter)
{
	DebugSpewAlways("MQ2Center::OnWriteChatColor(%s)",Line);
	return 0;
}

// This is called every time EQ shows a line of chat with CEverQuest::dsp_chat,
// but after MQ filters and chat events are taken care of.
PLUGIN_API DWORD OnIncomingChat(PCHAR Line, DWORD Color)
{
	DebugSpewAlways("MQ2Center::OnIncomingChat(%s)",Line);
	return 0;
}

// This is called each time a spawn is added to a zone (inserted into EQ's list of spawns),
// or for each existing spawn when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddSpawn(PSPAWNINFO pNewSpawn)
{
	DebugSpewAlways("MQ2Center::OnAddSpawn(%s)",pNewSpawn->Name);
}

// This is called each time a spawn is removed from a zone (removed from EQ's list of spawns).
// It is NOT called for each existing spawn when a plugin shuts down.
PLUGIN_API VOID OnRemoveSpawn(PSPAWNINFO pSpawn)
{
	DebugSpewAlways("MQ2Center::OnRemoveSpawn(%s)",pSpawn->Name);
}

// This is called each time a ground item is added to a zone
// or for each existing ground item when a plugin first initializes
// NOTE: When you zone, these will come BEFORE OnZoned
PLUGIN_API VOID OnAddGroundItem(PGROUNDITEM pNewGroundItem)
{
	DebugSpewAlways("MQ2Center::OnAddGroundItem(%d)",pNewGroundItem->DropID);
}

// This is called each time a ground item is removed from a zone
// It is NOT called for each existing ground item when a plugin shuts down.
PLUGIN_API VOID OnRemoveGroundItem(PGROUNDITEM pGroundItem)
{
	DebugSpewAlways("MQ2Center::OnRemoveGroundItem(%d)",pGroundItem->DropID);
}

/**************************************************************************************
***************************************************************************************
BEGIN PLAYER FUNCTIONALITY
***************************************************************************************
**************************************************************************************/
struct Player {
	int iPctHp, iPctMana, iPctEnd, iCurHp, iCurMana, iMaxHP, iMaxEnd, iCurEnd, iMaxMana;
	int iZone;
	int iLevel;
	bool bInCombat;
	std::string sName, sClass;
};

class MQ2PlayerType *pPlayerType = 0;
vector<Player> players;

//Need new data type, Player.

//Player[charname] OR Player[index]
// => This means we need to use a dynamic array, likely a vector

//Player[index].PctHPs
//Player[index].PctMana
//Player[index].Combat
//Player[index].CurHP
//Player[index].CurMana
//Player[index].MaxHP
//Player[index].MaxMana
//Player[index].Name
//Player[index].Class
//Player.Count
//Player[index].InZone
class MQ2PlayerType : public MQ2Type {
private:
	Player* plyr;

public:
	enum PlayerMembers {
		PctHP=1,
		PctMana=2,
		Combat=3,
		CurHP=4,
		CurMana=5,
		MaxHP=6,
		MaxMana=7,
		Name=8,
		Class=9,
		Count=10,
		InZone=11,
		Level=12
	};

	MQ2PlayerType():MQ2Type("Player")	{
		TypeMember(PctHP);
		TypeMember(PctMana);
		TypeMember(Combat);
		TypeMember(CurHP);
		TypeMember(CurMana);
		TypeMember(MaxMana);
		TypeMember(MaxHP);
		TypeMember(Name);
		TypeMember(Class);
		TypeMember(Count);
		TypeMember(InZone);
		TypeMember(Level);
	}

	~MQ2PlayerType()	{}

	void SetPlayer(Player* player) {
		plyr = player;
	}

	bool GetMember(MQ2VARPTR VarPtr, PCHAR Member, PCHAR Index, MQ2TYPEVAR &Dest)
	{
		PMQ2TYPEMEMBER pMember=MQ2PlayerType::FindMember(Member);
		if (!pMember)
			return false;
		switch((PlayerMembers)pMember->ID) {
		case PctHP:
			Dest.Type = pIntType;
			Dest.Int = plyr->iPctHp;
			return true;
		case PctMana:
			Dest.Type = pIntType;
			Dest.Int = plyr->iPctMana;
			return true;
		case MaxHP:		
			Dest.Type = pIntType;
			Dest.Int = plyr->iMaxHP;
			return true;
		case MaxMana:	
			Dest.Type = pIntType;
			Dest.Int = plyr->iMaxMana;
			return true;
		case CurHP:		
			Dest.Type = pIntType;
			Dest.Int = plyr->iCurHp;
			return true;
		case CurMana:
			Dest.Type = pIntType;
			Dest.Int = plyr->iCurMana;
			return true;
		case Name:			
			Dest.Type = pStringType;
			strcpy(DataTypeTemp, plyr->sName.c_str());
			Dest.Ptr = DataTypeTemp;
			return true;
		case Class:
			Dest.Type = pStringType;
			strcpy(DataTypeTemp, plyr->sClass.c_str());
			Dest.Ptr = DataTypeTemp;
			return true;
		case Combat:
			Dest.Type = pBoolType;
			if(plyr->bInCombat)
				Dest.DWord = true;
			else
				Dest.DWord = false;

			return true;
		case InZone:	
			Dest.Type = pBoolType;
			if(plyr->iZone == GetCharInfo()->zoneId)
				Dest.DWord = true;
			else
				Dest.DWord = false;

			return true;
		case Level:	
			Dest.Type = pIntType;
			Dest.Int = plyr->iLevel;
			return true;
		}
	}

	bool ToString(MQ2VARPTR VarPtr, PCHAR Destination) {
		strcpy(Destination,"Player");
		return true;
	}

	bool FromData(MQ2VARPTR &VarPtr, MQ2TYPEVAR &Source) {
		return false;
	}

	bool FromString(MQ2VARPTR &VarPtr, PCHAR Source) {
		return false;
	}
};
 
std::string ToLowerCase(string str) {  
	for (int i=0;i<strlen(str.c_str());i++) {
		if (str[i] >= 0x41 && str[i] <= 0x5A)
			str[i] = str[i] + 0x20;
	}

	return str;
}

int GetIndexByName(std::string name) {
	int pos = -1;
	int size = players.size();
	for(int i = 0; i < size; ++i) {
		if(ToLowerCase(players.at(i).sName) == ToLowerCase(name)) {
			pos = i;
			break;
		}
	}

	return pos;
}

Player* GetPlayer(int index) {
	return (&players.at(index));
}

BOOL dataPlayer(PCHAR szName, MQ2TYPEVAR &Ret) {
	if(strcmp(szName, "") == 0) {
		//this is for when there is no index specified
		//Player.Count
		return true;
	}
	
	int index;
	//we want it by index
	if(StringIsInt(szName)) {
		index = StringToInt(szName);
		if(index > players.size() || index < 0)
			return false;

	//we want it by player name
	} else {
		index = GetIndexByName(szName);
		if(index < 0 || index > players.size())
			return false;
	}

	pPlayerType->SetPlayer(GetPlayer(index));

	Ret.DWord = true;
	Ret.Type = pPlayerType;
	return true;
}
/*********************************************
**********************************************
END PLAYER FUNCTIONALITY
**********************************************
*********************************************/
