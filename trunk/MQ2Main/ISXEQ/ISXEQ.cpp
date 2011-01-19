//
// ISXEQ
//


#include "..\MQ2Main.h"
#pragma comment(lib,"ISXDK.lib")

// The mandatory pre-setup function.  Our name is "ISXEQ", and the class is ISXEQ.
// This sets up a "ModulePath" variable which contains the path to this module in case we want it,
// and a "PluginLog" variable, which contains the path and filename of what we should use for our
// debug logging if we need it.  It also sets up a variable "pExtension" which is the pointer to
// our instanced class.
ISXPreSetup("ISXEQ",CISXEQ);

// Basic IS datatypes, these get retrieved on startup by our initialize function, so we can use them in
// our Top-Level Objects or custom datatypes
LSType *pStringType=0;
LSType *pIntType=0;
LSType *pBoolType=0;
LSType *pFloatType=0;
LSType *pByteType=0;

LSType *pIntPtrType=0;
LSType *pBoolPtrType=0;
LSType *pFloatPtrType=0;
LSType *pBytePtrType=0;

LSType *pTimeType=0;

ISInterface *pISInterface=0;
HISXSERVICE hPulseService;
HISXSERVICE hMemoryService;
HISXSERVICE hHTTPService;
HISXSERVICE hTriggerService;

HISXSERVICE hChatService;
HISXSERVICE hUIService;
HISXSERVICE hGamestateService;
HISXSERVICE hSpawnService;
HISXSERVICE hZoneService;
unsigned int ChatEventID=0;
unsigned int PersistentPointerClass=0;

// Forward declarations of callbacks
void __cdecl PulseService(bool Broadcast, unsigned int MSG, void *lpData);
void __cdecl MemoryService(bool Broadcast, unsigned int MSG, void *lpData);
void __cdecl HTTPService(bool Broadcast, unsigned int MSG, void *lpData);
void __cdecl TriggerService(bool Broadcast, unsigned int MSG, void *lpData);
void __cdecl ProtectionRequest(ISXInterface *pClient, unsigned int MSG, void *lpData);

#if 0
void __cdecl SoftwareCursorService(bool Broadcast, unsigned int MSG, void *lpData);

HISXSERVICE hSoftwareCursorService=0;
#endif

HISXSERVICE hEQProtectionService=0;


// The constructor of our class.  General initialization cannot be done yet, because we're not given
// the pointer to the Inner Space interface until it is ready for us to initialize.  Just set the
// pointer we have to the interface to 0.  Initialize data members, too.
CISXEQ::CISXEQ(void)
{
}

// Free any remaining resources in the destructor.  This is called when the DLL is unloaded, but
// Inner Space calls the "Shutdown" function first.  Most if not all of the shutdown process should
// be done in Shutdown.
CISXEQ::~CISXEQ(void)
{
}

extern bool MQ2Initialize();
extern void MQ2Shutdown();
// Initialize is called by Inner Space when the extension should initialize.
bool CISXEQ::Initialize(ISInterface *p_ISInterface)
{
	pISInterface=p_ISInterface;
	
	char CurrentModule[512]={0};
	GetModuleFileName(0,CurrentModule,512);
	char *filename;
	if (filename=strrchr(CurrentModule,'\\'))
		filename++;
	else
		filename=CurrentModule;
	if (stricmp(filename,"eqgame.exe"))
	{
		printf("ISXEQ is only meant to be used in eqgame.exe");
		return false;
	}

	// retrieve basic ISData types
	pStringType=pISInterface->FindLSType("string");
	pIntType=pISInterface->FindLSType("int");
	pBoolType=pISInterface->FindLSType("bool");
	pFloatType=pISInterface->FindLSType("float");
	pTimeType=pISInterface->FindLSType("time");
	pByteType=pISInterface->FindLSType("byte");

	pIntPtrType=pISInterface->FindLSType("intptr");
	pBoolPtrType=pISInterface->FindLSType("boolptr");
	pFloatPtrType=pISInterface->FindLSType("floatptr");
	pBytePtrType=pISInterface->FindLSType("byteptr");


	ConnectServices();

	RegisterCommands();
	RegisterAliases();
	RegisterDataTypes();
	RegisterTopLevelObjects();
    RegisterServices();
	HookMemChecker(TRUE);
	strcpy(gszINIPath,INIFileName);
	MQ2Initialize();
	printf("ISXEQ Loaded");
	return true;
}

// shutdown sequence
void CISXEQ::Shutdown()
{
	MQ2Shutdown();
	DisconnectServices();

	UnRegisterServices();
	UnRegisterTopLevelObjects();
	UnRegisterDataTypes();
	UnRegisterAliases();
	UnRegisterCommands();
}

