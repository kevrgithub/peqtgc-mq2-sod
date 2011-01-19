!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Engage.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Engage.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Engage.dll"
	-@erase "$(OUTDIR)\MQ2Engage.exp"
	-@erase "$(OUTDIR)\MQ2Engage.lib"
	-@erase "$(OUTDIR)\MQ2Engage.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Engage.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Engage.dll" /implib:"$(OUTDIR)\MQ2Engage.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Engage.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Engage.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Engage.dep")
!INCLUDE "MQ2Engage.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Engage.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Engage.cpp

"$(INTDIR)\MQ2Engage.obj" : $(SOURCE) "$(INTDIR)"

