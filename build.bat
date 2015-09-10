@echo off

REM build.bat is to build the current projects things into a useable exe

set internalProgramFlags=

set WarningFlags= -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505
set Flags= %internalProgramFlags% -Zi -EHsc -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi %WarningFlags%

set LibsLinkedTo= user32.lib Gdi32.lib
set LinkerFlags= -incremental:no -opt:ref -NODEFAULTLIB:library %LibsLinkedTo%

IF NOT EXIST build mkdir build
pushd build

REM Clearing debug info so new information can be correctly stored
rm *.pdb

REM MSVC build
cl %Flags% ..\win32main.cpp /link %LinkerFlags%
popd
