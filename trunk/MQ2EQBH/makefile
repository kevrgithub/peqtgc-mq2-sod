!include "../global.mak"

ALL : "$(OUTDIR)\MQ2EQBH.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2EQBH.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2EQBH.dll"
	-@erase "$(OUTDIR)\MQ2EQBH.exp"
	-@erase "$(OUTDIR)\MQ2EQBH.lib"
	-@erase "$(OUTDIR)\MQ2EQBH.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2EQBH.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2EQBH.dll" /implib:"$(OUTDIR)\MQ2EQBH.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2EQBH.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2EQBH.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2EQBH.dep")
!INCLUDE "MQ2EQBH.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2EQBH.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2EQBH.cpp

"$(INTDIR)\MQ2EQBH.obj" : $(SOURCE) "$(INTDIR)"

