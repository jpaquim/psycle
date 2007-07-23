call cl main_winapi.cpp -nologo -DNDEBUG -D_CRT_SECURE_NO_DEPRECATE -EHsc -MT -GL -GS- -O2 -Ob2 -Oi -Ot -Oy -Op- -fp:fast -arch:SSE2 
-link -LTCG winmm.lib -out:a.exe
