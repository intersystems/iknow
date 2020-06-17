REM Install build dependencies on Windows

REM Visual Studio 2019
REM choco install visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

REM Python
REM wget https://www.python.org/ftp/python/3.5.4/python-3.5.4-amd64.exe
REM wget https://www.python.org/ftp/python/3.6.8/python-3.6.8-amd64.exe
REM wget https://www.python.org/ftp/python/3.7.7/python-3.7.7-amd64.exe
wget https://www.python.org/ftp/python/3.8.3/python-3.8.3-amd64.exe
REM python-3.5.4-amd64.exe /quiet InstallAllUsers=1 Shortcuts=0 Include_doc=0 Include_launcher=0 Include_tcltk=0 Include_test=0
REM python-3.6.8-amd64.exe /quiet InstallAllUsers=1 Shortcuts=0 Include_doc=0 Include_launcher=0 Include_tcltk=0 Include_test=0
REM python-3.7.7-amd64.exe /quiet InstallAllUsers=1 Shortcuts=0 Include_doc=0 Include_launcher=0 Include_tcltk=0 Include_test=0
python-3.8.3-amd64.exe /quiet InstallAllUsers=1 Shortcuts=0 Include_doc=0 Include_launcher=0 Include_tcltk=0 Include_test=0

REM Python packages
REM "C:\Program Files\Python35\python.exe" -m pip install -U pip
REM "C:\Program Files\Python35\python.exe" -m pip install -U cython setuptools wheel
REM "C:\Program Files\Python36\python.exe" -m pip install -U pip
REM "C:\Program Files\Python36\python.exe" -m pip install -U cython setuptools wheel
REM "C:\Program Files\Python37\python.exe" -m pip install -U pip
REM "C:\Program Files\Python37\python.exe" -m pip install -U cython setuptools wheel
"C:\Program Files\Python38\python.exe" -m pip install -U pip
"C:\Program Files\Python38\python.exe" -m pip install -U cython setuptools wheel twine