#if 0
class EQSoftwareCursorInterface : public ISXSoftwareCursorInterface
{
public:
	virtual bool CursorEnabled()
	{
		return !bMouseLook;
	}
	virtual bool GetPosition(int &X, int &Y)
	{
		X=EQADDR_MOUSE->X; 
		Y=EQADDR_MOUSE->Y;
		return true;
	}

	virtual bool SetPosition(int X, int Y)
	{
		EQADDR_MOUSE->X = X; 
		EQADDR_MOUSE->Y = Y;
		return true;
	}

	virtual bool DrawCursor()
	{
		if (bMouseLook)
			return false;
//		pWndMgr->DrawCursor();
		//pWndMgr->DrawCursor()
		return true;
	}
} SoftwareCursorInterface;
#endif

void CISXEQ::ConnectServices()
{
	// connect to any services.  Here we connect to "Pulse" which receives a
	// message every frame (after the frame is displayed) and "Memory" which
	// wraps "detours" and memory modifications
	hPulseService=pISInterface->ConnectService(this,"Pulse",PulseService);
	hMemoryService=pISInterface->ConnectService(this,"Memory",MemoryService);
	hHTTPService=pISInterface->ConnectService(this,"HTTP",HTTPService);
	hTriggerService=pISInterface->ConnectService(this,"Triggers",TriggerService);

#if 0
	hSoftwareCursorService=pISInterface->ConnectService(this,"Software Cursor",SoftwareCursorService);

	IS_SoftwareCursorEnable(this,pISInterface,hSoftwareCursorService,&SoftwareCursorInterface);
#endif
}
void CISXEQ::RegisterCommands()
{
	// add any commands
//	pISInterface->AddCommand("MyCommand",MyCommand,true,false);
}

void CISXEQ::RegisterAliases()
{
	// add any aliases
}

void CISXEQ::RegisterDataTypes()
{
	// add any datatypes
	// pMyType = new MyType;
	// pISInterface->AddLSType(*pMyType);

#define DATATYPE(_class_,_variable_,_persistentclass_) _variable_ = new _class_; pISInterface->AddLSType(*_variable_); if (_persistentclass_) pISInterface->SetPersistentClass(_variable_,pISInterface->RegisterPersistentClass(_persistentclass_));
#include "ISXEQDataTypes.h"
#undef DATATYPE
	pGroupMemberType->SetInheritance(pSpawnType);

	// NOTE: SetInheritance does NOT make it inherit, just notifies the syntax checker...
	pCharacterType->SetInheritance(pSpawnType);
	pBuffType->SetInheritance(pSpellType);
//	pCurrentZoneType->SetInheritance(pZoneType);
	pRaidMemberType->SetInheritance(pSpawnType);
}

void CISXEQ::RegisterTopLevelObjects()
{
	// add any Top-Level Objects
	//pISInterface->AddTopLevelObject("ISXEQ",ISXEQData);
#define TOPLEVELOBJECT(_name_,_function_) pISInterface->AddTopLevelObject(_name_,_function_);
#include "ISXEQTopLevelObjects.h"
#undef TOPLEVELOBJECT
}

extern void __cdecl GamestateRequest(ISXInterface *pClient, unsigned int MSG, void *lpData);
extern void __cdecl SpawnRequest(ISXInterface *pClient, unsigned int MSG, void *lpData);

void CISXEQ::RegisterServices()
{
	hEQProtectionService=pISInterface->RegisterService(this,"EQ Memory Protection Service",ProtectionRequest);
	pISInterface->ServiceRequest(this,hMemoryService,MEM_ENABLEPROTECTION,"EQ Memory Protection Service");

	hChatService=pISInterface->RegisterService(this,"EQ Chat Service",0);
	hUIService=pISInterface->RegisterService(this,"EQ UI Service",0);
	hGamestateService=pISInterface->RegisterService(this,"EQ Gamestate Service",GamestateRequest);
	hSpawnService=pISInterface->RegisterService(this,"EQ Spawn Service",SpawnRequest);
	hZoneService=pISInterface->RegisterService(this,"EQ Zone Service",0);

	ChatEventID = pISInterface->RegisterEvent("EQ Chat");
	PersistentPointerClass = pISInterface->RegisterPersistentClass("EQ Objects");
}

