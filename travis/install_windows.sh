#!/usr/bin/env bash

# Install build dependencies on Windows

# Visual Studio 2019
choco install visualstudio2019buildtools --package-parameters "--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64"

# Python
choco install python --version=3.5.4 -my
choco install python --version=3.6.8 -my
choco install python --version=3.7.7 -my
choco install python --version=3.8.2 -my
