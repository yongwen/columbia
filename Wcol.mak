# Microsoft Developer Studio Generated NMAKE File, Based on Wcol.dsp
!IF "$(CFG)" == ""
CFG=wcol - Win32 Debug
!MESSAGE No configuration specified. Defaulting to wcol - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "wcol - Win32 Release" && "$(CFG)" != "wcol - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Wcol.mak" CFG="wcol - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "wcol - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "wcol - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "wcol - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Wcol.exe"


CLEAN :
	-@erase "$(INTDIR)\bm.obj"
	-@erase "$(INTDIR)\cat.obj"
	-@erase "$(INTDIR)\cm.obj"
	-@erase "$(INTDIR)\expr.obj"
	-@erase "$(INTDIR)\group.obj"
	-@erase "$(INTDIR)\item.obj"
	-@erase "$(INTDIR)\logop.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mexpr.obj"
	-@erase "$(INTDIR)\physop.obj"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\rules.obj"
	-@erase "$(INTDIR)\ssp.obj"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\supp.obj"
	-@erase "$(INTDIR)\tasks.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wcol.obj"
	-@erase "$(INTDIR)\Wcol.pch"
	-@erase "$(INTDIR)\wcol.res"
	-@erase "$(OUTDIR)\Wcol.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_MEMORY_MANAGER" /Fp"$(INTDIR)\Wcol.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\wcol.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Wcol.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Wcol.pdb" /machine:I386 /out:"$(OUTDIR)\Wcol.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bm.obj" \
	"$(INTDIR)\cat.obj" \
	"$(INTDIR)\cm.obj" \
	"$(INTDIR)\expr.obj" \
	"$(INTDIR)\group.obj" \
	"$(INTDIR)\item.obj" \
	"$(INTDIR)\logop.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mexpr.obj" \
	"$(INTDIR)\physop.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\ssp.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\supp.obj" \
	"$(INTDIR)\tasks.obj" \
	"$(INTDIR)\wcol.obj" \
	"$(INTDIR)\wcol.res"

"$(OUTDIR)\Wcol.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Wcol.exe" "$(OUTDIR)\Wcol.bsc"


CLEAN :
	-@erase "$(INTDIR)\bm.obj"
	-@erase "$(INTDIR)\bm.sbr"
	-@erase "$(INTDIR)\cat.obj"
	-@erase "$(INTDIR)\cat.sbr"
	-@erase "$(INTDIR)\cm.obj"
	-@erase "$(INTDIR)\cm.sbr"
	-@erase "$(INTDIR)\expr.obj"
	-@erase "$(INTDIR)\expr.sbr"
	-@erase "$(INTDIR)\group.obj"
	-@erase "$(INTDIR)\group.sbr"
	-@erase "$(INTDIR)\item.obj"
	-@erase "$(INTDIR)\item.sbr"
	-@erase "$(INTDIR)\logop.obj"
	-@erase "$(INTDIR)\logop.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\mexpr.obj"
	-@erase "$(INTDIR)\mexpr.sbr"
	-@erase "$(INTDIR)\physop.obj"
	-@erase "$(INTDIR)\physop.sbr"
	-@erase "$(INTDIR)\query.obj"
	-@erase "$(INTDIR)\query.sbr"
	-@erase "$(INTDIR)\rules.obj"
	-@erase "$(INTDIR)\rules.sbr"
	-@erase "$(INTDIR)\ssp.obj"
	-@erase "$(INTDIR)\ssp.sbr"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\stdafx.sbr"
	-@erase "$(INTDIR)\supp.obj"
	-@erase "$(INTDIR)\supp.sbr"
	-@erase "$(INTDIR)\tasks.obj"
	-@erase "$(INTDIR)\tasks.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wcol.obj"
	-@erase "$(INTDIR)\Wcol.pch"
	-@erase "$(INTDIR)\wcol.res"
	-@erase "$(INTDIR)\wcol.sbr"
	-@erase "$(OUTDIR)\Wcol.bsc"
	-@erase "$(OUTDIR)\Wcol.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_TABLE_" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\Wcol.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\wcol.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Wcol.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\bm.sbr" \
	"$(INTDIR)\cat.sbr" \
	"$(INTDIR)\cm.sbr" \
	"$(INTDIR)\expr.sbr" \
	"$(INTDIR)\group.sbr" \
	"$(INTDIR)\item.sbr" \
	"$(INTDIR)\logop.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\mexpr.sbr" \
	"$(INTDIR)\physop.sbr" \
	"$(INTDIR)\query.sbr" \
	"$(INTDIR)\rules.sbr" \
	"$(INTDIR)\ssp.sbr" \
	"$(INTDIR)\stdafx.sbr" \
	"$(INTDIR)\supp.sbr" \
	"$(INTDIR)\tasks.sbr" \
	"$(INTDIR)\wcol.sbr"

