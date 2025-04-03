## How to build everything (for development)

first you have to clone IXWebSocket in this folder: https://github.com/machinezone/IXWebSocket

Using MSYS2 UCRT64 Shell (for windows)
go to `/c/Users/xrist/OneDrive/Desktop/GitHub/wevotelive/server/build`
write ` cmake .. -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++`
write ` cmake --build .`
then run the exe

How to build it on linux: TBD