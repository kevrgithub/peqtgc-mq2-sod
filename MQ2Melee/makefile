!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Melee.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Melee.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Melee.dll"
	-@erase "$(OUTDIR)\MQ2Melee.exp"
	-@erase "$(OUTDIR)\MQ2Melee.lib"
	-@erase "$(OUTDIR)\MQ2Melee.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Melee.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Melee.dll" /implib:"$(OUTDIR)\MQ2Melee.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Melee.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Melee.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Melee.dep")
!INCLUDE "MQ2Melee.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Melee.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Melee.cpp

"$(INTDIR)\MQ2Melee.obj" : $(SOURCE) "$(INTDIR)"

