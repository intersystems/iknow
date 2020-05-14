# iKnow

**iKnow** is a library for Natural Language Processing that identifies entities (phrases) and their semantic context in natural language text in English, German, Dutch, French, Spanish, Portuguese, Swedish, Russian, Ukrainian, Czech and Japanese. It was originally developed by [i.Know](https://www.linkedin.com/company/i.know/about/) in Belgium, acquired by [InterSystems](https://www.intersystems.com) in 2010 to be embedded in its Caché and [IRIS Data Platform](http://www.intersystems.com/iris) products. InterSystems published the iKnow engine as open source in 2020. 

This readme file has the basic pointers to get started, but make sure you click through to the **[wiki](https://github.com/intersystems/iknow/wiki)** for more details on any of these subjects.

- [Understanding iKnow](#understanding-iknow)
  - [Entities](#entities)
  - [Attributes](#attributes)
  - [How it works](#how-it-works)
- [Using iKnow](#using-iknow)
  - [Directly](#directly)
  - [From Python](#from-python)
  - [From SpaCy](#from-spacy)
  - [From InterSystems IRIS](#from-intersystems-iris)
  - [From UIMA](#from-uima)
- [Building iKnow](#building-iknow)
  - [Dependencies](#dependencies)
  - [On Windows](#on-windows)
  - [On Linux / Unix](#on-linux--unix)
  - [On Docker](#on-docker)
  - [Building the Python interface](#building-the-python-interface)
- [Contributing to iKnow](#contributing-to-iknow)

# Understanding iKnow

## Entities

iKnow identifies phrase boundaries that define **Entities**, entirely based on the syntactic structure of the sentences, rather than relying on an upfront dictionary or pretrained model. This makes iKnow well-suited for initial exploration of a new corpus. 
iKnow Entities are not [Named Entities](https://en.wikipedia.org/wiki/Named_entity) in the NER sense, but rather the word groups that need to be considered together, representing a concept or relationship as coined by the text author in its entirety. The following examples clearly show the importance of this phrase level to fully capture what the author meant:

| iKnow Entity | Meaning |
|-|-|
| *Dopamine* | small molecule |
| *Dopamine receptor* | drug target |
| *Dopamine receptor antagonist* | chemical drug |
| *Dopamine receptor gene* | gene, molecular sequence |
| *Dopamine receptor gene mutation* | physiological process |

iKnow will label every entity with a simple role that is either *concept* (usually corresponding to Noun Phrases in POS lingo) or *relation* (verbs, prepositions, ...). Typical stop words that have little meaning of their own get categorized as *PathRelevant* (e.g. pronouns) or *NonRelevant* parts, depending on whether they play a role in the sentence structure or are just linguistic fodder.

In the following sample sentence, we've highlighted **concepts**, <u>relations</u> and *PathRelevants* separately.

> **Belgian geuze** <u>is</u> **well-known** <u>across</u> the **continent** <u>for</u> *its* **delicate balance**.

[Read more...](https://github.com/intersystems/iknow/wiki/Entities)

## Attributes

Beyond this simple phrase recognition, iKnow also captures the context of these entities through **semantic attributes**. Attributes label spans (of entities) within a sentence that share a semantic context. Most attributes start from a *marker term* and are then, through linguistic rules, expanded left and right as appropriate per the syntactic structure of the sentence. iKnow's main contribution is in this fine-grained expansion, which has been shown to be more accurate than many ML-based techniques.

iKnow supports the following attribute types:

- **Negation**: iKnow tags all entities participating in a negation, as opposed to an (implied) affirmative context.

  > <u>After discussing</u> *his* **nausea**, the **\[**<span style="color: red">**patient** <u>didn't report suffering from</u> **chest pain**, **shortness** <u>of</u> **breath** <u>or</u> **tickling\]**</span>.

- **Sentiment**: based on a user-supplied list of marker terms, iKnow will identify spans with either a positive or negative sentiment (through separate attributes). Overlapping negation attributes will reverse the sentiment in some language models.

  > <span style="color: green">**\[** *I* <u>liked</u> *the* **striped pijamas\]**</span>, <u>but</u> the **\[slippers** <u>didn't really fit with</u> *it* **\]**.

- **Measurements**, **Time**, **Frequency** and **Duration**: all entities "participating" in an expression of something measurable or time-related will be tagged, enabling efficient recognition of facts in long stretches of natural language text.

  > Upon **exam** **\[two weeks ago\]** *the* **\[patient's weight** was **146.5 pounds\]**.

- **Certainty**: this attribute is a work in progress. See the corresponding [wiki section](https://github.com/intersystems/iknow/wiki/Attributes) for more details.

Some attributes are not available for all languages yet. See the [wiki section](https://github.com/intersystems/iknow/wiki/Language-model-guidelines) for more details.

## How it works

Some InterSystems-era resources on how iKnow works:
- A recent [introductory video](https://www.youtube.com/watch?v=2pQur_PJn_w)
- A not-so-recent [playlist](https://www.youtube.com/watch?v=QfbqISDFC1s&list=PLp4xNHWZ7IQmUwbtvHsiZnn2nblxjug15&index=1) on our video channel
- A fun animation of our unique bottom-up approach in [Japanese](https://www.youtube.com/watch?v=yOVP6visOfo) and [Russian](https://www.youtube.com/watch?v=dNJHWxgVr5I) (English version embedded [here](https://learning.intersystems.com/course/view.php?id=114))

[Read more...](https://github.com/intersystems/iknow/wiki/Approach)


# Using iKnow

Read more on the [APIs](https://github.com/intersystems/iknow/wiki/APIs) here.

## Directly

The C++ API file is "engine.h" (modules\engine\src), it defines the class "iKnowEngine", and it's main method : `index(TextSource, language)`. After indexing all data is stored in `iknowdata::Text_Source m_index`. "iknowdata" is the namespace used for all classes that contain meaningfull data :
- `iknowdata::struct Entity` : represents a text entity after indexing.
- `iknowdata::struct Sent_Attribute` : represents an attribute sentence marker.
- `iknowdata::struct Path_Attribute_Span` : represents a span in the sentence' path after attribute expansion.
- `iknowdata::struct Sentence` : represents a sentence in the text source after indexing.
- `iknowdata::Sentence::Path` : represents a path in a sentence.
- `iknowdata::struct Text_Source` : represents the whole text after indexing.

`enginetest.cpp` (modules\enginetest\enginetest.cpp) has a demo function (`void a_short_demo(void)`) that explains every step from indexing to retrieving the results.

The main `iKnowEngine::index()` method has currently 2 limitations : it only works synchronously and single threaded. A mutex is used to synchronize threads internally, no protection is needed from the side of the client.

## From Python

The `iknowpy` module provides a Python 3 interface to the iKnow engine and offers a one-to-one mapping from Python data types to C++ data types. In particular, `iknowpy` contains the `iKnowEngine` Python class, which behaves in much the same way as the C++ `iKnowEngine` class defined in `<repo_root>/modules/engine/src/engine.h`.

`<repo_root>/modules/iknowpy/test.py` contains an example of how to use the module.

## From SpaCy

WIP

## From InterSystems IRIS

For many years, the iKnow engine has been available as an embedded service on the InterSystems IRIS Data Platform. The obvious advantage of packaging it with a database is that indexing results from many documents can be stored in a single repository, enabling corpus-wide analytics through practical APIs. See the [iKnow documentation](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=GIKNOW) for IRIS or browse the InterSystems Developer Community's articles on setting up an [iKnow domain](https://community.intersystems.com/post/creating-domain-iknow-domain-architect), [browsing it](https://community.intersystems.com/post/iknow-demo-apps-part-1-knowledge-portal) and [using iFind](https://community.intersystems.com/post/free-text-search-way-search-your-text-fields-sql-developers-are-hiding-you) (iKnow-powered text search)

The [InterSystems IRIS Community Edition](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=ACLOUD) is available from Docker Hub free of charge.

## From UIMA

This part of the kit has not yet been added to the open source repository, but relevant documentation can be found [here](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=GUIMA).

# Building the iKnow Engine

The [source code](https://github.com/intersystems/iknow/wiki/Source-Code) for the iKnow engine is written in C++ and includes .sln files for building with [Microsoft Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/community/) and Makefiles for building in Linux/Unix. See also [this wiki page](https://github.com/intersystems/iknow/wiki/Build-Process) for more on the overall build process.

## Dependencies

* [ICU](http://site.icu-project.org/)	: Header files and libraries

## On Windows

### Step 1: Setting up dependencies

1. Download the Win64 binaries for a recent release of the [ICU library](https://github.com/unicode-org/icu/releases/) (e.g. [version 65.1](https://github.com/unicode-org/icu/releases/tag/release-65-1)) and unzip to ```<repo_root>/thirdparty/icu``` (or a local folder of your choice).

2. If you chose a different folder for your ICU libraries, update ```<repo_root>\modules\Dependencies.props``` to represent your local configuration. This is how it looks after download, which should be OK if you used the suggested directory paths:

```
  <PropertyGroup Label="UserMacros">
    <ICUDIR>$(SolutionDir)..\thirdparty\icu\</ICUDIR>
    <ICU_INCLUDE>$(ICUDIR)\include</ICU_INCLUDE>
    <ICU_LIB>$(ICUDIR)\lib64</ICU_LIB>
  </PropertyGroup>
```

### Step 2: Building iKnow

1. Open the Solution file ```<repo_root>\modules\iKnowEngine.sln``` in Visual Studio. We used Visual Studio Community 2019

2. In the Solution Explorer, choose "iKnowEngineTest" as "Set up as startup project"

3. In Solution Configurations, choose either "Debug|x86", or "Release|x64", depending on the kind of executable you prefer.

4. Build the solution, it will build all 29 projects.

### Step 3: Testing the indexer

Once building has succeeded, you can run the test program, depending on which build config you chose:

* ```<repo_root>\kit\x64\Debug\bin\iKnowEngineTest.exe```
* ```<repo_root>\kit\x64\Release\bin\iKnowEngineTest.exe```

:warning: Note that you'll have to add the ```$(ICUDIR)/bin64``` directory to your PATH or copy its .dll files to this test folder in order to run the test executable.

Alternatively, you can also start a debugging session in Visual Studio and walk through the code to inspect it.

The iKnow indexing demo program will index one sentence for each of the 11 languages, and write out the sentence boundaries. That's of course not very spectacular by itself, but future iterations of this demo program will expose more of the entity and context information iKnow detects.

### Optional: Compiling the language models.

The `iKnowLanguageCompiler` project lets you compile the [language models](https://github.com/intersystems/iknow/wiki/Language-Models) themselves to pick up any changes you may have made to the source .csv files. First, build the `iKnowEngineTest` program as described above, since the language compiler relies on common parts, and then build `iKnowLanguageCompiler`. This should result in a new executable:

* ```<repo_root>\kit\x64\(Debug|Release)\bin\iKnowLanguageCompiler.exe```

Open a command window, change directory to ```<repo_root>\kit\x64\(Debug|Release)\bin\```, and run the program with the requested language code (eg: `IKnowLanguageCompiler en` for building the English language model). If no language parameter is supplied, all language models will be rebuilt. After the build process, you must rebuild the test program to pick up the new language models.

It is important to understand the in- and output of this proces. The input consists of a collection of csv-files, representing the language model as assembled by a qualified linguist:

* ```<repo_root>\language_models\(cd|de|en|es|fr|ja|nl|pt|ru|sv|uk)\```

  Each language directory contains 8 (or less) csv-files : "acro", "filter", "labels", "lexreps", "metadata", "prepro", "regex" and "rules". See ```<repo_root>\docs\KB-file-formats.docx``` for a detailed description. These are **input** for the language model builder.

* ```<repo_root>\modules\engine\language_data\```

  This directory contains, per language, the binary representation of the linguistic data, in the form of a header file (`kb_<language>_data.h`), this is **output**, generated by the language compiler, *do not edit*!

* ```<repo_root>\modules\aho\inl\(cd|de|en|es|fr|ja|nl|pt|ru|sv|uk)\```

  This is the place where, per language, AHO state machine data is written, this is **output**, also the result of the language compilation proces, *do not edit*!

The language compiler must be run from its `/bin` directory, and knows the input and output directories, no need for any configuration. If you would like to change these, you'll have to edit the source code. After rebuilding a languge model data, a new build of the language module itself is needed, since this binary data is hard coded for maximum speed.

Please refer to the corresponding wiki section for more on our [language models](https://github.com/intersystems/iknow/wiki/Language-Models).


## On Linux / Unix

### Step 1: Setting up dependencies

1. Download the proper binaries for a recent release of the [ICU library](https://github.com/unicode-org/icu/releases/) (e.g. [version 65.1](https://github.com/unicode-org/icu/releases/tag/release-65-1)) and untar to ```<repo_root>/thirdparty/icu``` (or a local folder of your choice). 

2. Save the path you untarred the archive to a ```ICUDIR``` environment variable.
   Note that your ICU download may have a relative path inside the tar archive, so you may need to use ```--strip-components=4``` or manually reorganise to make sure the ```${ICUDIR}/include``` leads where you'd expect it to lead.

### Step 2: Building iKnow

1. Set the ```IKNOWPLAT``` environment variable to the target platform of your choice: e.g. "lnxubuntux64", "lnxrhx64" or "macx64"
   
2. In the ```<repo_root>``` folder, run ```make all```



## On Docker

While primarily useful for build-testing convenience, we're also providing a Dockerfile that stuffs the code in a clean container with the required ICU libraries. If your Linux / Unix build doesn't seem to work, perhaps a quick look at this Dockerfile will help nail down where trouble starts.

### Step 1: Building the container

1. Optionally open the ```Dockerfile``` to change the ICU library version to use
  
2. Use the ```docker build``` command to package things up:

  ```Shell
  docker build --tag iknow .
  ```
   This will automatically download the ICU library of your choice and register its path for onward building.

### Step 2: Building iKnow

1. Start and step into the container using ```docker run```:

```Shell
docker run --rm -it iknow
```
   The ```--rm``` flag will make sure the container gets dropped after you're done exploring.

2. Inside the container, use ```make all``` to kick off the build.

```Shell
cd /usr/src/iknow
make all
```

### Step 3: Testing iKnow

3. ```make test``` will build and run the testprogram ("iknowenginetest"). You will find the testprogram in /usr/src/iknow/kit/lnxubuntux64/release/bin

```Shell
cd /usr/src/iknow
make test
```


## Building the Python Interface

The `iknowpy` module provides a Python 3 interface to the iKnow engine. The following directions refer to the commands `pip` and `python`. On some platforms, these commands use Python 2 by default, in which case you should execute `pip3` and `python3` instead to ensure that you are using Python 3.

:warning: We are currently looking at packaging our work in a way that would enable a simple pip-wise installation. Stay tuned (or just follow the steps below in the interim) :-)

### Step 1: Build the iKnow engine

Build the iKnow engine following the above directions. If you are Windows, choose the "Release|x64" configuration.

### Step 2: Setting up dependencies

1. Install any version of Python 3 64-bit. Ensure that the installation includes Python header files.

2. Install Cython. You can do this by having a Python distribution that already includes Cython or by running 

   ```Shell
   pip install cython
   ```

### Step 3: Building iknowpy

Open a command shell in the directory `<repo_root>/modules/iknowpy` and execute the build script.

```Shell
python setup.py build_ext --inplace
```

If the build succeeds, a file with the name matching the pattern `iknowpy.*.pyd` appears in the directory. The name of the file will depend on the platform and version of Python you are using.

### Step 4: Testing iknowpy

1. Set up the shared libraries so that the runtime linker can find them.
    - Windows: If your Python version is less than 3.8, then copy the iKnow engine DLLs (`<repo_root>\kit\x64\Release\bin\*.dll`) and ICU DLLs (`<repo_root>\thirdparty\icu\bin64\*.dll`) to `<repo_root>\modules\iknowpy`. Otherwise, keep these shared libraries where they are.

    - Linux: Set the `LD_LIBRARY_PATH` environment variable to indicate where the iKnow engine and ICU shared libraries are.

        ```Shell
        export LD_LIBRARY_PATH = "<repo_root>/kit/$IKNOWPLAT/release/bin:$ICUDIR/lib"
        ```

    - Mac OS: Set the `DYLD_LIBRARY_PATH` environment variable to indicate where the iKnow engine and ICU shared libraries are.

        ```Shell
        export DYLD_LIBRARY_PATH = "<repo_root>/kit/$IKNOWPLAT/release/bin:$ICUDIR/lib"
        ```

2. The test script at `<repo_root>/modules/iknowpy/test.py` provides an example of how to use `iknowpy`. Run this script to call a few iKnow functions from Python and print their results.

    ```Shell
    python test.py
    ```


# Contributing to iKnow

You are welcome to contribute to iKnow's engine code and language models. Check out the [Wiki](https://github.com/intersystems/iknow/wiki) for more details on how they work and the [Issues](https://github.com/intersystems/iknow/issues) and [Projects](https://github.com/intersystems/iknow/wiki/Projects) sections for any particular work on the horizon.
