@echo off

set name="nephoid"

set path=C:\Users\Valentin\Documents\GitHub\Nephoid\

set CC65_HOME=F:\2ARC\cc65\bin\

%CC65_HOME%cc65 -Oi %name%.c --add-source
%CC65_HOME%ca65 reset.s
%CC65_HOME%ca65 %name%.s

%CC65_HOME%ld65 -C nes.cfg -o %name%.nes reset.o %name%.o nes.lib

del *.o

pause
