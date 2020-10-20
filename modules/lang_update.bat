echo off
set arg1=%1
shift
iKnowLanguageCompiler %arg1%
MSBuild ..\..\..\..\modules\iKnowEngine.sln -p:Configuration="Release" -p:Platform="x64" -maxcpucount