void CISXEQ::DisconnectServices()
{
	// gracefully disconnect from services
	if (hPulseService)
		pISInterface->DisconnectService(this,hPulseService);
	if (hMemoryService)
	{
		pISInterface->DisconnectService(this,hMemoryService);
		// memory modifications are automatically undone when disconnecting
		// also, since this service accepts messages from clients we should reset our handle to
		// 0 to make sure we dont try to continue using it
		hMemoryService=0; 
	}
	if (hHTTPService)
	{
		pISInterface->DisconnectService(this,hHTTPService);
	}
	if (hTriggerService)
	{
		pISInterface->DisconnectService(this,hTriggerService);
	}
#if 0
	if (hSoftwareCursorService)
	{
		IS_SoftwareCursorDisable(this,pISInterface,hSoftwareCursorService);
		pISInterface->DisconnectService(this,hSoftwareCursorService);
	}
#endif
	pISInterface->InvalidatePersistentClass(PersistentPointerClass);
}

void CISXEQ::UnRegisterCommands()
{
	// remove commands
//	pISInterface->RemoveCommand("MyCommand");
}
void CISXEQ::UnRegisterAliases()
{
	// remove aliases
}
void CISXEQ::UnRegisterDataTypes()
{
	// remove data types
#define DATATYPE(_class_,_variable_,_persistentclass_)  if (_variable_) {pISInterface->RemoveLSType(*_variable_);delete _variable_; }
#include "ISXEQDataTypes.h"
#undef DATATYPE

}
void CISXEQ::UnRegisterTopLevelObjects()
{
	// remove data items
//	pISInterface->RemoveTopLevelObject("ISXEQ");
#define TOPLEVELOBJECT(_name_,_function_) pISInterface->RemoveTopLevelObject(_name_);
#include "ISXEQTopLevelObjects.h"
#undef TOPLEVELOBJECT
}
void CISXEQ::UnRegisterServices()
{
	// shutdown our own services
	if (hEQProtectionService)
	{
		pISInterface->ServiceRequest(this,hMemoryService,MEM_DISABLEPROTECTION,"EQ Memory Protection Service");
		pISInterface->ShutdownService(this,hEQProtectionService);
	}
	if (hChatService)
		pISInterface->ShutdownService(this,hChatService);
	if (hUIService)
		pISInterface->ShutdownService(this,hUIService);
	if (hGamestateService)
		pISInterface->ShutdownService(this,hGamestateService);
	if (hSpawnService)
		pISInterface->ShutdownService(this,hSpawnService);
	if (hZoneService)
		pISInterface->ShutdownService(this,hZoneService);
}

bool CISXEQ::Protect(unsigned int Address, unsigned int Size, const void *OriginalData)
{
	EQProtected *pProtected=ProtectedMap[Address];
	if (pProtected)
		return false;

	if (IsBadReadPtr((void*)Address,Size))
		return false;

   pProtected = new EQProtected(Address,Size,OriginalData);
   ProtectedMap[Address]=pProtected;
   return true; 
}

bool CISXEQ::UnProtect(unsigned int Address)
{
		map<unsigned int,EQProtected*>::iterator i=ProtectedMap.find(Address);
		if (i==ProtectedMap.end())
			return false;
		EQProtected *pProtected=i->second;
		if (!pProtected)
			return false;
		delete pProtected;
		ProtectedMap.erase(Address);
		return true;
}

extern int __cdecl memcheck0(unsigned char *buffer, int count);
extern int __cdecl memcheck1(unsigned char *buffer, int count, struct mckey key);
extern int __cdecl memcheck2(unsigned char *buffer, int count, struct mckey key);
extern int __cdecl memcheck3(unsigned char *buffer, int count, struct mckey key);
extern int __cdecl memcheck4(unsigned char *buffer, int count, struct mckey key);

// this is the memory checker key struct
struct mckey {
    union {
        int x;
        unsigned char a[4];
        char sa[4];
    };
};

class CObfuscator 
{
public:
    int doit_tramp(int, int); 
    int doit_detour(int opcode, int flag);
};

class CEmoteHook 
{ 
public: 
    VOID Trampoline(void);
    VOID Detour(void);
};

DETOUR_TRAMPOLINE_EMPTY(int __cdecl memcheck0_tramp(unsigned char *buffer, int count));
DETOUR_TRAMPOLINE_EMPTY(int __cdecl memcheck1_tramp(unsigned char *buffer, int count, struct mckey key));
DETOUR_TRAMPOLINE_EMPTY(int __cdecl memcheck2_tramp(unsigned char *buffer, int count, struct mckey key));
DETOUR_TRAMPOLINE_EMPTY(int __cdecl memcheck3_tramp(unsigned char *buffer, int count, struct mckey key));
DETOUR_TRAMPOLINE_EMPTY(int __cdecl memcheck4_tramp(unsigned char *buffer, int count, struct mckey key));

extern VOID HookInlineChecks(BOOL Patch);

