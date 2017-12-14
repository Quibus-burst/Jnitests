
gcc -m64 -lstdc++ -c -I"%JAVA_HOME%\include" -I"%JAVA_HOME%\include\win32" cpuBoost.c
gcc -Wl,--add-stdcall-alias -shared -o cpuBoost.dll cpuBoost.o