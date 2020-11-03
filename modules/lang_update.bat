echo off
set arg1=%1
shift
iKnowLanguageCompiler %arg1%
MSBuild ..\..\..\..\modules\iKnowEngine.sln -p:Configuration="Release" -p:Platform="x64" -maxcpucount
iKnowEngineTest
REM run "pip show iknowpy" to discover the pipdir
set pipdir="c:\users\jdenys\miniconda3\lib\site-packages"
copy iKnowEngine.dll %pipdir%\iknowpy
copy iKnowModel*.dll %pipdir%\iknowpy