!include "../global.mak"

ALL : "$(OUTDIR)\MQ2NetBots.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2NetBots.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2NetBots.dll"
	-@erase "$(OUTDIR)\MQ2NetBots.exp"
	-@erase "$(OUTDIR)\MQ2NetBots.lib"
	-@erase "$(OUTDIR)\MQ2NetBots.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\Detours\lib60\detours.lib ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2NetBots.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2NetBots.dll" /implib:"$(OUTDIR)\MQ2NetBots.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2NetBots.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2NetBots.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2NetBots.dep")
!INCLUDE "MQ2NetBots.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2NetBots.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2NetBots.cpp

"$(INTDIR)\MQ2NetBots.obj" : $(SOURCE) "$(INTDIR)"

