!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Cast.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Cast.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Cast.dll"
	-@erase "$(OUTDIR)\MQ2Cast.exp"
	-@erase "$(OUTDIR)\MQ2Cast.lib"
	-@erase "$(OUTDIR)\MQ2Cast.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Cast.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Cast.dll" /implib:"$(OUTDIR)\MQ2Cast.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Cast.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Cast.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Cast.dep")
!INCLUDE "MQ2Cast.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Cast.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Cast.cpp

"$(INTDIR)\MQ2Cast.obj" : $(SOURCE) "$(INTDIR)"

