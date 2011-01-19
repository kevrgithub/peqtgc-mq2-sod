!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Center.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Center.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Center.dll"
	-@erase "$(OUTDIR)\MQ2Center.exp"
	-@erase "$(OUTDIR)\MQ2Center.lib"
	-@erase "$(OUTDIR)\MQ2Center.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Center.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Center.dll" /implib:"$(OUTDIR)\MQ2Center.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Center.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Center.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Center.dep")
!INCLUDE "MQ2Center.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Center.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Center.cpp

"$(INTDIR)\MQ2Center.obj" : $(SOURCE) "$(INTDIR)"

