!include "../global.mak"

ALL : "$(OUTDIR)\MQ2DataStructures.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2DataStructures.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2DataStructures.dll"
	-@erase "$(OUTDIR)\MQ2DataStructures.exp"
	-@erase "$(OUTDIR)\MQ2DataStructures.lib"
	-@erase "$(OUTDIR)\MQ2DataStructures.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2DataStructures.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2DataStructures.dll" /implib:"$(OUTDIR)\MQ2DataStructures.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2DataStructures.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2DataStructures.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2DataStructures.dep")
!INCLUDE "MQ2DataStructures.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2DataStructures.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2DataStructures.cpp

"$(INTDIR)\MQ2DataStructures.obj" : $(SOURCE) "$(INTDIR)"

