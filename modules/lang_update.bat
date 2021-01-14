echo off
set arg1=%1
shift
iKnowLanguageCompiler %arg1%
if %errorlevel% NEQ 0 (
	echo Failure in language compilation : Err=%errorlevel%
	goto exit
)
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild" ..\..\..\..\modules\iKnowEngine.sln -p:Configuration="Release" -p:Platform="x64" -maxcpucount
if %errorlevel% NEQ 0 (
	echo Failure in building the iKnow engine : Err=%errorlevel%
	goto exit
)
iKnowEngineTest
if %errorlevel% NEQ 0 (
	echo Failure in testing the iKnow engine : Err=%errorlevel%
	goto exit
)
pushd ..\..\..\..\modules\iknowpy
python setup.py install --fast
popd

:exit
echo Done...