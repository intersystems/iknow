# iKnow

**iKnow** is a library for Natural Language Processing that identifies entities (phrases) and their semantic context in natural language text in English, German, Dutch, French, Spanish, Portuguese, Swedish, Russian, Ukrainian, Czech and Japanese. It was originally developed by [i.Know](https://www.linkedin.com/company/i.know/about/) in Belgium, acquired by [InterSystems](https://www.intersystems.com) in 2010 to be embedded in its Caché and [IRIS Data Platform](http://www.intersystems.com/iris) products. InterSystems published the iKnow engine as open source in 2020. 

- [iKnow](#iknow)
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
- [Contributing to iKnow](#contributing-to-iknow)
  - [List of Authors](#list-of-authors)

# Understanding iKnow

## Entities
In those languages, it identifies phrase boundaries that define **Entities**, entirely based on the syntactic structure of the sentences, rather than relying on an upfront dictionary or pretrained model. This makes iKnow well-suited for initial exploration of a new corpus. 
iKnow Entities are not [Named Entities](https://en.wikipedia.org/wiki/Named_entity) in the NER sense, but rather the word groups that need to be considered together, representing a concept or relationship as coined by the text author in its entirety. The following examples clearly show the importance of this phrase level to fully capture what the author meant:

| iKnow Entity | Meaning |
|-|-|
| *Dopamine* | small molecule |
| *Dopamine receptor* | drug target |
| *Dopamine receptor antagonist* | chemical drug |
| *Dopamine receptor gene* | gene, molecular sequence |
| *Dopamine receptor gene mutation* | physiological process |

iKnow will label every entity with a simple role that is either *concept* (usually corresponding to Noun Phrases in POS lingo) or *relation* (verbs, prepositions, ...). Typical stop words that have little meaning of their own get categorized as *path-relevant* (e.g. pronouns) or *non-relevant* parts, depending on whether they play a role in the sentence structure or are just linguistic fodder.

In the following sample sentence, we've highlighted **concepts**, <u>relations</u> and *path-relevants* separately.

> **Belgian geuze** <u>is</u> **well-known** <u>across</u> the **continent** <u>for</u> *its* **delicate balance**.

## Attributes

Beyond this simple phrase recognition, iKnow also captures the context of these entities through **semantic attributes**. Attributes label spans (of entities) within a sentence that share a semantic context. Most attributes start from a *marker term* and are then, through linguistic rules, expanded left and right as appropriate per the syntactic structure of the sentence. iKnow's main contribution is in this fine-grained expansion, which has been shown to be more accurate than many ML-based techniques.

iKnow supports the following attribute types:

- **Negation**: iKnow tags all entities participating in a negation, as opposed to an (implied) affirmative context.

  > <u>After discussing</u> *his* **nausea**, the <span style="color: red">**patient** <u>didn't report suffering from</u> **chest pain**, **shortness** <u>of</u> **breath** <u>or</u> **tickling**</span>.

- **Sentiment**: based on a user-supplied list of marker terms, iKnow will identify spans with either a positive or negative sentiment (through separate attributes). Overlapping negation attributes will nullify the sentiment section.

  > <span style="color: green">*I* <u>liked</u> the **striped pijamas**</span>, <u>but</u> the **slippers** <u>didn't really fit with</u> *it*.

- **Measurements**, **Time**, **Frequency** and **Duration**: all entities "participating" in an expression of something measurable or time-related will be tagged, enabling efficient recognition of facts in long stretches of natural language text.

  > *He* <u>sustained</u> <span style="color: purple">**less than 5 BED**</span> <u>while inspecting</u> the **nuclear waste facilities** <span style="color: orange">**last year**</span>.

- **Certainty**: this attribute is a work in progress. See the corresponding [Wiki section](https://github.com/intersystems/iknow/wiki/Attributes) for more details.

Some attributes are not available for all languages yet. See the [wiki section](https://github.com/intersystems/iknow/wiki/Attributes) for more details.

## How it works

See the Wiki section for more details (WIP). 

Some InterSystems-era resources:
- A recent [introductory video](https://www.youtube.com/watch?v=2pQur_PJn_w)
- A not-so-recent [playlist](https://www.youtube.com/watch?v=QfbqISDFC1s&list=PLp4xNHWZ7IQmUwbtvHsiZnn2nblxjug15&index=1) on our video channel
- A fun animation of our unique bottom-up approach in [Japanese](https://www.youtube.com/watch?v=yOVP6visOfo) and [Russian](https://www.youtube.com/watch?v=dNJHWxgVr5I) (English version embedded [here](https://learning.intersystems.com/course/view.php?id=114))


# Using iKnow

## Directly

WIP - Jos' docs of iKnowEngine::index()

## From Python

WIP

## From SpaCy

WIP

## From InterSystems IRIS

For many years, the iKnow engine has been available as an embedded service on the InterSystems IRIS Data Platform. The obvious advantage of packaging it with a database is that indexing results from many documents can be stored in a single repository, enabling corpus-wide analytics through practical APIs. See the [iKnow documentation](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=GIKNOW) for IRIS or browse the InterSystems Developer Community's articles on setting up an [iKnow domain](https://community.intersystems.com/post/creating-domain-iknow-domain-architect), [browsing it](https://community.intersystems.com/post/iknow-demo-apps-part-1-knowledge-portal) and [using iFind](https://community.intersystems.com/post/free-text-search-way-search-your-text-fields-sql-developers-are-hiding-you) (iKnow-powered text search)

The [InterSystems IRIS Community Edition](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=ACLOUD) is available from Docker Hub free of charge.

## From UIMA

This part of the kit has not yet been added to the open source repository, but relevant documentation can be found [here](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=GUIMA).

# Building iKnow

The source code is written in C++ and includes .sln files for building with [Microsoft Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/community/) and Makefiles for building in Linux/Unix.

## Dependencies

* [ICU](http://site.icu-project.org/)	: Header files and libraries

## On Windows

### Step 1: Setting up the dependencies

1. Download the [ICU source code archive](https://github.com/unicode-org/icu/releases/tag/release-4-8-2) and unzip to ```<git_dir>/thirdparty/icu-4.0``` or a local folder of your choice.
2. ICU is an elaborate library which requires building it before all of the header files and libraries used by iKnow are in place. Look for the ```<icu_dir>/source/allinone/allinone.sln``` file included in the archive and build it (using the Debug|x64 configuration), or see the ```readme.html``` file also included in the archive for details. 
3. Edit the properties file ```<git_dir>\modules\Dependencies.props``` to represent your local configuration. This is how it looks after download, which should be OK if you used the directory paths suggested in previous steps:

```
    <ICU_DIR>$(SolutionDir)..\thirdparty\icu-4.0\</ICU_DIR>
    <ISC_SHARED>$(SolutionDir)..\shared\</ISC_SHARED>
    <ISC_KERNEL>$(SolutionDir)..\kernel\</ISC_KERNEL>
```

Make sure ```ICU_DIR``` point to the respective local installation directories.

For your convenience, the libraries for icu-4.0 are part of this github distribution, they can be found in :
* ```<git_dir>\kit\winamd64\debug\bin```: icuucd.lib icuucd.dll icuind.lib icuind.dll, the debug versions of the necessary ICU libraries
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

## On Linux / Unix

Coming soon!

# Contributing to iKnow

You are welcome to contribute to iKnow's engine code and language models. Check out the [Wiki section](https://github.com/intersystems/iknow/wiki) for more details on how they work and the Issues section for any particular work on the horizon.

