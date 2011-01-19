!include "../global.mak"

ALL : "$(OUTDIR)\MQ2Exchange.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2Exchange.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2Exchange.dll"
	-@erase "$(OUTDIR)\MQ2Exchange.exp"
	-@erase "$(OUTDIR)\MQ2Exchange.lib"
	-@erase "$(OUTDIR)\MQ2Exchange.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\Detours\lib60\detours.lib ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2Exchange.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2Exchange.dll" /implib:"$(OUTDIR)\MQ2Exchange.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2Exchange.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2Exchange.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2Exchange.dep")
!INCLUDE "MQ2Exchange.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2Exchange.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2Exchange.cpp

"$(INTDIR)\MQ2Exchange.obj" : $(SOURCE) "$(INTDIR)"

