@echo off
cls

set buildDIR=\build\
set outputDirectories=/Fd.%buildDIR% /Fe.%buildDIR% /Fo.%buildDIR%

set WarningFlags= -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4351
set Flags= %internalProgramFlags% -Zi -EHsc -nologo -fp:fast -Gm- -GR- -EHa- %WarningFlags% %outputDirectories%

set LibsLinkedTo= user32.lib Gdi32.lib
set LinkerFlags= -incremental:no -opt:ref -NODEFAULTLIB:library %LibsLinkedTo%

IF NOT EXIST build mkdir build

del rm %buildDIR%*.pdb

cl %Flags% win32main.cpp /link %LinkerFlags%
