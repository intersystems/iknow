[![CI Status](https://github.com/intersystems/iknow/workflows/CI/badge.svg)](https://github.com/intersystems/iknow/actions?query=workflow%3A%22CI%22) [![PyPI version](https://img.shields.io/pypi/v/iknowpy?logo=pypi)](https://pypi.org/project/iknowpy) [![Python versions](https://img.shields.io/pypi/pyversions/iknowpy?logo=python)](https://pypi.org/project/iknowpy)

# iKnow

**iKnow** is a library for Natural Language Processing that identifies entities (phrases) and their semantic context in natural language text in English, German, Dutch, French, Spanish, Portuguese, Swedish, Russian, Ukrainian, Czech and Japanese. It was originally developed by [i.Know](https://www.linkedin.com/company/i.know/about/) in Belgium, acquired by [InterSystems](https://www.intersystems.com) in 2010 to be embedded in its Caché and [IRIS Data Platform](http://www.intersystems.com/iris) products. InterSystems published the iKnow engine as open source in 2020. 

[![Getting started with iKnow](https://img.youtube.com/vi/VkR6rcv4_aA/0.jpg)](https://www.youtube.com/watch?v=VkR6rcv4_aA)

This readme file has everything you need to get started, but make sure you click through to the **[wiki](https://github.com/intersystems/iknow/wiki)** for more details on any of these subjects.

- [iKnow](#iknow)
- [Using iKnow](#using-iknow)
  - [From Python](#from-python)
  - [From C++](#from-c)
  - [From InterSystems IRIS](#from-intersystems-iris)
  - [From Different Platforms](#from-other-platforms)
- [Understanding iKnow](#understanding-iknow)
  - [Entities](#entities)
  - [Attributes](#attributes)
  - [How it works](#how-it-works)
- [Building the iKnow Engine](#building-the-iknow-engine)
- [Contributing to iKnow](#contributing-to-iknow)

# Using iKnow

## From Python

The easiest way to see for yourself what iKnow does with text is by giving it a try! Thanks to our [Python interface](https://github.com/intersystems/iknow/wiki/Getting-Started), that only takes two simple steps:

1. Use `pip` to install the `iknowpy` module as follows:

   ```Shell
   pip install iknowpy
   ```

2. From your Python prompt, instantiate the engine and start indexing:

   ```Python
   import iknowpy
   
   engine = iknowpy.iKnowEngine()

   # show supported languages
   print(engine.get_languages_set())

   # index some text
   text = 'This is a test of the Python interface to the iKnow engine.'
   engine.index(text, 'en')

   # print the raw results
   print(engine.m_index)

   # or make it a little nicer
   for s in engine.m_index['sentences']:
       for e in s['entities']:
           print('<'+e['type']+'>'+e['index']+'</'+e['type']+'>', end=' ')
       print('\n')
   ```

If you are looking for another programming language or interface, check out the other [APIs](https://github.com/intersystems/iknow/wiki/APIs). For more on the Python interface, move on to the [Getting Started](https://github.com/intersystems/iknow/wiki/Getting-Started) section in the wiki!

## From C++

The main C++ API file is [engine.h](https://github.com/intersystems/iknow/blob/master/modules/engine/src/engine.h#L203), defining the class `iKnowEngine` with the main entry point:

```C++
index(TextSource, language)
```

 After indexing all data is stored in `iknowdata::Text_Source m_index`. "iknowdata" is the namespace used for all classes that contain output data. Fore more details, please refer to the [API overview](https://github.com/intersystems/iknow/wiki/APIs) on the wiki.


## From InterSystems IRIS

For many years, the iKnow engine has been available as an embedded service on the InterSystems IRIS Data Platform. The obvious advantage of packaging it with a database is that indexing results from many documents can be stored in a single repository, enabling corpus-wide analytics through practical APIs. See the [iKnow documentation](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=GIKNOW) for IRIS or browse the InterSystems Developer Community's articles on setting up an [iKnow domain](https://community.intersystems.com/post/creating-domain-iknow-domain-architect), [browsing it](https://community.intersystems.com/post/iknow-demo-apps-part-1-knowledge-portal) and [using iFind](https://community.intersystems.com/post/free-text-search-way-search-your-text-fields-sql-developers-are-hiding-you) (iKnow-powered text search)

The [InterSystems IRIS Community Edition](https://docs.intersystems.com/irislatest/csp/docbook/DocBook.UI.Page.cls?KEY=ACLOUD) is available from Docker Hub free of charge.

## From Different Platforms

Since version 1.3, a C-interface is available, enabling communication with the iKnow engine in a JSON encoded request/response style:
```C
const char* j_response;
iknow_json(R"({"method" : "index", "language" : "en", "text_source" : "Hello World"})", &j_response);
```
Most API functionality is available in a serialized json format.

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

## CRC's

As of v1.4, the iKnow engine now also produces Concept-Relation-Concept clusters (aka CRC's)

[Read more...](https://github.com/intersystems/iknow/wiki/Concept-Relation-Concept)

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
- A recent [introductory video](https://www.youtube.com/watch?v=VkR6rcv4_aA)
- A not-so-recent [playlist](https://www.youtube.com/watch?v=QfbqISDFC1s&list=PLp4xNHWZ7IQmUwbtvHsiZnn2nblxjug15&index=1) on our video channel
- A fun animation of our unique bottom-up approach in [Japanese](https://www.youtube.com/watch?v=yOVP6visOfo) and [Russian](https://www.youtube.com/watch?v=dNJHWxgVr5I) (English version embedded [here](https://learning.intersystems.com/course/view.php?id=114))

[Read more...](https://github.com/intersystems/iknow/wiki/Approach)



# Building the iKnow Engine

The [source code](https://github.com/intersystems/iknow/wiki/Source-Code) for the iKnow engine is written in C++ and includes .sln files for building with [Microsoft Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/community/) and Makefiles for building in Linux/Unix. 

Please refer to [this wiki page](https://github.com/intersystems/iknow/wiki/Build-Process) for more on the overall build process.


# Contributing to iKnow

You are welcome to contribute to iKnow's engine code and language models. Check out the [Wiki](https://github.com/intersystems/iknow/wiki) for more details on how they work and the [Issues](https://github.com/intersystems/iknow/issues) and [Projects](https://github.com/intersystems/iknow/wiki/Projects) sections for any particular work on the horizon.
