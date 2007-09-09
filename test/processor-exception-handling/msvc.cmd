rem call cl main_winapi.cpp -nologo -DNDEBUG -D_CRT_SECURE_NO_DEPRECATE -EHa -MT -GL -GS- -O2 -Ob2 -Oi -Ot -Oy -Op- -fp:fast -arch:SSE2 -link -LTCG winmm.lib -out:a.exe
call cl main_winapi.cpp -nologo -D_CRT_SECURE_NO_DEPRECATE -EHa -MT -O0 -link -LTCG winmm.lib -out:a.exe 
