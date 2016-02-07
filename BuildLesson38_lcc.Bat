lrc.exe -v -r -IC:\lcc\include -foobjllc\Debug\Lesson38.res Lesson38.rc 
copy lesson38.res objllc\Debug\Lesson38.res
lcc.exe -c -IC:\lcc\include -g2 NeHeGL.c -Foobjllc\Debug\NeHeGL.obj
lcc.exe -c -IC:\lcc\include -g2 Lesson38.C -Foobjllc\Debug\Lesson38.obj
lcclnk.exe -LC:\lcc\lib -subsystem windows -s objllc\Debug\NeHeGL.obj objllc\Debug\Lesson38.obj objllc\Debug\Lesson38.res  opengl32.lib glu32.lib advapi32.lib comdlg32.lib gdi32.lib winmm.lib -o binllc\Debug\Lesson38.exe
ECHO Finished!