VOID CISXEQ::HookMemChecker(BOOL Patch)
{
    if (Patch) {

		if (!EzDetour(__MemChecker0,memcheck0,memcheck0_tramp))
		{
			printf("memcheck0 detour failed");
		}
		if (!EzDetour(__MemChecker2,memcheck2,memcheck2_tramp))
		{
			printf("memcheck2 detour failed");
		}
		if (!EzDetour(__MemChecker3,memcheck3,memcheck3_tramp))
		{
			printf("memcheck3 detour failed");
		}
		if (!EzDetour(__MemChecker4,memcheck4,memcheck4_tramp))
		{
			printf("memcheck4 detour failed");
		}
                if (!EzDetour(CEverQuest__Emote,&CEmoteHook::Detour,&CEmoteHook::Trampoline))
		{
			printf("emote detour failed");
		}
		if (!EzDetour(CObfuscator__doit,&CObfuscator::doit_detour,&CObfuscator::doit_tramp))
		{
			printf("CObfuscator::doit detour failed");
		}
		HookInlineChecks(Patch);
    } else {
		HookInlineChecks(Patch);
		EzUnDetour(__MemChecker0);
		EzUnDetour(__MemChecker2);
		EzUnDetour(__MemChecker3);
		EzUnDetour(__MemChecker4);
		EzUnDetour(CObfuscator__doit);
		EzUnDetour(CEverQuest__Emote);
    }
}

//extern void Heartbeat();
void __cdecl PulseService(bool Broadcast, unsigned int MSG, void *lpData)
{
	if (MSG==PULSE_PREFRAME)
	{
		// "OnPulse"
		// Heartbeat is moved back into ProcessGameEvents, where MQ2's heartbeat is
//		Heartbeat();
	}
}

void __cdecl MemoryService(bool Broadcast, unsigned int MSG, void *lpData)
{
	// no messages are currently associated with this service (other than
	// system messages such as client disconnect), so do nothing.
}
void __cdecl TriggerService(bool Broadcast, unsigned int MSG, void *lpData)
{
	// no messages are currently associated with this service (other than
	// system messages such as client disconnect), so do nothing.
}
void __cdecl HTTPService(bool Broadcast, unsigned int MSG, void *lpData)
{
	switch(MSG)
	{
#define pReq ((HttpFile*)lpData)
	case HTTPSERVICE_FAILURE:
		// HTTP request failed to retrieve document
		printf("ISXEQ URL %s failed",pReq->URL);
		break;
	case HTTPSERVICE_SUCCESS:
		// HTTP request successfully retrieved document
		printf("ISXEQ URL %s -- %d bytes",pReq->URL,pReq->Size);
		// Retrieved data buffer is pReq->pBuffer and is null-terminated
		break;
#undef pReq
	}
}

void __cdecl ProtectionRequest(ISXInterface *pClient, unsigned int MSG, void *lpData)
{
   switch(MSG)
   {
   case MEMPROTECT_PROTECT:
#define pData ((MemProtect*)lpData)
	   pData->Success=pExtension->Protect(pData->Address,pData->Length,pData->OriginalData);
//	   printf("Protection: %X for %d length, success=%d",pData->Address,pData->Length,pData->Success);
#undef pData
	   break;
   case MEMPROTECT_UNPROTECT:
		pExtension->UnProtect((unsigned int)lpData);
	   break;
   }
}

#if 0
void __cdecl SoftwareCursorService(bool Broadcast, unsigned int MSG, void *lpData)
{
	// receives nothing
}
#endif

bool CISXEQ::Memcpy_Clean(unsigned int BeginAddress, unsigned char *buf, unsigned int buflen)
{
	memcpy(buf,(void*)BeginAddress,buflen);

	unsigned int EndAddress=BeginAddress+buflen;
	for (map<unsigned int,EQProtected*>::iterator i = ProtectedMap.begin() ; i!=ProtectedMap.end() ; i++)
	{
		if (EQProtected *pProt=i->second)
		{

		// find leftmost end
		unsigned int RangeEnd=pProt->EndAddress;
		if (RangeEnd>EndAddress)
			RangeEnd=EndAddress;
		
		// find rightmost beginning
		unsigned int RangeBegin=pProt->Address;
		if (RangeBegin<BeginAddress)
			RangeBegin=BeginAddress;

		int Length=RangeEnd-RangeBegin;
		if (Length<=0)
			continue;

		unsigned int ProtOffset = RangeBegin-pProt->Address;
		unsigned int BufOffset = RangeBegin-BeginAddress;
		unsigned int Range = RangeEnd-RangeBegin;
		memcpy(&buf[BufOffset],&pProt->Array[ProtOffset],Range);
		}
	}		
	return true;
}


