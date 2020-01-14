# iKnow Smart Indexer

This repository contains the source code for [InterSystems iKnow](https://community.intersystems.com/tags/iknow) smart indexer. The build tools allow for building an executable that can index text input in 11 languages : 
- "en" English, 
- "de" German, 
- "ru" Russian, 
- "es" Spanish, 
- "fr" French, 
- "ja" Japanese, 
- "nl" Dutch, 
- "pt" Portuguese, 
- "sv" Swedish, 
- "uk" Ukranian, 
- "cs" Czech
	
The source code is written in C++ and includes .sln files for building with [Microsoft Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/community/). Linux/Unix makefiles will be added soon.

## Dependencies

* [ICU 4](http://site.icu-project.org/)	: Header files and libraries
* [Boost 1.55](https://www.boost.org/users/history/version_1_55_0.html)	: Only header files

## On Windows

### Step 1: Setting up the dependencies

1. Download the [ICU source code archive](https://github.com/unicode-org/icu/releases/tag/release-4-8-2) and unzip to ```<git_dir>/thirdparty/icu-4.0``` or a local folder of your choice.
2. ICU is an elaborate library which requires building it before all of the header files and libraries used by iKnow are in place. Look for the ```<icu_dir>/source/allinone/allinone.sln``` file included in the archive and build it (using the Debug|x64 configuration), or see the ```readme.html``` file also included in the archive for details. 
3. Download the [boost_1_55_0 source code archive](https://www.boost.org/users/history/version_1_55_0.html) and unzip to ```<git_dir>/thirdparty/boost_1_55_0``` or a local folder of your choice.
4. Edit the properties file ```<git_dir>\modules\Dependencies.props``` to represent your local configuration. This is how it looks after download, which should be OK if you used the directory paths suggested in previous steps:

```
    <ICU_DIR>$(SolutionDir)..\thirdparty\icu-4.0\</ICU_DIR>
    <BOOST_DIR>$(SolutionDir)..\thirdparty\boost_1_55_0\</BOOST_DIR>
    <ISC_SHARED>$(SolutionDir)..\shared\</ISC_SHARED>
    <ISC_KERNEL>$(SolutionDir)..\kernel\</ISC_KERNEL>
```

Make sure ```ICU_DIR``` and ```BOOST_DIR``` point to the respective local installation directories.

For your convenience, the libraries for icu-4.0 are part of this github distribution, they can be found in :
* ```<git_dir>\kit\winamd64\debug\bin```: icuucd.lib icuucd.dll icuind.lib icuind.dll, the debug versions of the necessary icu libraries
* ```<git_dir>\kit\winamd64\release\bin```: icuuc.lib icuuc.dll icuin.lib icuin.dll, the release versions

All binaries are put in these debug/release directories, there's no need to move them for deployment.


### Step 2: Building iKnow

1. Open the Visual Studio Solution file ```<git_dir>\modules\iKnowEngineDriver.sln```

2. In Solution Explorer, choose "iKnowEngineDriver" as "Set up as startup project", in "Solution Configurations", choose either "winamd64 Debug", or "winamd64 Release", depending on the kind of executable you prefer.

3. Build the solution, it will build all 29 projects.

If all succeeds, you can run the test program :

* ```<git_dir>\kit\winamd64\debug\bin\iKnowEngineDriver```: the debug version
* ```<git_dir>\kit\winamd64\release\bin\iKnowEngineDriver```: the release version

You can also start a debugging session in Visual Studio IDE, and walk through the code to inspect it.

### Step 3: Testing the indexer

The iKnow indexing demo program will index one sentence for each of the 11 languages, and write out the sentence boundaries. That's of course not very spectacular by itself, but future iterations of this demo program will expose more of the entity and context information iKnow detects.

## Linux / Unix

Coming soon!