echo off
set arg1=%1
shift
iKnowLanguageCompiler %arg1%
if %errorlevel% NEQ 0 (
	echo Failure in language compilation : Err=%errorlevel%
	goto exit
)
MSBuild ..\..\..\..\modules\iKnowEngine.sln -p:Configuration="Release" -p:Platform="x64" -maxcpucount
iKnowEngineTest
pushd ..\..\..\..\modules\iknowpy
python setup.py install
popd

:exit
echo Done...