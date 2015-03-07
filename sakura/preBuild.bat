@echo off

HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_define.h -mode=define
HeaderMake -in=..\sakura_core\Funccode_x.hsrc -out=..\sakura_core\Funccode_enum.h -mode=enum -enum=EFunctionCode
MakefileMake -file=..\sakura_core\Makefile -dir=..\sakura_core

rem SubWCRev.exe "..\\" "..\sakura_core\svnrev_template.h" "..\sakura_core\svnrev.h"
rem if %ERRORLEVEL% NEQ 0 (
rem   echo Automatic revision update unavailable, using generic template instead.
rem   echo You can safely ignore this message - see svnrev.h for details.
rem   copy /Y "..\sakura_core\svnrev_unknown.h" "..\sakura_core\svnrev.h"
rem )

ENDLOCAL
:: Always return an errorlevel of 0 -- this allows compilation to continue if SubWCRev failed.
rem exit 0
