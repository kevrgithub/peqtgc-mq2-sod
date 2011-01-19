!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Dps.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Dps.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Dps.dll"
	-@erase "$(OUTDIR)\MQ2Dps.exp"
	-@erase "$(OUTDIR)\MQ2Dps.lib"
	-@erase "$(OUTDIR)\MQ2Dps.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\Detours\lib60\detours.lib ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Dps.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Dps.dll" /implib:"$(OUTDIR)\MQ2Dps.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Dps.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Dps.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Dps.dep")
!INCLUDE "MQ2Dps.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Dps.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Dps.cpp

"$(INTDIR)\MQ2Dps.obj" : $(SOURCE) "$(INTDIR)"