"$(OUTDIR)\Wcol.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /profile /debug /machine:I386 /out:"$(OUTDIR)\Wcol.exe" 
LINK32_OBJS= \
	"$(INTDIR)\bm.obj" \
	"$(INTDIR)\cat.obj" \
	"$(INTDIR)\cm.obj" \
	"$(INTDIR)\expr.obj" \
	"$(INTDIR)\group.obj" \
	"$(INTDIR)\item.obj" \
	"$(INTDIR)\logop.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mexpr.obj" \
	"$(INTDIR)\physop.obj" \
	"$(INTDIR)\query.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\ssp.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\supp.obj" \
	"$(INTDIR)\tasks.obj" \
	"$(INTDIR)\wcol.obj" \
	"$(INTDIR)\wcol.res"

"$(OUTDIR)\Wcol.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Wcol.dep")
!INCLUDE "Wcol.dep"
!ELSE 
!MESSAGE Warning: cannot find "Wcol.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "wcol - Win32 Release" || "$(CFG)" == "wcol - Win32 Debug"
SOURCE=.\bm.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_MEMORY_MANAGER" /Fp"$(INTDIR)\Wcol.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\bm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_TABLE_" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\Wcol.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\bm.obj"	"$(INTDIR)\bm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\cat.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\cat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\cat.obj"	"$(INTDIR)\cat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\cm.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\cm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\cm.obj"	"$(INTDIR)\cm.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\expr.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\expr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\expr.obj"	"$(INTDIR)\expr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\group.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\group.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\group.obj"	"$(INTDIR)\group.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\item.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\item.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\item.obj"	"$(INTDIR)\item.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\logop.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\logop.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\logop.obj"	"$(INTDIR)\logop.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\main.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_MEMORY_MANAGER" /Fp"$(INTDIR)\Wcol.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_TABLE_" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\Wcol.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\mexpr.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\mexpr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\mexpr.obj"	"$(INTDIR)\mexpr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\physop.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\physop.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\physop.obj"	"$(INTDIR)\physop.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\query.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\query.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\query.obj"	"$(INTDIR)\query.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\rules.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\rules.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\rules.obj"	"$(INTDIR)\rules.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\ssp.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\ssp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\ssp.obj"	"$(INTDIR)\ssp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\stdafx.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_MEMORY_MANAGER" /Fp"$(INTDIR)\Wcol.pch" /Yc /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\stdafx.obj"	"$(INTDIR)\Wcol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_WINDOWS" /D "_DEBUG" /D "_TABLE_" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\Wcol.pch" /Yc /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\stdafx.obj"	"$(INTDIR)\stdafx.sbr"	"$(INTDIR)\Wcol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\supp.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\supp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\supp.obj"	"$(INTDIR)\supp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\tasks.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\tasks.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\tasks.obj"	"$(INTDIR)\tasks.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\wcol.cpp

!IF  "$(CFG)" == "wcol - Win32 Release"


"$(INTDIR)\wcol.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ELSEIF  "$(CFG)" == "wcol - Win32 Debug"


"$(INTDIR)\wcol.obj"	"$(INTDIR)\wcol.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Wcol.pch"


!ENDIF 

SOURCE=.\wcol.rc

"$(INTDIR)\wcol.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

