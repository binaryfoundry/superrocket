@echo off
mkdir bin >nul 2>&1
cd bin
@echo on
cmake -G "Visual Studio 17 2022" ../
@echo off
cd ../
@echo on
