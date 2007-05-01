cl doubletoint-msvc71.cpp -nologo -DNDEBUG -D_CRT_SECURE_NO_DEPRECATE -EHsc -MT -GL -GS- -O2 -Ob2 -Oi -Ot -Oy -Op- -fp:fast              -link -LTCG winmm.lib -out:x87.exe
cl doubletoint-msvc71.cpp -nologo -DNDEBUG -D_CRT_SECURE_NO_DEPRECATE -EHsc -MT -GL -GS- -O2 -Ob2 -Oi -Ot -Oy -Op- -fp:fast -QIfist -link -LTCG winmm.lib -out:x87-QIfist.exe
cl doubletoint-msvc71.cpp -nologo -DNDEBUG -D_CRT_SECURE_NO_DEPRECATE -EHsc -MT -GL -GS- -O2 -Ob2 -Oi -Ot -Oy -Op- -fp:fast -arch:SSE2   -link -LTCG winmm.lib -out:sse2.exe
