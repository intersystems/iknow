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
pushd ..\..\..\..\modules\iknowpy
python setup.py install
popd
```

This batch file must be copied to the binary directory (&lt;git_repo_dir&gt;\iknow\kit\x64\Release\bin), it will run the language compiler, and rebuild the language modules. It will run the test program, and then reinstall iknowpy. If the process does not report any errors, the language update can immediately be tested.
To run, choose "Tools\Visual Studio Command prompt" in Visual Studio, then navigate to the "bin" directory. For the English language, run "lang_update en", choose another language code for other languages. If no language parameter is passed, all languages will be rebuild.
