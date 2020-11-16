# Language Development Utilities

This folder contains Python scripts utility programs that can help with language development:

- `genRAW.py` : will recusively scan a directory and index all *.txt files. The result is one big ".RAW" file collecting all data. This tool is mainly used to build reference material.
- `genTrace.py` : all linguistic "events" during indexing are collected, and stored into a ".log" file. Use this for finer debugging during language modeling. These traces correspond to the [SEMANTIC_ACTION](https://github.com/intersystems/iknow/blob/master/modules/core/src/IkIndexProcess.cpp#L37) macro in the C++ code.

---
### A handy (Windows) utility, is the lang_update.bat script, that automates the needed operations after changing the language data.

- `\modules\lang_update.bat` : this is a Windows utility script that runs the language compiler and rebuilds the engine. To be used during language development to test modifications. It must be copied into the "bin" directory (`iknow\kit\x64\Release\bin`) after building the engine a first time.
