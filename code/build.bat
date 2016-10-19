@echo off

set CommonCompilerFlags=-MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -Wv:18 -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4244 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib
set PdbFileName=handmade_%random%.pdb

REM TODO - can we just build both with one exe?

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
REM Optimization switches /O2
timeout 5
echo WAITING FOR PDB > lock.tmp

cl %CommonCompilerFlags% C:\MyGameFromScratch\code\handmadehero.cpp -Fmhandmadehero.map -LD /link -incremental:no -opt:ref -PDB:%PdbFileName% -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender


del lock.tmp

cl %CommonCompilerFlags% C:\MyGameFromScratch\code\Win32_main_handmadehero.cpp -FmWin32_main_handmadehero.map /link %CommonLinkerFlags%
popd