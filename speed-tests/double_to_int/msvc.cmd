cl doubletoint-msvc71.cpp -c -D_CRT_SECURE_NO_DEPRECATE -EHsc -MT -GL -GS- -Ob2 -Oi -Ot -Oy -fp:fast -arch:SSE2 /link doubletoint-msvc71.obj /OUT:a.exe /LTCG winmm.lib
