echo off
REM
REM usage = lang_update_iris <language> <IRIS binary directory>, example: "lang_update_iris en C:\InterSystems\NLP117"
REM
REM First argument is language = "en" for English, "nl" for Dutch, "ja" for Japanese, etc...
set lang=%1
REM Second argument is InterSystems IRIS installation directory, like "C:\InterSystems\NLP117"
set irisbindir=%2
shift
iKnowLanguageCompiler %lang%
if %errorlevel% NEQ 0 (
	echo Failure in language compilation : Err=%errorlevel%
	goto exit
)
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild" ..\..\..\..\modules\iKnowEngine.sln -p:Configuration="Release" -p:Platform="x64" -maxcpucount
if %errorlevel% NEQ 0 (
	echo Failure in building the iKnow engine : Err=%errorlevel%
	goto exit
)

copy iKnowModel%lang%.dll %irisbindir%\bin
copy iKnowModel%lang%x.dll %irisbindir%\bin

:exit
echo Done...