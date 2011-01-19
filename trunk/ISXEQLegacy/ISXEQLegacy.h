#pragma once
#include "../ISXEQClient.h"
#define __LSTYPE_H__
#define LSTYPEVAR DATAVAR
#define LSOBJECT DATAVAR
#include <isxdk.h>

class ISXEQLegacy :
	public ISXInterface
{
public:

	virtual bool Initialize(ISInterface *p_ISInterface);
	virtual void Shutdown();

	void LoadSettings();
	void ConnectServices();
	void RegisterServices();

	void DisconnectServices();
	void UnRegisterServices();

};

extern ISInterface *pISInterface;
extern HISXSERVICE hPulseService;
extern HISXSERVICE hMemoryService;
extern HISXSERVICE hServicesService;
extern HISXSERVICE hScriptEngineService;

extern HISXSERVICE hEQChatService;
extern HISXSERVICE hEQUIService;
extern HISXSERVICE hEQGamestateService;
extern HISXSERVICE hEQSpawnService;
extern HISXSERVICE hEQZoneService;

extern ISXEQLegacy *pExtension;
#define printf pISInterface->Printf

#undef EzDetour
#define EzDetour(Address, Detour, Trampoline) IS_Detour(pExtension,pISInterface,hMemoryService,(unsigned int)Address,Detour,Trampoline)
#define EzUnDetour(Address) IS_UnDetour(pExtension,pISInterface,hMemoryService,(unsigned int)Address)

#define EzModify(Address,NewData,Length,Reverse) Memory_Modify(pExtension,pISInterface,hMemoryService,(unsigned int)Address,NewData,Length,Reverse)
#define EzUnModify(Address) Memory_UnModify(pExtension,pISInterface,hMemoryService,(unsigned int)Address)

#define EzHttpRequest(_URL_,_pData_) IS_HttpRequest(pExtension,pISInterface,hHTTPService,_URL_,_pData_)

#include "ISXEQLegacyEngine.h"
extern class ISXEQLegacyEngine g_LegacyEngine;
