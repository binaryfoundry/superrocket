@echo off
mkdir bin >nul 2>&1
cd bin
@echo on
cmake -G "Visual Studio 15 2017 Win64" ../
@echo off
cd ../
@echo on
