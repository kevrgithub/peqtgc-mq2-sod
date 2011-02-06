!include "../global.mak"

ALL : "$(OUTDIR)\MQ2CastTimer.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2CastTimer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2CastTimer.dll"
	-@erase "$(OUTDIR)\MQ2CastTimer.exp"
	-@erase "$(OUTDIR)\MQ2CastTimer.lib"
	-@erase "$(OUTDIR)\MQ2CastTimer.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2CastTimer.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2CastTimer.dll" /implib:"$(OUTDIR)\MQ2CastTimer.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2CastTimer.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2CastTimer.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2CastTimer.dep")
!INCLUDE "MQ2CastTimer.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2CastTimer.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2CastTimer.cpp

"$(INTDIR)\MQ2CastTimer.obj" : $(SOURCE) "$(INTDIR)"

