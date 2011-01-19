!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Test.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Test.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Test.dll"
	-@erase "$(OUTDIR)\MQ2Test.exp"
	-@erase "$(OUTDIR)\MQ2Test.lib"
	-@erase "$(OUTDIR)\MQ2Test.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Test.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Test.dll" /implib:"$(OUTDIR)\MQ2Test.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Test.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Test.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Test.dep")
!INCLUDE "MQ2Test.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Test.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Test.cpp

"$(INTDIR)\MQ2Test.obj" : $(SOURCE) "$(INTDIR)"

