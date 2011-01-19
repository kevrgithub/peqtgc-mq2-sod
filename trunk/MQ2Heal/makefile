!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Heal.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Heal.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Heal.dll"
	-@erase "$(OUTDIR)\MQ2Heal.exp"
	-@erase "$(OUTDIR)\MQ2Heal.lib"
	-@erase "$(OUTDIR)\MQ2Heal.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Heal.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Heal.dll" /implib:"$(OUTDIR)\MQ2Heal.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Heal.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Heal.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Heal.dep")
!INCLUDE "MQ2Heal.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Heal.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Heal.cpp

"$(INTDIR)\MQ2Heal.obj" : $(SOURCE) "$(INTDIR)"

