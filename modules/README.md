## Windows only files

- **Dependencies.props** : Loaded by iKnowEngine.sln to track module dependencies.
- **EnableBuildCache.props** : Loaded by iKnowEngine.sln to speed up module building.
- **iKnowEngine.sln** : Microsoft Visual Studio 2019 solution file, to build all modules.
- **lang_update.bat** : Utility for building and testing language model changes : 

```Shell
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
```

This batch file must be copied to the binary directory (&lt;git_repo_dir&gt;\iknow\kit\x64\Release\bin), it will run the language compiler, and rebuild the language modules. It will run the test program, and then copy all language related *.dll files to the iknowpy installation directory. If the process does not report any errors, the language update can immediately be tested.
To be able to copy correctly, the pip installation path must be entered (the pipdir), run "pip show iknowpy" to discover your local installation, and modify the "lang_update.bat" file accordingly.
To run, choose "Tools\Visual Studio Command prompt" in Visual Studio, then navigate to the "bin" directory. For the English language, run "lang_update en", choose another language code for other languages. If no language parameter is passed, all languages will be rebuild.
